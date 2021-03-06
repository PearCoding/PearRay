#include "Environment.h"
#include "Logger.h"
#include "Profiler.h"
#include "SceneLoadContext.h"
#include "container/Interval.h"
#include "material/IMaterial.h"
#include "material/IMaterialPlugin.h"
#include "math/Distribution1D.h"
#include "math/Sampling.h"
#include "math/Spherical.h"
#include "math/SplitSample.h"
#include "renderer/RenderContext.h"

#include <sstream>
#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>

#include <pugixml.hpp>

namespace PR {
static inline /*constexpr*/ float projSA(float theta_low, float theta_high, float phi_low, float phi_high)
{
	const float ta1 = std::cos(theta_high);
	const float ta2 = std::cos(theta_low);
	return 0.5f * (ta2 * ta2 - ta1 * ta1) * (phi_high - phi_low);
}

//#define FILTER_KLEMS 5

// Our sampling space flips implicitly to the inside configuration and has to be considered when working with the Klems basis
// therefor the opposite funtions fi and bi are not needed
const auto fo = [](const Vector3f& v) { return v; };
const auto fi = [](const Vector3f& v) { return Vector3f(-v[0], -v[1], v[2]); };
const auto bo = [](const Vector3f& v) { return Vector3f(v[0], v[1], -v[2]); };
const auto bi = [](const Vector3f& v) { return -v; };

class KlemsBasis {
public:
	using MultiIndex = std::pair<int, int>;

	KlemsBasis() = default;

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

	inline int linearizeMultiIndex(const MultiIndex& mi) const
	{
		return mThetaLinearOffset[mi.first] + mi.second;
	}

	inline MultiIndex multiIndexFromLinear(size_t i) const
	{
		int i1 = Interval::binary_search(mThetaLinearOffset.size(), [&](size_t index) {
			return mThetaLinearOffset[index] <= i;
		});

		PR_ASSERT(i >= mThetaLinearOffset[i1], "Invalid use of linear offset array together with binary search");
		return MultiIndex{ i1, i - mThetaLinearOffset[i1] };
	}

	inline int indexOf(float theta, float phi) const
	{
		return linearizeMultiIndex(multiIndexOf(theta, phi));
	}

	inline Vector2f center(const MultiIndex& mi) const
	{
		Vector2f tp;
		tp[0] = mThetaBasis[mi.first].CenterTheta;
		tp[1] = 2 * PR_PI * mi.second / (float)mThetaBasis[mi.first].PhiCount;
		return tp;
	}

	inline float pdf(const MultiIndex& mi) const
	{
		return 1 / projectedSolidAngle(mi);
	}

	inline Vector2f sample(const Vector2f& uv, const MultiIndex& mi, float& pdf) const
	{
		Vector2f tp;
		tp[0] = mThetaBasis[mi.first].LowerTheta * (1 - uv[0]) + mThetaBasis[mi.first].UpperTheta * uv[0];
		tp[1] = 2 * PR_PI * (mi.second + uv[1]) / (float)mThetaBasis[mi.first].PhiCount;
		pdf	  = this->pdf(mi);
		return tp;
	}

	// Also called lambda or area in some publications
	inline float projectedSolidAngle(const MultiIndex& mi) const
	{
		const auto& tb = mThetaBasis[mi.first];
		return projSA(tb.LowerTheta, tb.UpperTheta, 0, 2 * PR_PI / (float)tb.PhiCount);
	}

	inline float projectedSolidAngle(size_t linear_i) const
	{
		return projectedSolidAngle(multiIndexFromLinear(linear_i));
	}

private:
	std::vector<ThetaBasis> mThetaBasis;
	std::vector<size_t> mThetaLinearOffset;
	size_t mEntryCount = 0;
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

