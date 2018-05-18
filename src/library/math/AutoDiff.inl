namespace PR {
template <typename T, size_t P>
AutoDiff_Base<T, P>::AutoDiff_Base(const value_t v, const array_t& l)
	: mValue(v)
	, mGradients(l)
{
}

template <typename T, size_t P>
template <typename... Args>
AutoDiff_Base<T, P>::AutoDiff_Base(const value_t v, const Args&... gs)
	: mValue(v)
	, mGradients{ (T)gs... }
{
}

template <typename T, size_t P>
template <typename ADLO>
AutoDiff_Base<T, P>::AutoDiff_Base(const ADLO& adlo, std::enable_if_t<Lazy::is_adlo<ADLO>::value>*)
	: mValue(adlo.v())
	, mGradients()
{
	for (size_t i = 0; i < P; ++i) {
		mGradients[i] = adlo.d(i);
	}
}

template <typename T, size_t P>
template <typename ADLO>
std::enable_if_t<Lazy::is_adlo<ADLO>::value, typename AutoDiff_Base<T, P>::AutoDiff_Base&>
AutoDiff_Base<T, P>::operator=(const ADLO& adlo)
{
	mValue = adlo.v();
	for (size_t i = 0; i < P; ++i) {
		mGradients[i] = adlo.d(i);
	}
	return *this;
}

// Accessor
template <typename T, size_t P>
inline const typename AutoDiff_Base<T, P>::value_t& AutoDiff_Base<T, P>::v() const
{
	return mValue;
}

template <typename T, size_t P>
inline typename AutoDiff_Base<T, P>::value_t& AutoDiff_Base<T, P>::v()
{
	return mValue;
}

template <typename T, size_t P>
inline const typename AutoDiff_Base<T, P>::value_t& AutoDiff_Base<T, P>::d(size_t p) const
{
	return mGradients[p];
}

template <typename T, size_t P>
inline typename AutoDiff_Base<T, P>::value_t& AutoDiff_Base<T, P>::d(size_t p)
{
	return mGradients[p];
}
}