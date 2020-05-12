#pragma once

namespace PR {
inline bool Serializer::isReadMode() const { return mReadMode; }
inline uint32 Serializer::version() const { return mVersion; }

inline void Serializer::write(bool v)
{
	uint8 tmp = v ? 1 : 0;
	write(tmp);
}

inline void Serializer::write(int8 v) { write((uint8)v); }
inline void Serializer::write(uint8 v)
{
	writeRaw(&v, 1, sizeof(uint8));
}

inline void Serializer::write(int16 v) { write((uint16)v); }
inline void Serializer::write(uint16 v)
{
	writeRaw(reinterpret_cast<uint8*>(&v), 1, sizeof(uint16));
}

inline void Serializer::write(int32 v) { write((uint32)v); }
inline void Serializer::write(uint32 v)
{
	writeRaw(reinterpret_cast<uint8*>(&v), 1, sizeof(uint32));
}

inline void Serializer::write(int64 v) { write((uint64)v); }
inline void Serializer::write(uint64 v)
{
	writeRaw(reinterpret_cast<uint8*>(&v), 1, sizeof(uint64));
}

inline void Serializer::write(float v)
{
	writeRaw(reinterpret_cast<uint8*>(&v), 1, sizeof(float));
}

inline void Serializer::write(double v)
{
	writeRaw(reinterpret_cast<uint8*>(&v), 1, sizeof(double));
}

inline void Serializer::write(const std::string& v)
{
	const char* data = v.c_str();
	for (size_t i = 0; data[i]; ++i) {
		write((uint8)data[i]);
	}
	write((uint8)0);
}

inline void Serializer::write(const std::wstring& v)
{
	const wchar_t* data = v.c_str();
	for (size_t i = 0; data[i]; ++i) {
		write((wchar_t)data[i]);
	}
	write((wchar_t)0);
}

template <typename T, typename Alloc>
inline std::enable_if_t<is_trivial_serializable<T>::value, void>
Serializer::write(const std::vector<T, Alloc>& vec)
{
	write((uint64)vec.size());
	writeRaw(reinterpret_cast<const uint8*>(vec.data()),
			 vec.size(), sizeof(T));
}

template <typename T, typename Alloc>
inline std::enable_if_t<!is_trivial_serializable<T>::value, void>
Serializer::write(const std::vector<T, Alloc>& vec)
{
	write((uint64)vec.size());
	for (size_t i = 0; i < vec.size(); ++i) {
		write(vec.at(i));
	}
}

template <typename T1, typename T2>
inline void Serializer::write(const std::unordered_map<T1, T2>& map)
{
	write((uint64)map.size());
	for (auto it = map.begin(); it != map.end(); ++it) {
		write(it->first);
		write(it->second);
	}
}

template <typename Scalar, int Rows, int Cols, int Options>
inline void Serializer::write(const Eigen::Matrix<Scalar, Rows, Cols, Options>& v)
{
	for (int r = 0; r < Rows; ++r)
		for (int c = 0; c < Cols; ++c)
			write(v(r, c));
}

inline void Serializer::write(const ISerializable& v)
{
	const_cast<ISerializable&>(v).serialize(*this);
}

//////////////////////////////////////////////////////////

inline void Serializer::read(bool& v)
{
	uint8 tmp;
	read(tmp);
	v = (tmp != 0);
}

inline void Serializer::read(int8& v)
{
	uint8 tmp;
	read(tmp);
	v = static_cast<int8>(tmp);
}

inline void Serializer::read(uint8& v)
{
	readRaw(&v, 1, sizeof(uint8));
}

inline void Serializer::read(int16& v)
{
	uint16 tmp;
	read(tmp);
	v = static_cast<int8>(tmp);
}

inline void Serializer::read(uint16& v)
{
	readRaw(reinterpret_cast<uint8*>(&v), 1, sizeof(uint16));
}

inline void Serializer::read(int32& v)
{
	uint32 tmp;
	read(tmp);
	v = static_cast<int8>(tmp);
}

inline void Serializer::read(uint32& v)
{
	readRaw(reinterpret_cast<uint8*>(&v), 1, sizeof(uint32));
}

inline void Serializer::read(int64& v)
{
	uint64 tmp;
	read(tmp);
	v = static_cast<int8>(tmp);
}

inline void Serializer::read(uint64& v)
{
	readRaw(reinterpret_cast<uint8*>(&v), 1, sizeof(uint64));
}

inline void Serializer::read(float& v)
{
	readRaw(reinterpret_cast<uint8*>(&v), 1, sizeof(float));
}

inline void Serializer::read(double& v)
{
	readRaw(reinterpret_cast<uint8*>(&v), 1, sizeof(double));
}

inline void Serializer::read(std::string& v)
{
	v.clear();

	uint8 c;
	read(c);
	for (; c != 0; read(c)) {
		v += static_cast<uint8>(c);
	}
}

inline void Serializer::read(std::wstring& v)
{
	v.clear();

	uint32 c;
	read(c);
	for (; c != 0; read(c)) {
		v += static_cast<wchar_t>(c);
	}
}

inline void Serializer::read(ISerializable& v)
{
	v.serialize(*this);
}

template <typename T, typename Alloc>
inline std::enable_if_t<is_trivial_serializable<T>::value, void>
Serializer::read(std::vector<T, Alloc>& vec)
{
	uint64 size;
	read(size);
	vec.resize(size);
	readRaw(reinterpret_cast<uint8*>(vec.data()), vec.size(), sizeof(T));
}

template <typename T, typename Alloc>
inline std::enable_if_t<!is_trivial_serializable<T>::value, void>
Serializer::read(std::vector<T, Alloc>& vec)
{
	uint64 size;
	read(size);
	vec.resize(size);

	T tmp;
	for (size_t i = 0; i < size; ++i) {
		read(tmp);
		vec[i] = tmp;
	}
}

template <typename T1, typename T2>
inline void Serializer::read(std::unordered_map<T1, T2>& map)
{
	map.clear();

	uint64 size;
	read(size);

	T1 t1;
	T2 t2;
	for (size_t i = 0; i < size; ++i) {
		read(t1);
		read(t2);
		map[t1] = t2;
	}

	PR_ASSERT(map.size() == size, "Given size is not same as produced one!");
}

template <typename Scalar, int Rows, int Cols, int Options>
inline void Serializer::read(Eigen::Matrix<Scalar, Rows, Cols, Options>& v)
{
	Scalar tmp[Rows * Cols];
	for (int r = 0; r < Rows; ++r)
		for (int c = 0; c < Cols; ++c)
			read(tmp[r * Cols + c]);

	for (int r = 0; r < Rows; ++r)
		for (int c = 0; c < Cols; ++c)
			v(r, c) = tmp[r * Cols + c];
}

////////////////////////////////////////////////////

#define PR_SER_OPERATOR(type)                              \
	inline Serializer& operator|(Serializer& ser, type& v) \
	{                                                      \
		if (ser.isReadMode())                              \
			ser.read(v);                                   \
		else                                               \
			ser.write(v);                                  \
		return ser;                                        \
	}

#define PR_SER_OPERATOR_T2(type, t1, t2)                           \
	inline Serializer& operator|(Serializer& ser, type<t1, t2>& v) \
	{                                                              \
		if (ser.isReadMode())                                      \
			ser.read(v);                                           \
		else                                                       \
			ser.write(v);                                          \
		return ser;                                                \
	}

#define PR_SER_OPERATOR_T3(type, t1, t2, t3)                           \
	inline Serializer& operator|(Serializer& ser, type<t1, t2, t3>& v) \
	{                                                                  \
		if (ser.isReadMode())                                          \
			ser.read(v);                                               \
		else                                                           \
			ser.write(v);                                              \
		return ser;                                                    \
	}

#define PR_SER_OPERATOR_T4(type, t1, t2, t3, t4)                           \
	inline Serializer& operator|(Serializer& ser, type<t1, t2, t3, t4>& v) \
	{                                                                      \
		if (ser.isReadMode())                                              \
			ser.read(v);                                                   \
		else                                                               \
			ser.write(v);                                                  \
		return ser;                                                        \
	}

PR_SER_OPERATOR(bool)
PR_SER_OPERATOR(int8)
PR_SER_OPERATOR(uint8)
PR_SER_OPERATOR(int16)
PR_SER_OPERATOR(uint16)
PR_SER_OPERATOR(int32)
PR_SER_OPERATOR(uint32)
PR_SER_OPERATOR(int64)
PR_SER_OPERATOR(uint64)
PR_SER_OPERATOR(float)
PR_SER_OPERATOR(double)
PR_SER_OPERATOR(std::string)
PR_SER_OPERATOR(ISerializable)

template <typename T, typename Alloc>
PR_SER_OPERATOR_T2(std::vector, T, Alloc)
template <typename T1, typename T2>
PR_SER_OPERATOR_T2(std::unordered_map, T1, T2)

template <typename Scalar, int Rows, int Cols, int Options>
PR_SER_OPERATOR_T4(Eigen::Matrix, Scalar, Rows, Cols, Options)

#undef PR_SER_OPERATOR

} // namespace PR