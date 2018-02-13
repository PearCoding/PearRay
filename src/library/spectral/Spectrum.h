#pragma once

#include "PR_Config.h"

#include <algorithm>

namespace PR {

class SpectrumDescriptor;
class PR_LIB Spectrum {
public:
	explicit Spectrum(const SpectrumDescriptor* descriptor);
	Spectrum(const SpectrumDescriptor* descriptor, float initial);
	Spectrum(const SpectrumDescriptor* descriptor, uint32 start, uint32 end);
	Spectrum(const SpectrumDescriptor* descriptor, uint32 start, uint32 end, float initial);
	Spectrum(const SpectrumDescriptor* descriptor, uint32 start, uint32 end, float* data);

	virtual ~Spectrum() = default;

	// Copy/Move operations
	Spectrum(const Spectrum& other) = default;
	Spectrum(Spectrum&& other) = default;

	Spectrum& operator=(const Spectrum& other) = default;
	Spectrum& operator=(Spectrum&& other) = default;

	Spectrum clone() const;

	// Simple Properties
	inline uint32 samples() const;
	inline uint32 spectralStart() const;
	inline uint32 spectralEnd() const;

	inline const SpectrumDescriptor* descriptor() const;
	inline bool isExternal() const;

	// Simple Access
	inline void setValue(uint32 index, float v);
	inline float value(uint32 index) const;

	inline const float& operator[](uint32 index) const;
	inline float& operator[](uint32 index);

	inline float* ptr();
	inline const float* c_ptr() const;

	// Memory management -> Dangerous functions!
	inline void copyFrom(const float* data);
	inline void copyTo(float* data) const;

	// Operators
	inline Spectrum operator+(const Spectrum& spec) const;
	inline Spectrum& operator+=(const Spectrum& spec);

	inline Spectrum operator-(const Spectrum& spec) const;
	inline Spectrum& operator-=(const Spectrum& spec);

	inline Spectrum operator*(const Spectrum& spec) const; // Element wise
	inline Spectrum operator*(float f) const;
	inline Spectrum& operator*=(const Spectrum& spec);
	inline Spectrum& operator*=(float f);

	inline Spectrum operator/(const Spectrum& spec) const; // Element wise
	inline Spectrum operator/(float f) const;
	inline Spectrum& operator/=(const Spectrum& spec);
	inline Spectrum& operator/=(float f);

	// Fill Functions
	inline void fill(float v);
	inline void fill(uint32 si, uint32 ei, float v);
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
	inline bool isOnlyZero() const;

	// Vector Operations
	inline void normalize();
	inline void clamp(float start = 0, float end = 1);
	inline void lerp(const Spectrum& spec, float t);
	inline void sqrt();

	inline Spectrum normalized() const;
	inline Spectrum clamped(float start = 0, float end = 1) const;
	inline static Spectrum lerp(const Spectrum& spec1, const Spectrum& spec2, float t);
	inline Spectrum sqrted() const;

	// Photometric Operations
	void weightPhotometric();
	float luminousFlux_nm() const;
	inline double luminousFlux() const { return luminousFlux_nm() * PR_NM_TO_M; }

private:
	struct Spectrum_Internal {
		const SpectrumDescriptor* Descriptor;
		uint32 Start;
		uint32 End;
		bool External;
		float* Data;
		
		Spectrum_Internal(const SpectrumDescriptor* descriptor, uint32 start, uint32 end, float* data);
		Spectrum_Internal(const SpectrumDescriptor* descriptor, uint32 start, uint32 end);
		~Spectrum_Internal();
	};
	std::shared_ptr<Spectrum_Internal> mInternal;
};

inline Spectrum operator*(float f, const Spectrum& spec);
inline Spectrum operator/(float f, const Spectrum& spec);

inline bool operator==(const Spectrum& v1, const Spectrum& v2);
inline bool operator!=(const Spectrum& v1, const Spectrum& v2);
}

#include "Spectrum.inl"
