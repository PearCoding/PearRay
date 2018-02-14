namespace PR {

// Simple Properties
inline uint32 Spectrum::samples() const
{
	return spectralEnd() - spectralStart();
}

inline uint32 Spectrum::spectralStart() const
{
	return mInternal->Start;
}

inline uint32 Spectrum::spectralEnd() const
{
	return mInternal->End;
}

inline const std::shared_ptr<SpectrumDescriptor>& Spectrum::descriptor() const
{
	return mInternal->Descriptor;
}

inline bool Spectrum::isExternal() const
{
	return mInternal->External;
}

// Simple Access
inline void Spectrum::setValue(uint32 index, float v)
{
	PR_ASSERT(index < samples(), "Bad access");
	mInternal->Data[index] = v;
}

inline float Spectrum::value(uint32 index) const
{
	PR_ASSERT(index < samples(), "Bad access");
	return mInternal->Data[index];
}

inline const float& Spectrum::operator[](uint32 index) const
{
	PR_ASSERT(index < samples(), "Bad access");
	return mInternal->Data[index];
}

inline float& Spectrum::operator[](uint32 index)
{
	PR_ASSERT(index < samples(), "Bad access");
	return mInternal->Data[index];
}

inline float* Spectrum::ptr()
{
	return mInternal->Data;
}

inline const float* Spectrum::c_ptr() const
{
	return mInternal->Data;
}

// Memory management -> Dangerous functions!
inline void Spectrum::copyFrom(const float* data)
{
	for (uint32 i = 0; i < samples(); ++i) {
		ptr()[i] = data[i];
	}
}

inline void Spectrum::copyTo(float* data) const
{
	for (uint32 i = 0; i < samples(); ++i) {
		data[i] = c_ptr()[i];
	}
}

// Operators
inline Spectrum Spectrum::operator+(const Spectrum& spec) const
{
	Spectrum tmp = clone();
	tmp += spec;
	return tmp;
}

inline Spectrum& Spectrum::operator+=(const Spectrum& spec)
{
	PR_ASSERT(spec.samples() == samples(), "Need same spectrum types");

	for (uint32 i = 0; i < samples(); ++i)
		setValue(i, value(i) + spec.value(i));

	return *this;
}
inline Spectrum Spectrum::operator-(const Spectrum& spec) const
{
	Spectrum tmp = clone();
	tmp -= spec;
	return tmp;
}

inline Spectrum& Spectrum::operator-=(const Spectrum& spec)
{
	PR_ASSERT(spec.samples() == samples(), "Need same spectrum types");

	for (uint32 i = 0; i < samples(); ++i)
		setValue(i, value(i) - spec.value(i));

	return *this;
}

inline Spectrum Spectrum::operator*(const Spectrum& spec) const
{
	Spectrum tmp = clone();
	tmp *= spec;
	return tmp;
}
inline Spectrum Spectrum::operator*(float f) const
{
	Spectrum tmp = clone();
	tmp *= f;
	return tmp;
}
inline Spectrum& Spectrum::operator*=(const Spectrum& spec)
{
	PR_ASSERT(spec.samples() == samples(), "Need same spectrum types");

	for (uint32 i = 0; i < samples(); ++i)
		setValue(i, value(i) * spec.value(i));

	return *this;
}

inline Spectrum& Spectrum::operator*=(float f)
{
	for (uint32 i = 0; i < samples(); ++i)
		setValue(i, value(i) * f);

	return *this;
}

inline Spectrum Spectrum::operator/(const Spectrum& spec) const
{
	Spectrum tmp = clone();
	tmp /= spec;
	return tmp;
}
inline Spectrum Spectrum::operator/(float f) const
{
	Spectrum tmp = clone();
	tmp /= f;
	return tmp;
}

inline Spectrum& Spectrum::operator/=(const Spectrum& spec)
{
	PR_ASSERT(spec.samples() == samples(), "Need same spectrum types");

	for (uint32 i = 0; i < samples(); ++i)
		setValue(i, value(i) / spec.value(i));

	return *this;
}

inline Spectrum& Spectrum::operator/=(float f)
{
	for (uint32 i = 0; i < samples(); ++i)
		setValue(i, value(i) / f);

	return *this;
}

// Fill Functions
inline void Spectrum::fill(float v)
{
	for (uint32 i = 0; i < samples(); ++i) {
		setValue(i, v);
	}
}

inline void Spectrum::fill(uint32 si, uint32 ei, float v)
{
	for (uint32 i = si; i < samples() && i < ei; ++i) {
		setValue(i, v);
	}
}

inline void Spectrum::clear()
{
	fill(0);
}

// Apply Functions
inline float Spectrum::max() const
{
	float h = 0;

	for (uint32 i = 0; i < samples(); ++i) {
		if (h < std::abs(value(i)))
			h = std::abs(value(i));
	}

	return h;
}

inline float Spectrum::min() const
{
	float h = std::numeric_limits<float>::max();

	for (uint32 i = 0; i < samples(); ++i) {
		if (h > std::abs(value(i)))
			h = std::abs(value(i));
	}

	return h;
}

inline float Spectrum::avg() const
{
	float h = 0;
	for (uint32 i = 0; i < samples(); ++i)
		h += value(i);

	return h / samples();
}

inline float Spectrum::sum() const
{
	float h = 0;
	for (uint32 i = 0; i < samples(); ++i)
		h += value(i);

	return h;
}

inline float Spectrum::sqrSum() const
{
	float h = 0;
	for (uint32 i = 0; i < samples(); ++i)
		h += value(i) * value(i);

	return h;
}

// Detect Functions
inline bool Spectrum::hasNaN() const
{
	for (uint32 i = 0; i < samples(); ++i) {
		if (std::isnan(value(i)))
			return true;
	}

	return false;
}

inline bool Spectrum::hasInf() const
{
	for (uint32 i = 0; i < samples(); ++i) {
		if (std::isinf(value(i)))
			return true;
	}

	return false;
}

inline bool Spectrum::hasNegative() const
{
	for (uint32 i = 0; i < samples(); ++i) {
		if (value(i) < 0.0f)
			return true;
	}

	return false;
}

inline bool Spectrum::isOnlyZero() const
{
	for (uint32 i = 0; i < samples(); ++i) {
		if (std::abs(value(i)) <= PR_EPSILON)
			return false;
	}

	return true;
}

// Vector Operations
inline void Spectrum::normalize()
{
	float h = max();

	if (h > PR_EPSILON) {
		float sh = 1 / h;
		for (uint32 i = 0; i < samples(); ++i)
			setValue(i, value(i) * sh);
	}
}

inline void Spectrum::clamp(float start, float end)
{
	for (uint32 i = 0; i < samples(); ++i)
		setValue(i, value(i) > end ? end : (value(i) < start ? start : value(i)));
}

inline void Spectrum::lerp(const Spectrum& spec, float t)
{
	PR_ASSERT(spec.samples() == samples(), "Need same spectrum types");

	for (uint32 i = 0; i < samples(); ++i)
		setValue(i, value(i) * (1 - t) + spec.value(i) * t);
}

inline void Spectrum::sqrt()
{
	for (uint32 i = 0; i < samples(); ++i)
		setValue(i, std::sqrt(value(i)));
}

inline Spectrum Spectrum::normalized() const
{
	Spectrum spec = clone();
	spec.normalize();
	return spec;
}

inline Spectrum Spectrum::clamped(float start, float end) const
{
	Spectrum spec = clone();
	spec.clamp(start, end);
	return spec;
}

inline Spectrum Spectrum::lerp(const Spectrum& spec1, const Spectrum& spec2, float t)
{
	Spectrum spec = spec1.clone();
	spec.lerp(spec2, t);
	return spec;
}

inline Spectrum Spectrum::sqrted() const
{
	Spectrum spec = clone();
	spec.sqrt();
	return spec;
}

// Standard Methods
inline Spectrum Spectrum::black(const std::shared_ptr<SpectrumDescriptor>& desc)
{
	return Spectrum::gray(desc, 0.0f);
}

inline Spectrum Spectrum::white(const std::shared_ptr<SpectrumDescriptor>& desc)
{
	return Spectrum::gray(desc, 1.0f);
}

inline Spectrum Spectrum::gray(const std::shared_ptr<SpectrumDescriptor>& desc, float f)
{
	return Spectrum(desc, f);
}

// Global
inline bool operator==(const Spectrum& v1, const Spectrum& v2)
{
	if (v1.samples() != v2.samples())
		return false;

	for (uint32 i = 0; i < v1.samples(); ++i) {
		if (v1.value(i) != v2.value(i))
			return false;
	}
	return true;
}

inline bool operator!=(const Spectrum& v1, const Spectrum& v2)
{
	return !(v1 == v2);
}

inline Spectrum operator*(float f, const Spectrum& spec)
{
	return spec * f;
}

inline Spectrum operator/(float f, const Spectrum& spec)
{
	Spectrum tmp = spec.clone();

	for (uint32 i = 0; i < spec.samples(); ++i)
		tmp.setValue(i, f / spec.value(i));

	return tmp;
}
} // namespace PR
