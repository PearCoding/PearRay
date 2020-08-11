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
#include "sampler/Distribution1D.h"
#include "sampler/SplitSample.h"
#include "spectral/SpectralUpsampler.h"

#include <sstream>
#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>

#include <pugixml.hpp>

namespace PR {
static inline constexpr float projSA(float theta_low, float theta_high, float phi_low, float phi_high)
{
	const float ta1 = std::cos(theta_high);
	const float ta2 = std::cos(theta_low);
	return 0.5f * (ta2 * ta2 - ta1 * ta1) * (phi_high - phi_low);
}

//#define FILTER_KLEMS 5

const auto bi = [](const Vector3f& v) { return -v; };
const auto bo = [](const Vector3f& v) { return Vector3f(v[0], v[1], -v[2]); };
const auto fi = [](const Vector3f& v) { return Vector3f(-v[0], -v[1], v[2]); };
const auto fo = [](const Vector3f& v) { return v; };
// Our sampling space flips implicitly to the inside configuration and has to be considered when working with the Klems basis
const auto warp_inside = [](const Vector3f& v) { return Vector3f(v[0], v[1], -v[2]); };

class KlemsBasis {
public:
	using MultiIndex = std::pair<int, int>;

	struct ThetaBasis {
		float CenterTheta;
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
	inline MultiIndex multiIndexOf(float theta, float phi) const
	{
		int i = Interval::binary_search(mThetaBasis.size(), [&](size_t index) {
			return mThetaBasis[index].LowerTheta <= theta;
		});

		int j = mThetaBasis[i].getPhiIndex(phi);

		return MultiIndex{ i, j };
	}

	// Indexing with theta and phi index offset
	inline MultiIndex multiIndexOf(float theta, float phi, int dti, int dpi) const
	{
		const MultiIndex ij = multiIndexOf(theta, phi);
		return MultiIndex{ (ij.first + dti) % mThetaBasis.size(), (ij.second + dpi) % mThetaBasis[ij.first].PhiCount };
	}

	inline int linearizeMultiIndex(const MultiIndex& mi) const
	{
		return mThetaLinearOffset[mi.first] + mi.second;
	}

	inline MultiIndex multiIndexFromLinear(size_t i) const
	{
		int i1 = Interval::binary_search(mThetaLinearOffset.size(), [&](size_t index) {
			return mThetaLinearOffset[index] <= i;
		});

		return MultiIndex{ i1, i - mThetaLinearOffset[i1] };
	}

	inline int indexOf(float theta, float phi) const
	{
		return linearizeMultiIndex(multiIndexOf(theta, phi));
	}

	inline int indexOf(float theta, float phi, int dti, int dpi) const
	{
		return linearizeMultiIndex(multiIndexOf(theta, phi, dti, dpi));
	}

	Vector2f center(const MultiIndex& mi) const
	{
		Vector2f tp;
		tp[0] = mThetaBasis[mi.first].CenterTheta;
		tp[1] = 2 * PR_PI * mi.second / (float)mThetaBasis[mi.first].PhiCount;
		return tp;
	}

	float pdf(float theta, const MultiIndex& mi) const
	{
		const float area = projSA(mThetaBasis[mi.first].LowerTheta, mThetaBasis[mi.first].UpperTheta, 2 * PR_PI * mi.second / (float)mThetaBasis[mi.first].PhiCount, 2 * PR_PI * (mi.second + 1) / (float)mThetaBasis[mi.first].PhiCount);
		const float cos	 = std::cos(theta);
		return 1 / (area * cos);
	}

	Vector2f sample(const Vector2f& uv, const MultiIndex& mi, float& pdf) const
	{
		Vector2f tp;
		tp[0] = mThetaBasis[mi.first].LowerTheta * (1 - uv[0]) + mThetaBasis[mi.first].UpperTheta * uv[0];
		tp[1] = 2 * PR_PI * (mi.second + uv[1]) / (float)mThetaBasis[mi.first].PhiCount;
		pdf	  = this->pdf(tp[0], mi);
		return tp;
	}

