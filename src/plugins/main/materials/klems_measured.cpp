#include "Environment.h"
#include "Logger.h"
#include "Profiler.h"
#include "SceneLoadContext.h"
#include "container/Interval.h"
#include "material/IMaterial.h"
#include "material/IMaterialPlugin.h"
#include "math/Sampling.h"
#include "math/Spherical.h"
#include "math/Tangent.h"
#include "renderer/RenderContext.h"
#include "spectral/SpectralUpsampler.h"

#include <sstream>

#include <pugixml.hpp>

namespace PR {
class KlemsBasis {
public:
	struct ThetaBasis {
		float LowerTheta;
		float UpperTheta;
		size_t PhiCount;

		inline int getPhiIndex(float phi) const
		{
			return std::min<int>(PhiCount - 1, std::max<int>(0, phi * PR_INV_PI * 0.5f * PhiCount));
		}

		inline bool isValid() const
		{
			return PhiCount > 0 && LowerTheta < UpperTheta;
		}
	};

	inline void addBasis(const ThetaBasis& basis)
	{
		mThetaBasis.push_back(basis);
	}

	inline void setup()
	{
		std::sort(mThetaBasis.begin(), mThetaBasis.end(),
				  [](const ThetaBasis& a, const ThetaBasis& b) { return a.UpperTheta < b.UpperTheta; });

		mThetaLinearOffset.resize(mThetaBasis.size());
		size_t off = 0;
		for (size_t i = 0; i < mThetaBasis.size(); ++i) {
			mThetaLinearOffset[i] = off;
			off += mThetaBasis[i].PhiCount;
		}

		mEntryCount = off;
	}

	inline size_t entryCount() const { return mEntryCount; }
	inline int indexOf(float theta, float phi) const
	{
		int i = Interval::binary_search(mThetaBasis.size(), [&](size_t index) {
			return mThetaBasis[index].LowerTheta <= theta;
		});

		int j = mThetaBasis[i].getPhiIndex(phi);

		return mThetaLinearOffset[i] + j;
	}

private:
	std::vector<ThetaBasis> mThetaBasis;
	std::vector<size_t> mThetaLinearOffset;
	size_t mEntryCount;
};

using KlemsMatrix = Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic>;

class KlemsMeasurement {
public:
	KlemsMeasurement(SpectralUpsampler* upsampler, const std::string& filename)
		: mFilename(filename)
		, mGood(false)
	{
		PR_ASSERT(upsampler, "Expected valid upsampler");

		// Read Radiance based klems BSDF xml document
		pugi::xml_document doc;
		pugi::xml_parse_result result = doc.load_file(filename.c_str());
		if (!result) {
			PR_LOG(L_ERROR) << "Could not load file " << filename << ": " << result.description() << std::endl;
			return;
		}

		const auto layer = doc.child("WindowElement").child("Optical").child("Layer");
		if (!layer) {
			PR_LOG(L_ERROR) << "Could not parse " << filename << ": No Layer tag" << std::endl;
			return;
		}

		// Extract data definition and therefor also angle basis
		const auto datadefinition = layer.child("DataDefinition");
		if (!datadefinition) {
			PR_LOG(L_ERROR) << "Could not parse " << filename << ": No DataDefinition tag" << std::endl;
			return;
		}

		std::string type = datadefinition.child_value("IncidentDataStructure");
		if (type != "Columns") {
			PR_LOG(L_ERROR) << "Could not parse " << filename << ": Expected IncidentDataStructure of 'Columns' but got '" << type << "' instead" << std::endl;
			return;
		}

		const auto anglebasis = datadefinition.child("AngleBasis");
		if (!anglebasis) {
			PR_LOG(L_ERROR) << "Could not parse " << filename << ": No AngleBasis tag" << std::endl;
			return;
		}

		// Extract basis information
		for (const auto& child : anglebasis.children("AngleBasisBlock")) {
			KlemsBasis::ThetaBasis basis;

			const auto bounds = child.child("ThetaBounds");
			basis.LowerTheta  = PR_DEG2RAD * bounds.child("LowerTheta").text().as_float(0);
			basis.UpperTheta  = PR_DEG2RAD * bounds.child("UpperTheta").text().as_float(0);
			basis.PhiCount	  = child.child("nPhis").text().as_uint(0);
			if (!basis.isValid()) {
				PR_LOG(L_ERROR) << "Could not parse " << filename << ": Invalid AngleBasisBlock given" << std::endl;
				return;
			}

			mBasis.addBasis(basis);
		}
		mBasis.setup();

		// Extract wavelengths
		for (const auto& data : layer.children("WavelengthData")) {
			const auto block = data.child("WavelengthDataBlock");
			if (!block) {
				PR_LOG(L_ERROR) << "Could not parse " << filename << ": No WavelengthDataBlock given" << std::endl;
				return;
			}

			// Select correct component
			const std::string direction = block.child_value("WavelengthDataDirection");
			KlemsMatrix* matrix			= &mReflectionFront;
			if (direction.find("Transmission Front") != std::string::npos)
				matrix = &mTransmissionFront;
			if (direction.find("Reflection Back") != std::string::npos)
				matrix = &mReflectionBack;
			if (direction.find("Transmission Back") != std::string::npos)
				matrix = &mTransmissionBack;

			*matrix = KlemsMatrix(mBasis.entryCount(), mBasis.entryCount());

			// Parse list of floats
			const char* scat_str = block.child_value("ScatteringData");
			char* end			 = nullptr;
			Eigen::Index ind	 = 0;
			while (ind < matrix->size()) {
				const Eigen::Index row = ind / mBasis.entryCount(); //nin
				const Eigen::Index col = ind % mBasis.entryCount(); //nout
				(*matrix)(row, col)	   = std::strtof(scat_str, &end);
				if (scat_str == end)
					break;
				scat_str = end;
				++ind;
			}
		}

		// Make sure all data is present
		if (mReflectionBack.rows() == 0)
			mReflectionBack = mReflectionFront;
		if (mReflectionFront.rows() == 0)
			mReflectionFront = mReflectionBack;

		if (mTransmissionBack.rows() == 0)
			mTransmissionBack = mTransmissionFront;
		if (mTransmissionFront.rows() == 0)
			mTransmissionFront = mTransmissionBack;

		mGood = true;
	}

