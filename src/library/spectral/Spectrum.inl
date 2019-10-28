namespace PR {

// Simple Properties
inline size_t Spectrum::samples() const
{
	return spectralEnd() - spectralStart();
}

inline size_t Spectrum::spectralStart() const
{
	return mInternal->Start;
}

inline size_t Spectrum::spectralEnd() const
{
	return mInternal->End;
}

inline std::shared_ptr<SpectrumDescriptor> Spectrum::descriptor() const
{
	return mInternal->Descriptor;
}

inline bool Spectrum::isExternal() const
{
	return mInternal->External;
}

// Simple Access
inline void Spectrum::setValue(size_t index, float v)
{
	PR_ASSERT(index < samples(), "Bad access");
	mInternal->Data[index] = v;
}

inline float Spectrum::value(size_t index) const
{
	PR_ASSERT(index < samples(), "Bad access");
	return mInternal->Data[index];
}

inline const float& Spectrum::operator[](size_t index) const
{
	PR_ASSERT(index < samples(), "Bad access");
	return mInternal->Data[index];
}

inline float& Spectrum::operator[](size_t index)
{
	PR_ASSERT(index < samples(), "Bad access");
	return mInternal->Data[index];
}

inline const float& Spectrum::operator()(size_t index) const
{
	PR_ASSERT(index < samples(), "Bad access");
	return mInternal->Data[index];
}

inline float& Spectrum::operator()(size_t index)
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

inline void Spectrum::copyTo(Spectrum& spec) const
{
	PR_ASSERT(spec.samples() == samples(), "Can not assign data to non equal sized spectrum");
	copyTo(spec.ptr());
}

// Operators
inline Spectrum& Spectrum::operator+=(const Spectrum& spec)
{
	PR_ASSERT(spec.samples() == samples(), "Need same spectrum types");

	for (size_t i = 0; i < samples(); ++i)
		setValue(i, value(i) + spec.value(i));

	return *this;
}

inline Spectrum& Spectrum::operator-=(const Spectrum& spec)
{
	PR_ASSERT(spec.samples() == samples(), "Need same spectrum types");

	for (size_t i = 0; i < samples(); ++i)
		setValue(i, value(i) - spec.value(i));

	return *this;
}

inline Spectrum& Spectrum::operator*=(const Spectrum& spec)
{
	PR_ASSERT(spec.samples() == samples(), "Need same spectrum types");

	for (size_t i = 0; i < samples(); ++i)
		setValue(i, value(i) * spec.value(i));

	return *this;
}

inline Spectrum& Spectrum::operator*=(float f)
{
	for (size_t i = 0; i < samples(); ++i)
		setValue(i, value(i) * f);

	return *this;
}

inline Spectrum& Spectrum::operator/=(const Spectrum& spec)
{
	PR_ASSERT(spec.samples() == samples(), "Need same spectrum types");

	for (size_t i = 0; i < samples(); ++i)
		setValue(i, value(i) / spec.value(i));

	return *this;
}

inline Spectrum& Spectrum::operator/=(float f)
{
	const float inv = 1.0f / f;
	return (*this *= inv);
}

// Fill Functions
inline void Spectrum::fill(float v)
{
	for (uint32 i = 0; i < samples(); ++i) {
		setValue(i, v);
	}
}

inline void Spectrum::fill(size_t si, size_t ei, float v)
{
	for (size_t i = si; i < samples() && i < ei; ++i) {
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

	for (size_t i = 0; i < samples(); ++i) {
		if (h < std::abs(value(i)))
			h = std::abs(value(i));
	}

	return h;
}

inline float Spectrum::min() const
{
	float h = std::numeric_limits<float>::max();

	for (size_t i = 0; i < samples(); ++i) {
		if (h > std::abs(value(i)))
			h = std::abs(value(i));
	}

	return h;
}

inline float Spectrum::avg() const
{
	float h = 0;
	for (size_t i = 0; i < samples(); ++i)
		h += value(i);

	return h / samples();
}

inline float Spectrum::sum() const
{
	float h = 0;
	for (size_t i = 0; i < samples(); ++i)
		h += value(i);

	return h;
}

inline float Spectrum::sqrSum() const
{
	float h = 0;
	for (size_t i = 0; i < samples(); ++i)
		h += value(i) * value(i);

	return h;
}

// Detect Functions
inline bool Spectrum::hasNaN() const
{
	for (size_t i = 0; i < samples(); ++i) {
		if (std::isnan(value(i)))
			return true;
	}

	return false;
}

inline bool Spectrum::hasInf() const
{
	for (size_t i = 0; i < samples(); ++i) {
		if (std::isinf(value(i)))
			return true;
	}

	return false;
}

inline bool Spectrum::hasNegative() const
{
	for (size_t i = 0; i < samples(); ++i) {
		if (value(i) < 0.0f)
			return true;
	}

	return false;
}

inline bool Spectrum::isOnlyZero(float eps) const
{
	for (size_t i = 0; i < samples(); ++i) {
		if (std::abs(value(i)) > eps)
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
		for (size_t i = 0; i < samples(); ++i)
			setValue(i, value(i) * sh);
	}
}

inline void Spectrum::clamp(float start, float end)
{
	for (size_t i = 0; i < samples(); ++i)
		setValue(i, value(i) > end ? end : (value(i) < start ? start : value(i)));
}

inline void Spectrum::sqrt()
{
	for (size_t i = 0; i < samples(); ++i)
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

// SLO
template <typename T>
inline std::enable_if_t<Lazy::is_slo<T>::value, Spectrum&> Spectrum::operator=(const T& slo)
{
	for (size_t i = 0; i < samples(); ++i) {
		setValue(i, slo(i));
	}
	return *this;
}

template <typename T>
inline std::enable_if_t<Lazy::is_slo<T>::value, Spectrum&> Spectrum::operator+=(const T& slo)
{
	for (size_t i = 0; i < samples(); ++i) {
		setValue(i, value(i) + slo(i));
	}
	return *this;
}

template <typename T>
inline std::enable_if_t<Lazy::is_slo<T>::value, Spectrum&> Spectrum::operator-=(const T& slo)
{
	for (size_t i = 0; i < samples(); ++i) {
		setValue(i, value(i) - slo(i));
	}
	return *this;
}

template <typename T>
inline std::enable_if_t<Lazy::is_slo<T>::value, Spectrum&> Spectrum::operator*=(const T& slo)
{
	for (size_t i = 0; i < samples(); ++i) {
		setValue(i, value(i) * slo(i));
	}
	return *this;
}

template <typename T>
inline std::enable_if_t<Lazy::is_slo<T>::value, Spectrum&> Spectrum::operator/=(const T& slo)
{
	for (size_t i = 0; i < samples(); ++i) {
		setValue(i, value(i) / slo(i));
	}
	return *this;
}

template <typename T>
inline Lazy::enable_if_slo_t<T, T, void> Spectrum::lerp(const T& slo, float t)
{
	for (size_t i = 0; i < samples(); ++i) {
		setValue(i, value(i) * (1 - t) + slo(i) * t);
	}
}

template <typename T>
inline Lazy::enable_if_slo_t<T, T, void> Spectrum::copyFrom(const T& slo)
{
	for (size_t i = 0; i < samples(); ++i) {
		setValue(i, slo(i));
	}
}

inline std::ostream& operator<<(std::ostream& o, const Spectrum& spec)
{
	o << "[";
	for (size_t i = 0; i < spec.samples() - 1; ++i) {
		o << spec[i] << " ";
	}
	o << spec[spec.samples() - 1] << "]";

	return o;
}
} // namespace PR