	float theta(size_t linear_i) const
	{
		return mThetaBasis[multiIndexFromLinear(linear_i).first].CenterTheta;
	}

	// Also called lambda in some publications [Also called area or lambda]
	float projectedSolidAngle(const MultiIndex& mi) const
	{
		const auto& tb = mThetaBasis[mi.first];
		return projSA(tb.LowerTheta, tb.UpperTheta, 0, 2 * PR_PI / (float)tb.PhiCount);
	}

	float projectedSolidAngle(size_t linear_i) const
	{
		return projectedSolidAngle(multiIndexFromLinear(linear_i));
	}

private:
	std::vector<ThetaBasis> mThetaBasis;
	std::vector<size_t> mThetaLinearOffset;
	size_t mEntryCount;
};

static inline float distanceSpherical(const Vector2f& tp1, const Vector2f& tp2)
{
	return PR_SQRT2 * std::sqrt(1 + std::cos(tp1(0)) * std::cos(tp2(0)) - std::sin(tp1(0)) * std::sin(tp2(0)) * std::cos(tp1(1) - tp2(1)));
}

using KlemsMatrix = Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic>;
class KlemsComponent {
public:
	inline KlemsComponent(const std::shared_ptr<KlemsBasis>& row, const std::shared_ptr<KlemsBasis>& column)
		: mRowBasis(row)
		, mColumnBasis(column)
		, mMatrix(row->entryCount(), column->entryCount())
	{
	}

	float eval(size_t row, size_t column) const
	{
#ifndef FILTER_KLEMS
		return mMatrix(row, column);
#else // TODO
		constexpr float SIGMA				   = 10 * PR_INV_PI;
		constexpr float NORM				   = 1 / (2 * PR_PI * SIGMA * SIGMA);
#if FILTER_KLEMS == 5
		constexpr int FILTER_COUNT			   = 5;
		static const int THETA_D[FILTER_COUNT] = { 0, -1, 1, 0, 0 };
		static const int PHI_D[FILTER_COUNT]   = { 0, 0, 0, -1, 1 };
#else
		constexpr int FILTER_COUNT			   = 9;
		static const int THETA_D[FILTER_COUNT] = { 0, -1, 1, 0, 0, -1, -1, 1, 1 };
		static const int PHI_D[FILTER_COUNT]   = { 0, 0, 0, -1, 1, -1, 1, -1, 1 };
#endif

		float weight = 0.0f;
		PR_UNROLL_LOOP(FILTER_COUNT)
		for (int i = 0; i < FILTER_COUNT; ++i) {
			const auto col_mi = mColumnBasis->multiIndexOf(in(0), in(1), THETA_D[i], PHI_D[i]);
			const int col	  = mColumnBasis->linearizeMultiIndex(col_mi);
			const auto center = mColumnBasis->center(col_mi);
			const float dist  = distanceSpherical(center, out);

			weight += mMatrix(row, col) * std::exp(-dist * dist / (2 * SIGMA * SIGMA)) * NORM;
		}
		return weight;
#endif
	}

	// Adapted to Front Outgoing
	float eval(const Vector3f& iV, const Vector3f& oV) const
	{
		const Vector2f in  = Spherical::from_direction_hemi(iV);
		const Vector2f out = Spherical::from_direction_hemi(oV);
		const int row	   = mRowBasis->indexOf(out(0), out(1));
		const int col	   = mColumnBasis->indexOf(in(0), in(1));
		return eval(row, col);
	}

