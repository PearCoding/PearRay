namespace PR {

template <typename T, size_t D, size_t P>
template <typename U>
inline Lazy::enable_if_adlo_vector1_t<U, AutoDiff_Vector<T, D, P>&>
AutoDiff_Vector<T, D, P>::operator+=(const U& other)
{
	this->mValue += other.v();
	for (size_t i = 0; i < P; ++i) {
		this->mGradients[i] += other.d(i);
	}
	return *this;
}

template <typename T, size_t D, size_t P>
template <typename U>
inline Lazy::enable_if_adlo_vector1_t<U, AutoDiff_Vector<T, D, P>&>
AutoDiff_Vector<T, D, P>::operator-=(const U& other)
{
	this->mValue -= other.v();
	for (size_t i = 0; i < P; ++i) {
		this->mGradients[i] -= other.d(i);
	}
	return *this;
}

template <typename T, size_t D, size_t P>
template <typename U>
inline Lazy::enable_if_adlo_vector1_t<U, AutoDiff_Vector<T, D, P>&>
AutoDiff_Vector<T, D, P>::operator*=(const U& other)
{
	const auto v = other.v();
	for (size_t i = 0; i < P; ++i) {
		this->mGradients[i] = other.d(i).cwiseProduct(this->mValue) + this->mGradients[i].cwiseProduct(v);
	}

	this->mValue = this->mValue.cwiseProduct(v);
	return *this;
}

template <typename T, size_t D, size_t P>
inline AutoDiff_Vector<T, D, P>&
AutoDiff_Vector<T, D, P>::operator*=(const scalar_t& other)
{
	for (size_t i = 0; i < P; ++i) {
		this->mGradients[i] *= other;
	}

	this->mValue *= other;
	return *this;
}

template <typename T, size_t D, size_t P>
template <typename U>
inline Lazy::enable_if_adlo_vector1_t<U, AutoDiff_Vector<T, D, P>&>
AutoDiff_Vector<T, D, P>::operator/=(const U& other)
{
	const auto v	  = other.v();
	const value_t inv = v.cwiseProduct(v).cwiseInverse();

	for (size_t i = 0; i < P; ++i) {
		this->mGradients[i] = (-other.d(i).cwiseProduct(this->mValue) + this->mGradients[i].cwiseProduct(v)).cwiseProduct(inv);
	}

	this->mValue = this->mValue.cwiseQuotient(v);

	return *this;
}

template <typename T, size_t D, size_t P>
inline AutoDiff_Vector<T, D, P>&
AutoDiff_Vector<T, D, P>::operator/=(const scalar_t& other)
{
	for (size_t i = 0; i < P; ++i) {
		this->mGradients[i] /= other;
	}

	this->mValue /= other;

	return *this;
}

template <typename T, size_t D, size_t P>
inline typename AutoDiff_Vector<T, D, P>::InvReturnType
AutoDiff_Vector<T, D, P>::inv() const
{
	return InvReturnType(*this, 1);
}

template <typename T, size_t D, size_t P>
inline typename AutoDiff_Vector<T, D, P>::AbsReturnType
AutoDiff_Vector<T, D, P>::abs() const
{
	return AbsReturnType(*this);
}

template <typename T, size_t D, size_t P>
template <typename U>
inline Lazy::enable_if_adlo_vector1_t<U, typename AutoDiff_Vector<T, D, P>::DotReturnType>
AutoDiff_Vector<T, D, P>::dot(const U& other) const
{
	const auto v = other.v();
	DotReturnType d;
	d.mValue = this->mValue.dot(v);
	for (size_t i = 0; i < P; ++i) {
		d.mGradients[i] = this->mGradients[i].dot(v) + this->mValue.dot(other->d(i));
	}
	return d;
}

template <typename T, size_t D, size_t P>
template <typename U>
inline Lazy::enable_if_adlo_vector1_t<U, typename AutoDiff_Vector<T, D, P>::CrossReturnType>
AutoDiff_Vector<T, D, P>::cross(const U& other) const
{
	const auto v = other.v();
	CrossReturnType d;
	d.mValue = this->mValue.cross(v);
	for (size_t i = 0; i < P; ++i) {
		d.mGradients[i] = this->mGradients[i].cross(v) + this->mValue.cross(other->d(i));
	}
	return d;
}

template <typename T, size_t D, size_t P>
inline typename AutoDiff_Vector<T, D, P>::scalar_v_t
AutoDiff_Vector<T, D, P>::norm() const
{
	return this->dot(*this);
}

template <typename T, size_t D, size_t P>
inline void AutoDiff_Vector<T, D, P>::normalize()
{
	*this /= norm();
}

template <typename T, size_t D, size_t P>
inline typename AutoDiff_Vector<T, D, P>::self_t
AutoDiff_Vector<T, D, P>::normalized() const
{
	self_t v = *this;
	v.normalize();
	return v;
}
}
