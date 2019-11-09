#pragma once

#include "SpectrumLazyOperator.h"

#include <algorithm>

namespace PR {

class SpectrumDescriptor;
class PR_LIB Spectrum {
public:
	explicit Spectrum(const std::shared_ptr<SpectrumDescriptor>& descriptor);
	Spectrum(const std::shared_ptr<SpectrumDescriptor>& descriptor, float initial);
	Spectrum(const std::shared_ptr<SpectrumDescriptor>& descriptor, size_t start, size_t end);
	Spectrum(const std::shared_ptr<SpectrumDescriptor>& descriptor, size_t start, size_t end, float initial);
	Spectrum(const std::shared_ptr<SpectrumDescriptor>& descriptor, size_t start, size_t end, float* data);

	virtual ~Spectrum() = default;

	// Copy/Move operations
	Spectrum(const Spectrum& other) = default;
	Spectrum(Spectrum&& other)		= default;

	Spectrum& operator=(const Spectrum& other) = default;
	Spectrum& operator=(Spectrum&& other) = default;

	Spectrum clone() const;

	// Simple Properties
	inline size_t samples() const;
	inline size_t spectralStart() const;
	inline size_t spectralEnd() const;

	inline std::shared_ptr<SpectrumDescriptor> descriptor() const;
	inline bool isExternal() const;

	// Simple Access
	inline void setValue(size_t index, float v);
	inline float value(size_t index) const;

	inline const float& operator[](size_t index) const;
	inline float& operator[](size_t index);

	inline const float& operator()(size_t index) const;
	inline float& operator()(size_t index);

	inline float* ptr();
	inline const float* c_ptr() const;

	// Memory management -> Dangerous functions!
	inline void copyFrom(const float* data);
	inline void copyTo(float* data) const;
	inline void copyTo(Spectrum& spec) const;

	// Operators
	inline Spectrum& operator+=(const Spectrum& spec);
	inline Spectrum& operator-=(const Spectrum& spec);
	inline Spectrum& operator*=(const Spectrum& spec);
	inline Spectrum& operator*=(float f);
	inline Spectrum& operator/=(const Spectrum& spec);
	inline Spectrum& operator/=(float f);

	// Fill Functions
	inline void fill(float v);
	inline void fill(size_t si, size_t ei, float v);
	inline void clear();

	// Apply Functions
	inline float max() const;	// Amplitude
	inline float min() const;	// Amplitude
	inline float avg() const;	// General average
	inline float sum() const;	// Sum
	inline float sqrSum() const; // Squared sum

	// Detect Functions
	inline bool hasNaN() const;
	inline bool hasInf() const;
	inline bool hasNegative() const;
	inline bool isOnlyZero(float eps = PR_EPSILON) const;

	// Vector Operations
	inline void normalize();
	inline void clamp(float start = 0, float end = 1);
	inline void sqrt();

	inline Spectrum normalized() const;
	inline Spectrum clamped(float start = 0, float end = 1) const;
	inline Spectrum sqrted() const;

	// Photometric Operations
	void weightPhotometric();
	float luminousFlux_nm() const;
	inline double luminousFlux() const { return luminousFlux_nm() * PR_NM_TO_M; }
	float relativeLuminance() const;

	// Standard Constructor
	inline static Spectrum black(const std::shared_ptr<SpectrumDescriptor>& desc);
	inline static Spectrum white(const std::shared_ptr<SpectrumDescriptor>& desc);
	inline static Spectrum gray(const std::shared_ptr<SpectrumDescriptor>& desc, float f);

	static Spectrum blackbody(const std::shared_ptr<SpectrumDescriptor>& desc, float temp); // Temp in Kelvin (K), Output W·sr^−1·m^−3

	// SLO
	template <typename T, typename = std::enable_if_t<Lazy::is_slo<T>::value>>
	inline Spectrum(const T& slo)
		: Spectrum(slo.descriptor(), slo.spectralStart(), slo.spectralEnd())
	{
		for (size_t i = 0; i < samples(); ++i) {
			setValue(i, slo(i));
		}
	}

	template <typename T>
	inline std::enable_if_t<Lazy::is_slo<T>::value, Spectrum&> operator=(const T& slo);
	template <typename T>
	inline std::enable_if_t<Lazy::is_slo<T>::value, Spectrum&> operator+=(const T& slo);
	template <typename T>
	inline std::enable_if_t<Lazy::is_slo<T>::value, Spectrum&> operator-=(const T& slo);
	template <typename T>
	inline std::enable_if_t<Lazy::is_slo<T>::value, Spectrum&> operator*=(const T& slo);
	template <typename T>
	inline std::enable_if_t<Lazy::is_slo<T>::value, Spectrum&> operator/=(const T& slo);

	template <typename T>
	inline Lazy::enable_if_slo_t<T, T, void> lerp(const T& slo, float t);

	template <typename T1, typename T2, typename = Lazy::enable_if_slo_t<T1, T2, void>>
	inline static auto lerp(const T1& spec1, const T2& spec2, float t)
	{
		return spec1 * (1 - t) + spec2 * t;
	}

	template <typename T>
	inline Lazy::enable_if_slo_t<T, T, void> copyFrom(const T& slo);

private:
	struct Spectrum_Internal {
		std::shared_ptr<SpectrumDescriptor> Descriptor;
		size_t Start;
		size_t End;
		bool External;
		float* Data;

		Spectrum_Internal(const std::shared_ptr<SpectrumDescriptor>& descriptor, size_t start, size_t end, float* data);
		Spectrum_Internal(const std::shared_ptr<SpectrumDescriptor>& descriptor, size_t start, size_t end);
		~Spectrum_Internal();

		Spectrum_Internal(const Spectrum_Internal& other) = default;
		Spectrum_Internal(Spectrum_Internal&& other)	  = default;
		Spectrum_Internal& operator=(const Spectrum_Internal& other) = default;
		Spectrum_Internal& operator=(Spectrum_Internal&& other) = default;
	};
	std::shared_ptr<Spectrum_Internal> mInternal;
};

inline std::ostream& operator<<(std::ostream& o, const Spectrum& spec);

} // namespace PR

#include "Spectrum.inl"