	float pdf(const Vector3f& iV, const Vector3f& oV) const
	{
		const Vector2f in  = Spherical::from_direction_hemi(iV);
		const Vector2f out = Spherical::from_direction_hemi(oV);
		const int row	   = mRowBasis->indexOf(out(0), out(1));
		const int col	   = mColumnBasis->indexOf(in(0), in(1));
		return mColumnCDF[row].discretePdf(mMatrix(row, col) / mColumnCDF[row].integral()) * mColumnBasis->pdf(in(0), mColumnBasis->multiIndexFromLinear(col));
	}

	Vector3f sampleIncident(const Vector2f& uv, const Vector3f& oV, float& pdf, float& weight) const
	{
		const Vector2f out = Spherical::from_direction_hemi(oV);
		const int row	   = mRowBasis->indexOf(out(0), out(1));
		const size_t col   = mColumnCDF[row].sampleDiscrete(uv[0], pdf);
		weight			   = eval(row, col);
		float pdf2;
		const Vector2f in = mColumnBasis->sample(Vector2f(uv[0] / (1 - pdf), uv[1]), mColumnBasis->multiIndexFromLinear(col), pdf2);
		pdf *= pdf2;
		return Spherical::cartesian(in(0), in(1));
	}

	inline KlemsMatrix& matrix() { return mMatrix; }

	inline void transpose()
	{
		mMatrix.transposeInPlace();
		std::swap(mRowBasis, mColumnBasis);
		if (!mColumnCDF.empty())
			buildCDF();
	}