	std::string filename() const { return mFilename; }
	bool isValid() const { return mGood; }

	float eval(const Vector3f& V, const Vector3f& L, bool inside, bool transmission) const
	{
		const Vector2f in  = Spherical::from_direction(V);
		const Vector2f out = Spherical::from_direction(L);
		const int row	   = mBasis.indexOf(std::abs(in(0)), in(1));
		const int col	   = mBasis.indexOf(std::abs(out(0)), out(1));

		if (!inside) {
			if (!transmission) {
				return mReflectionFront(row, col);
			} else {
				return mTransmissionFront(row, col);
			}
		} else {
			if (!transmission) {
				return mReflectionBack(row, col);
			} else {
				return mTransmissionBack(row, col);
			}
		}
	}

private:
	const std::string mFilename;
	bool mGood;
	KlemsBasis mBasis; // Currently only one basis is supported
	KlemsMatrix mReflectionFront;
	KlemsMatrix mTransmissionFront;
	KlemsMatrix mReflectionBack;
	KlemsMatrix mTransmissionBack;
};

class KlemsMeasuredMaterial : public IMaterial {
public:
	KlemsMeasuredMaterial(uint32 id, const KlemsMeasurement& measurement, const std::shared_ptr<FloatSpectralNode>& tint, bool swapside)
		: IMaterial(id)
		, mMeasurement(measurement)
		, mTint(tint)
		, mSwapSide(swapside)
	{
	}

	virtual ~KlemsMeasuredMaterial() = default;

	void startGroup(size_t, const RenderTileSession&) override
	{
	}

	void endGroup() override
	{
	}

	void eval(const MaterialEvalInput& in, MaterialEvalOutput& out,
			  const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		//const float dot = std::max(0.0f, in.Context.NdotL());
		const bool inside = mSwapSide ? !in.Context.IsInside : in.Context.IsInside;
		out.Weight		  = mTint->eval(in.ShadingContext) * mMeasurement.eval(in.Context.V, in.Context.L, inside, in.Context.L(2) < 0.0f);
		out.PDF_S		  = Sampling::sphere_pdf();
		out.Type		  = MST_DiffuseReflection;
	}

	void sample(const MaterialSampleInput& in, MaterialSampleOutput& out,
				const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		float pdf;
		out.L = Sampling::sphere(in.RND[0], in.RND[1], pdf);

		auto ectx		  = in.Context.expand(out.L);
		const bool inside = mSwapSide ? !ectx.IsInside : ectx.IsInside;
		out.Weight		  = mTint->eval(in.ShadingContext) * mMeasurement.eval(ectx.V, ectx.L, inside, ectx.L(2) < 0.0f);
		out.Type		  = MST_DiffuseReflection;
		out.PDF_S		  = pdf;
	}

	std::string dumpInformation() const override
	{
		std::stringstream stream;

		stream << std::boolalpha << IMaterial::dumpInformation()
			   << "  <KlemsMeasuredMaterial>:" << std::endl
			   << "    Filename: " << mMeasurement.filename() << std::endl
			   << "    Tint:     " << mTint << std::endl
			   << "    SwapSide: " << (mSwapSide ? "true" : "false") << std::endl;

		return stream.str();
	}

private:
	const KlemsMeasurement mMeasurement;
	const std::shared_ptr<FloatSpectralNode> mTint;
	const bool mSwapSide;
};

class KlemsMeasuredMaterialPlugin : public IMaterialPlugin {
public:
	std::shared_ptr<IMaterial> create(uint32 id, const std::string&, const SceneLoadContext& ctx) override
	{
		const ParameterGroup& params = ctx.Parameters;
		KlemsMeasurement measurement(ctx.Env->defaultSpectralUpsampler().get(), ctx.escapePath(params.getString("filename", "")));

		if (measurement.isValid())
			return std::make_shared<KlemsMeasuredMaterial>(id,
														   measurement,
														   ctx.Env->lookupSpectralNode(params.getParameter("tint"), 1),
														   params.getBool("swap_side", false));
		else
			return nullptr;
	}

	const std::vector<std::string>& getNames() const override
	{
		const static std::vector<std::string> names({ "klems", "klems_measured", "klems-measured" });
		return names;
	}

	bool init() override
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::KlemsMeasuredMaterialPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)