	inline float eval(size_t row, size_t column, const Vector2f& col_angles) const
	{
#ifndef FILTER_KLEMS
		PR_UNUSED(col_angles);
		return mMatrix(row, column);
#else
		constexpr float FILTER_THETA_R = 5 * PR_DEG2RAD;
		constexpr float CENTER_WEIGHT  = 0.5f;
		constexpr int FILTER_COUNT	   = FILTER_KLEMS;
		constexpr float FILTER_PHI_D   = 2 * PR_PI / (FILTER_COUNT - 1);
		constexpr float WEIGHT_F	   = (1 - CENTER_WEIGHT) / (FILTER_COUNT - 1);

		float weight = mMatrix(row, column) * CENTER_WEIGHT;
		PR_UNROLL_LOOP(FILTER_COUNT)
		for (int i = 1; i < FILTER_COUNT; ++i) {
			Vector2f angles = col_angles + Vector2f(-FILTER_THETA_R, (i - 1) * FILTER_PHI_D);
			if (angles(0) < 0)
				angles(0) = -angles(0);
			if (angles(1) > 2 * PR_PI)
				angles(1) -= 2 * PR_PI;

			const int col = mColumnBasis->indexOf(angles(0), angles(1));

			weight += mMatrix(row, col) * WEIGHT_F;
		}
		return weight;
#endif
	}

	// Adapted to Front Outgoing
	inline float eval(const Vector3f& iV, const Vector3f& oV) const
	{
		const Vector2f in  = Spherical::from_direction_hemi(iV);
		const Vector2f out = Spherical::from_direction_hemi(oV);
		const int row	   = mRowBasis->indexOf(out(0), out(1));
		const int col	   = mColumnBasis->indexOf(in(0), in(1));
		return eval(row, col, in);
	}

	inline float pdf(const Vector3f& iV, const Vector3f& oV) const
	{
		const Vector2f in  = Spherical::from_direction_hemi(iV);
		const Vector2f out = Spherical::from_direction_hemi(oV);
		const int row	   = mRowBasis->indexOf(out(0), out(1));
		const int col	   = mColumnBasis->indexOf(in(0), in(1));
		return mColumnCDF[row].discretePdf(col) * mColumnBasis->pdf(mColumnBasis->multiIndexFromLinear(col));
	}

	inline Vector3f sample(const Vector2f& uv, const Vector3f& oV, float& pdf, float& weight) const
	{
		const Vector2f out = Spherical::from_direction_hemi(oV);
		const int row	   = mRowBasis->indexOf(out(0), out(1));

		float u;
		const size_t col = mColumnCDF[row].sampleDiscrete(uv[0], pdf, &u);

		float pdf2;
		const Vector2f in = mColumnBasis->sample(Vector2f(u, uv[1]), mColumnBasis->multiIndexFromLinear(col), pdf2);
		pdf *= pdf2;
		weight = eval(row, col, in);
		return Spherical::cartesian(in(0), in(1));
	}

	inline KlemsMatrix& matrix() { return mMatrix; }
	inline size_t size() const { return mMatrix.size(); }

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

	inline std::shared_ptr<KlemsBasis> row() const { return mRowBasis; }
	inline std::shared_ptr<KlemsBasis> column() const { return mColumnBasis; }

private:
	std::shared_ptr<KlemsBasis> mRowBasis;
	std::shared_ptr<KlemsBasis> mColumnBasis;
	KlemsMatrix mMatrix;
	std::vector<Distribution1D> mColumnCDF;
};

// Scattering Transmission Pair
class KlemsComponentRTPair {
public:
	inline KlemsComponentRTPair(const std::shared_ptr<KlemsComponent>& reflection, const std::shared_ptr<KlemsComponent>& transmission)
		: mReflection(reflection)
		, mTransmission(transmission)
	{
		calcReflectivity();
	}

	inline float evalReflection(const Vector3f& in, const Vector3f& out) const
	{
		return mReflection ? mReflection->eval(in, out) : 0;
	}

	inline float evalTransmission(const Vector3f& in, const Vector3f& out) const
	{
		return mTransmission ? mTransmission->eval(in, out) : 0;
	}

	inline float pdfReflectivity(const Vector3f& in) const
	{
		PR_ASSERT(mTransmission && mReflection, "No reflectivity calculated");
		const Vector2f inA = Spherical::from_direction_hemi(in);
		const size_t row   = mReflection->row()->indexOf(inA(0), inA(1));
		PR_ASSERT(row < mReflectivity.size(), "Invalid row index");
		return mReflectivity[row];
	}

	inline float pdfReflection(const Vector3f& in, const Vector3f& out) const
	{
		float pdf2 = 1.0f;
		if (mTransmission && mReflection)
			pdf2 = pdfReflectivity(in);

		return mReflection ? mReflection->pdf(in, out) * pdf2 : 0;
	}

	inline float pdfTransmission(const Vector3f& in, const Vector3f& out) const
	{
		float pdf2 = 1.0f;
		if (mTransmission && mReflection)
			pdf2 = 1 - pdfReflectivity(in);

		return mTransmission ? mTransmission->pdf(in, out) * pdf2 : 0;
	}