	inline void buildCDF()
	{
		mColumnCDF.resize(mRowBasis->entryCount(), Distribution1D(mColumnBasis->entryCount()));
		tbb::parallel_for(tbb::blocked_range<size_t>(0, mColumnCDF.size()),
						  [&](const tbb::blocked_range<size_t>& r) {
							  for (size_t row = r.begin(); row != r.end(); ++row)
								  mColumnCDF[row].generate([&](size_t i) { return mMatrix(row, i); });
						  });
	}

private:
	std::shared_ptr<KlemsBasis> mRowBasis;
	std::shared_ptr<KlemsBasis> mColumnBasis;
	KlemsMatrix mMatrix;
	std::vector<Distribution1D> mColumnCDF;
};

class KlemsMeasurement {
public:
	KlemsMeasurement(SpectralUpsampler* upsampler, const std::string& filename)
		: mFilename(filename)
		, mGood(false)
	{
		PR_ASSERT(upsampler, "Expected valid upsampler");
		// Information about format etc is available at https://windows.lbl.gov/tools/window/documentation

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
		bool rowBased	 = type == "Rows";
		if (!rowBased && type != "Columns") {
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

				const auto theta = child.child("Theta");
				if (theta)
					basis.CenterTheta = PR_DEG2RAD * theta.text().as_float(0);
				else
					basis.CenterTheta = (basis.UpperTheta + basis.LowerTheta) / 2;

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
			const char* type = data.child_value("Wavelength");
			if (!type || strcmp(type, "Visible") != 0) // Skip entries for non-visible wavelengths
				continue;

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
			while (ind < component->matrix().size() && *scat_str) {
				const float value = std::strtof(scat_str, &end);
				if (scat_str == end && value == 0) {
					scat_str = scat_str + 1; // Skip entry
					continue;
				}
				const Eigen::Index row = ind / columnBasis->entryCount(); //Outgoing direction
				const Eigen::Index col = ind % columnBasis->entryCount(); //Incoming direction

				component->matrix()(row, col) = value;
				++ind;
				if (scat_str == end)
					break;
				scat_str = end;
			}

			if (ind != component->matrix().size()) {
				PR_LOG(L_ERROR) << "Could not parse " << filename << ": Given scattered data is not of length " << component->matrix().size() << std::endl;
				return;
			}

			if (rowBased)
				component->transpose();

			component->buildCDF();

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
		const Vector3f iV = L;
		const Vector3f oV = V;
		if (!inside) {
			if (!transmission)
				return mReflectionFront ? mReflectionFront->eval(fi(iV), fo(oV)) : 0;
			else
				return mTransmissionFront ? mTransmissionFront->eval(fi(iV), bo(oV)) : 0;
		} else {
			if (!transmission)
				return mReflectionBack ? mReflectionBack->eval(warp_inside(bi(iV)), warp_inside(bo(oV))) : 0;
			else
				return mTransmissionBack ? mTransmissionBack->eval(warp_inside(bi(iV)), warp_inside(fo(oV))) : 0;
		}
	}

	float pdf(const Vector3f& V, const Vector3f& L, bool inside, bool transmission) const
	{
		const Vector3f iV = L;
		const Vector3f oV = V;
		if (!inside) {
			if (!transmission)
				return mReflectionFront ? mReflectionFront->pdf(fi(iV), fo(oV)) : 0;
			else
				return mTransmissionFront ? mTransmissionFront->pdf(fi(iV), bo(oV)) : 0;
		} else {
			if (!transmission)
				return mReflectionBack ? mReflectionBack->pdf(warp_inside(bi(iV)), warp_inside(bo(oV))) : 0;
			else
				return mTransmissionBack ? mTransmissionBack->pdf(warp_inside(bi(iV)), warp_inside(fo(oV))) : 0;
		}
	}

	Vector3f sampleIncident(const Vector2f& u, const Vector3f& V, bool inside, bool transmission, float& pdf, float& weight) const
	{
		const Vector3f oV = V;

		pdf	   = 0;
		weight = 0;
		if (!inside) {
			if (!transmission && mReflectionFront)
				return fi(mReflectionFront->sampleIncident(u, fo(oV), pdf, weight));
			else if (mTransmissionFront)
				return fi(mTransmissionFront->sampleIncident(u, bo(oV), pdf, weight));
		} else {
			if (!transmission && mReflectionBack)
				return warp_inside(bi(mReflectionBack->sampleIncident(u, warp_inside(bo(oV)), pdf, weight)));
			else if (mTransmissionBack)
				return warp_inside(bi(mTransmissionBack->sampleIncident(u, warp_inside(fo(oV)), pdf, weight)));
		}
		return Vector3f::Zero();
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

	inline bool hasReflection() const { return mReflectionFront || mReflectionBack; }
	inline bool hasTransmission() const { return mTransmissionFront || mTransmissionBack; }
	inline bool hasBothRT() const { return hasReflection() && hasTransmission(); }

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
		out.Weight			  = mTint->eval(in.ShadingContext) * mMeasurement.eval(in.Context.V, in.Context.L, inside, refraction) * std::abs(in.Context.NdotL());
		out.PDF_S			  = (mMeasurement.hasBothRT() ? 0.5f : 1) * mMeasurement.pdf(in.Context.V, in.Context.L, inside, refraction);
		out.Type			  = refraction ? MST_DiffuseTransmission : MST_DiffuseReflection;
	}

	void sample(const MaterialSampleInput& in, MaterialSampleOutput& out,
				const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		const bool inside = mSwapSide ? !in.Context.IsInside : in.Context.IsInside;

		float pdf;
		float weight;
		if (mMeasurement.hasBothRT()) {
			SplitSample1D sample(in.RND[0], 0, 2);
			out.L = mMeasurement.sampleIncident(Vector2f(sample.uniform(), in.RND[1]), in.Context.V, inside, (sample.integral() != 1), pdf, weight);
			pdf /= 2;
		} else if (mMeasurement.hasTransmission()) {
			out.L = mMeasurement.sampleIncident(in.RND, in.Context.V, inside, true, pdf, weight);
		} else {
			out.L = mMeasurement.sampleIncident(in.RND, in.Context.V, inside, false, pdf, weight);
		}

		auto ectx			  = in.Context.expand(out.L);
		const bool refraction = ectx.L(2) < 0.0f;
		out.Weight			  = mTint->eval(in.ShadingContext) * weight * std::abs(ectx.NdotL());
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