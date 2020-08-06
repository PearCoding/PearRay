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
// Information about format etc is available at https://windows.lbl.gov/tools/window/documentation

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
class KlemsComponent {
public:
	inline KlemsComponent(const std::shared_ptr<KlemsBasis>& row, const std::shared_ptr<KlemsBasis>& column)
		: mRowBasis(row)
		, mColumnBasis(column)
		, mMatrix(row->entryCount(), column->entryCount())
	{
	}

	float eval(const Vector3f& V, const Vector3f& L) const
	{
		const Vector2f in  = Spherical::from_direction(L); // The light is incoming, not the view/eye
		const Vector2f out = Spherical::from_direction(V);
		const int row	   = mRowBasis->indexOf(std::abs(out(0)), out(1));
		const int col	   = mColumnBasis->indexOf(std::abs(in(0)), in(1));

		return mMatrix(row, col);
	}

	inline KlemsMatrix& matrix() { return mMatrix; }

private:
	std::shared_ptr<KlemsBasis> mRowBasis;
	std::shared_ptr<KlemsBasis> mColumnBasis;
	KlemsMatrix mMatrix;
};

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

		std::unordered_map<std::string, std::shared_ptr<KlemsBasis>> allbasis;
		for (const auto& anglebasis : datadefinition.children("AngleBasis")) {
			const char* name = anglebasis.child_value("AngleBasisName");
			if (!name) {
				PR_LOG(L_ERROR) << "Could not parse " << filename << ": AngleBasis has no name" << std::endl;
				return;
			}

			// Extract basis information
			std::shared_ptr<KlemsBasis> fullbasis = std::make_shared<KlemsBasis>();
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

				fullbasis->addBasis(basis);
			}
			fullbasis->setup();
			allbasis[name] = std::move(fullbasis);
		}

		if (allbasis.empty()) {
			PR_LOG(L_ERROR) << "Could not parse " << filename << ": No basis given" << std::endl;
			return;
		}

		// Extract wavelengths
		for (const auto& data : layer.children("WavelengthData")) {
			const auto block = data.child("WavelengthDataBlock");
			if (!block) {
				PR_LOG(L_ERROR) << "Could not parse " << filename << ": No WavelengthDataBlock given" << std::endl;
				return;
			}

			// Connect angle basis
			const char* columnBasisName = block.child_value("ColumnAngleBasis");
			const char* rowBasisName	= block.child_value("RowAngleBasis");
			if (!columnBasisName || !rowBasisName) {
				PR_LOG(L_ERROR) << "Could not parse " << filename << ": WavelengthDataBlock has no column or row basis given" << std::endl;
				return;
			}

			std::shared_ptr<KlemsBasis> columnBasis;
			std::shared_ptr<KlemsBasis> rowBasis;
			if (allbasis.count(columnBasisName))
				columnBasis = allbasis.at(columnBasisName);
			if (allbasis.count(rowBasisName))
				rowBasis = allbasis.at(rowBasisName);
			if (!columnBasis || !rowBasis) {
				PR_LOG(L_ERROR) << "Could not parse " << filename << ": WavelengthDataBlock has no known column or row basis given" << std::endl;
				return;
			}

			// Setup component
			std::shared_ptr<KlemsComponent> component = std::make_shared<KlemsComponent>(rowBasis, columnBasis);

			// Parse list of floats
			const char* scat_str = block.child_value("ScatteringData");
			char* end			 = nullptr;
			Eigen::Index ind	 = 0;
			while (ind < component->matrix().size()) {
				const Eigen::Index row		  = ind / rowBasis->entryCount();	 //Outgoing direction
				const Eigen::Index col		  = ind % columnBasis->entryCount(); //Incoming direction
				component->matrix()(row, col) = std::strtof(scat_str, &end);
				if (scat_str == end)
					break;
				scat_str = end;
				++ind;
			}

			// Select correct component
			const std::string direction = block.child_value("WavelengthDataDirection");
			if (direction == "Transmission Front")
				mTransmissionFront = component;
			else if (direction == "Reflection Back")
				mReflectionBack = component;
			else if (direction == "Transmission Back")
				mTransmissionBack = component;
			else
				mReflectionFront = component;
		}

		mGood = true;
	}

	std::string filename() const { return mFilename; }
	bool isValid() const { return mGood; }

	float eval(const Vector3f& V, const Vector3f& L, bool inside, bool transmission) const
	{
		if (!inside) {
			if (!transmission)
				return mReflectionFront ? mReflectionFront->eval(V, L) : 0;
			else
				return mTransmissionFront ? mTransmissionFront->eval(V, L) : 0;
		} else {
			if (!transmission)
				return mReflectionBack ? mReflectionBack->eval(V, L) : 0;
			else
				return mTransmissionBack ? mTransmissionBack->eval(V, L) : 0;
		}
	}

	inline void ensureFrontBack()
	{
		// Make sure all data is present
		if (!mReflectionBack)
			mReflectionBack = mReflectionFront;
		if (!mReflectionFront)
			mReflectionFront = mReflectionBack;

		if (!mTransmissionBack)
			mTransmissionBack = mTransmissionFront;
		if (!mTransmissionFront)
			mTransmissionFront = mTransmissionBack;
	}

private:
	const std::string mFilename;
	bool mGood;
	std::shared_ptr<KlemsComponent> mReflectionFront;
	std::shared_ptr<KlemsComponent> mTransmissionFront;
	std::shared_ptr<KlemsComponent> mReflectionBack;
	std::shared_ptr<KlemsComponent> mTransmissionBack;
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

	void eval(const MaterialEvalInput& in, MaterialEvalOutput& out,
			  const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		const bool inside	  = mSwapSide ? !in.Context.IsInside : in.Context.IsInside;
		const bool refraction = in.Context.L(2) < 0.0f;
		out.Weight			  = mTint->eval(in.ShadingContext) * mMeasurement.eval(in.Context.V, in.Context.L, inside, refraction);
		out.PDF_S			  = Sampling::sphere_pdf();
		out.Type			  = refraction ? MST_DiffuseTransmission : MST_DiffuseReflection;
	}

	void sample(const MaterialSampleInput& in, MaterialSampleOutput& out,
				const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;
		float pdf;
		out.L = Sampling::sphere(in.RND[0], in.RND[1], pdf);

		auto ectx			  = in.Context.expand(out.L);
		const bool inside	  = mSwapSide ? !ectx.IsInside : ectx.IsInside;
		const bool refraction = ectx.L(2) < 0.0f;
		out.Weight			  = mTint->eval(in.ShadingContext) * mMeasurement.eval(ectx.V, ectx.L, inside, refraction);
		out.PDF_S			  = pdf;
		out.Type			  = refraction ? MST_DiffuseTransmission : MST_DiffuseReflection;
	}

	std::string dumpInformation() const override
	{
		std::stringstream stream;

		stream << std::boolalpha << IMaterial::dumpInformation()
			   << "  <KlemsMeasuredMaterial>:" << std::endl
			   << "    Filename: " << mMeasurement.filename() << std::endl
			   << "    Tint:     " << mTint->dumpInformation() << std::endl
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

		if (params.getBool("both_sides", true))
			measurement.ensureFrontBack();

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