	inline Vector3f sample(const Vector2f& u, const Vector3f& in, float& pdf, float& weight) const
	{
		bool do_reflection = false;
		Vector2f u2		   = u;
		float pdf2		   = 1.0f;
		if (!mTransmission && mReflection) {
			do_reflection = true;
		} else if (mTransmission && !mReflection) {
			do_reflection = false;
		} else if (mTransmission && mReflection) {
			const float refl = pdfReflectivity(in);
			if (u[0] <= refl) {
				do_reflection = true;
				u2[0] /= refl;
				pdf2 = refl;
			} else {
				do_reflection = false;
				u2[0] /= 1 - refl;
				pdf2 = 1 - refl;
			}
		} else { // Nothing present!
			pdf	   = 0;
			weight = 0;
			return Vector3f::Zero();
		}

		Vector3f L;
		if (do_reflection)
			L = fi(mReflection->sample(u2, in, pdf, weight));
		else
			L = bi(mTransmission->sample(u2, in, pdf, weight));
		pdf *= pdf2;
		return L;
	}

private:
	inline void calcReflectivity()
	{
		if (mReflection && mTransmission) {
			PR_ASSERT(mReflection->row()->entryCount() == mTransmission->row()->entryCount(), "Only computable if both components are of same size");
			mReflectivity.resize(mReflection->row()->entryCount());
			const auto rm = mReflection->matrix().rowwise().sum().eval();
			const auto tm = mTransmission->matrix().rowwise().sum().eval();
			PR_OPT_LOOP
			for (size_t i = 0; i < mReflectivity.size(); ++i) {
				const float r = rm[i];
				const float t = tm[i];

				const float n = r + t;
				if (PR_UNLIKELY(n <= PR_EPSILON))
					mReflectivity[i] = r >= t ? 1.0f : 0.0f;
				else
					mReflectivity[i] = r / n;
			}
		}
	}

	std::vector<float> mReflectivity; // For each outgoing patch
	std::shared_ptr<KlemsComponent> mReflection;
	std::shared_ptr<KlemsComponent> mTransmission;
};

enum AllowedComponents {
	AC_FrontReflection	 = 0x1,
	AC_FrontTransmission = 0x2,
	AC_BackReflection	 = 0x4,
	AC_BackTransmission	 = 0x8,
	AC_FrontAll			 = AC_FrontReflection | AC_FrontTransmission,
	AC_BackAll			 = AC_BackReflection | AC_BackTransmission,
	AC_ReflectionAll	 = AC_FrontReflection | AC_BackReflection,
	AC_TransmissionAll	 = AC_FrontTransmission | AC_BackTransmission,
	AC_All				 = AC_FrontAll | AC_BackAll
};
class KlemsMeasurement {
public:
	KlemsMeasurement(const std::filesystem::path& filename, int allowedComponents)
		: mFilename(filename)
		, mGood(false)
	{
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
			PR_LOG(L_ERROR) << "Could not parse " << filename << ": Expected IncidentDataStructure of 'Columns' or 'Rows' but got '" << type << "' instead" << std::endl;
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

		std::shared_ptr<KlemsComponent> reflectionFront;
		std::shared_ptr<KlemsComponent> transmissionFront;
		std::shared_ptr<KlemsComponent> reflectionBack;
		std::shared_ptr<KlemsComponent> transmissionBack;
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
			if (direction == "Transmission Front" && (allowedComponents & AC_FrontTransmission))
				transmissionFront = component;
			else if (direction == "Scattering Back" && (allowedComponents & AC_BackReflection))
				reflectionBack = component;
			else if (direction == "Transmission Back" && (allowedComponents & AC_BackTransmission))
				transmissionBack = component;
			else if (allowedComponents & AC_FrontReflection)
				reflectionFront = component;
		}

		// Make sure both transmission parts are equal if not specified otherwise
		if (!transmissionBack)
			transmissionBack = transmissionFront;
		if (!transmissionFront)
			transmissionFront = transmissionBack;

		mFront = std::make_shared<KlemsComponentRTPair>(reflectionFront, transmissionFront);
		mBack  = std::make_shared<KlemsComponentRTPair>(reflectionBack, transmissionBack);

		if (!reflectionFront && !reflectionBack && !transmissionFront && !transmissionBack) {
			PR_LOG(L_ERROR) << "Could not parse " << filename << ": No valid data found" << std::endl;
			mGood = false;
			return;
		}

		if (!transmissionFront && !transmissionBack)
			PR_LOG(L_WARNING) << "While parsing " << filename << ": No transmission data found" << std::endl;

		mGood = true;
	}

	inline const std::filesystem::path& filename() const { return mFilename; }
	inline bool isValid() const { return mGood; }

	inline float eval(const Vector3f& in, const Vector3f& out) const
	{
		const bool outFront = out(2) > 0;
		const bool inFront	= in(2) > 0;

		if (inFront && outFront)
			return mFront->evalReflection(fi(in), fo(out));
		else if (!inFront && !outFront)
			return mBack->evalReflection(bi(in), bo(out));
		else if (inFront)
			return mFront->evalTransmission(fi(in), bo(out));
		else
			return mBack->evalTransmission(bi(in), fo(out));
	}

	inline float pdf(const Vector3f& in, const Vector3f& out) const
	{
		const bool outFront = out(2) > 0;
		const bool inFront	= in(2) > 0;

		if (inFront && outFront)
			return mFront->pdfReflection(fi(in), fo(out));
		else if (!inFront && !outFront)
			return mBack->pdfReflection(bi(in), bo(out));
		else if (inFront)
			return mFront->pdfTransmission(fi(in), bo(out));
		else
			return mBack->pdfTransmission(bi(in), fo(out));
	}

	inline Vector3f sample(const Vector2f& u, const Vector3f& in, float& pdf, float& weight) const
	{
		if (in(2) >= 0)
			return mFront->sample(u, in, pdf, weight);
		else
			return -mBack->sample(u, -in, pdf, weight);
	}

private:
	const std::filesystem::path mFilename;
	bool mGood;
	std::shared_ptr<KlemsComponentRTPair> mFront;
	std::shared_ptr<KlemsComponentRTPair> mBack;
};

template <bool SwapSide, bool HasTint>
class KlemsMeasuredMaterial : public IMaterial {
public:
	KlemsMeasuredMaterial(const KlemsMeasurement& measurement, const std::shared_ptr<FloatSpectralNode>& tint)
		: IMaterial()
		, mMeasurement(measurement)
		, mTint(tint)
	{

#if 0
		// Ignore this... works only with aerc
		Vector3f i = Vector3f(0, 0, 1).normalized();
		Vector3f o = Vector3f(0, 1, 1).normalized();
		float a1   = mMeasurement.eval(i, o);		  // 4.933000e-02
		float a2   = mMeasurement.eval(bi(i), o);	  // 6.322000e-03
		float a3   = mMeasurement.eval(i, bo(o));	  // 1.003000e-02
		float a4   = mMeasurement.eval(bi(i), bo(o)); // 0
		PR_LOG(L_INFO) << "M> " << a1 << " " << a2 << " " << a3 << " " << a4 << std::endl;
#endif
	}

	virtual ~KlemsMeasuredMaterial() = default;

	void eval(const MaterialEvalInput& in, MaterialEvalOutput& out,
			  const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		Vector3f oV = in.Context.V;
		Vector3f iV = in.Context.L;

		if constexpr (SwapSide) {
			oV = -oV;
			iV = -iV;
		}

		const bool refraction = !in.Context.V.sameHemisphere(in.Context.L);
		out.Weight			  = mMeasurement.eval(oV, iV) /* std::abs(in.Context.NdotL())*/;
		out.PDF_S			  = mMeasurement.pdf(oV, iV);
		out.Type			  = refraction ? MaterialScatteringType::DiffuseTransmission : MaterialScatteringType::DiffuseReflection;

		if constexpr (HasTint)
			out.Weight *= mTint->eval(in.ShadingContext);
	}

	void pdf(const MaterialEvalInput& in, MaterialPDFOutput& out,
			 const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		Vector3f oV = in.Context.V;
		Vector3f iV = in.Context.L;

		if constexpr (SwapSide) {
			oV = -oV;
			iV = -iV;
		}

		out.PDF_S = mMeasurement.pdf(oV, iV);
	}

	void sample(const MaterialSampleInput& in, MaterialSampleOutput& out,
				const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		float pdf;
		float weight;

		if constexpr (SwapSide)
			out.L = (Vector3f)-mMeasurement.sample(in.RND.get2D(), -in.Context.V, pdf, weight);
		else
			out.L = mMeasurement.sample(in.RND.get2D(), in.Context.V, pdf, weight);

		const bool refraction = !in.Context.V.sameHemisphere(out.L);
		out.IntegralWeight	  = weight / pdf /* std::abs(ectx.NdotL())*/;
		out.PDF_S			  = pdf;
		out.Type			  = refraction ? MaterialScatteringType::DiffuseTransmission : MaterialScatteringType::DiffuseReflection;

		if constexpr (HasTint)
			out.IntegralWeight *= mTint->eval(in.ShadingContext);
	}

	std::string dumpInformation() const override
	{
		std::stringstream stream;

		stream << std::boolalpha << IMaterial::dumpInformation()
			   << "  <KlemsMeasuredMaterial>:" << std::endl
			   << "    Filename: " << mMeasurement.filename() << std::endl
			   << "    Tint:     " << (HasTint ? mTint->dumpInformation() : "None") << std::endl
			   << "    SwapSide: " << (SwapSide ? "true" : "false") << std::endl;

		return stream.str();
	}

private:
	const KlemsMeasurement mMeasurement;
	const std::shared_ptr<FloatSpectralNode> mTint;
};

class KlemsMeasuredMaterialPlugin : public IMaterialPlugin {
public:
	std::shared_ptr<IMaterial> create(const std::string&, const SceneLoadContext& ctx) override
	{
		const ParameterGroup& params = ctx.parameters();
		int allowedComponents		 = AC_All;
		if (params.getBool("no_front_reflection", false))
			allowedComponents &= ~AC_FrontReflection;
		if (params.getBool("no_front_transmission", false))
			allowedComponents &= ~AC_FrontTransmission;
		if (params.getBool("no_back_reflection", false))
			allowedComponents &= ~AC_BackReflection;
		if (params.getBool("no_back_transmission", false))
			allowedComponents &= ~AC_BackTransmission;
		if (params.getBool("no_front", false))
			allowedComponents &= ~AC_FrontAll;
		if (params.getBool("no_back", false))
			allowedComponents &= ~AC_BackAll;
		if (params.getBool("no_reflection", false))
			allowedComponents &= ~AC_ReflectionAll;
		if (params.getBool("no_transmission", false))
			allowedComponents &= ~AC_TransmissionAll;

		if (allowedComponents == 0)
			PR_LOG(L_WARNING) << "No allowed components in the Klems BSDF, therefor it will be black. Are you sure you want this?" << std::endl;

		KlemsMeasurement measurement(ctx.escapePath(params.getString("filename", "")), allowedComponents);
		const bool swapSide = params.getBool("swap_side", false);

		if (measurement.isValid()) {
			if (params.hasParameter("tint")) {
				auto tint = ctx.lookupSpectralNode("tint", 1);
				if (swapSide)
					return std::make_shared<KlemsMeasuredMaterial<true, true>>(measurement, tint);
				else
					return std::make_shared<KlemsMeasuredMaterial<false, true>>(measurement, tint);
			} else {
				if (swapSide)
					return std::make_shared<KlemsMeasuredMaterial<true, false>>(measurement, nullptr);
				else
					return std::make_shared<KlemsMeasuredMaterial<false, false>>(measurement, nullptr);
			}
		}
		return nullptr;
	}

	const std::vector<std::string>& getNames() const override
	{
		const static std::vector<std::string> names({ "klems", "klems_measured", "klems-measured" });
		return names;
	}

	PluginSpecification specification(const std::string&) const override
	{
		return PluginSpecificationBuilder("Klems BSDF", "Klems BSDF")
			.Identifiers(getNames())
			.Inputs()
			.Bool("no_front_reflection", "No front reflection", false)
			.Bool("no_front_transmission", "No front transmission", false)
			.Bool("no_back_reflection", "No back reflection", false)
			.Bool("no_back_transmission", "No back transmission", false)
			.Bool("no_front", "No front reflection or transmission", false)
			.Bool("no_back", "No back reflection or transmission", false)
			.Bool("no_reflection", "No reflection", false)
			.Bool("no_transmission", "No transmission", false)
			.Bool("swap_side", "Swap front and back", false)
			.Filename("filename", "A path to a file given as a Radiance type XML")
			.SpectralNode("tint", "Tint", true)
			.Specification()
			.get();
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::KlemsMeasuredMaterialPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)