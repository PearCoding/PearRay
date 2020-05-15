#pragma once

#include "ISerializable.h"

#include <unordered_map>
#include <vector>

namespace PR {
/* Speed up for trivial types.
 * Some container have boolean specializations (std::vector<bool>).
 * We can not use direct access there. */
template <typename T>
using is_trivial_serializable = std::integral_constant<
	bool,
	std::is_trivial<T>::value
		&& !std::is_same<T, bool>::value>;

/* Major reason for own serialization class is the 'non' use of templates in the members. */
class SerializerInternal;
class PR_LIB_CORE Serializer {
	PR_CLASS_NON_COPYABLE(Serializer);

public:
	explicit Serializer(bool readmode);
	virtual ~Serializer();

	inline bool isReadMode() const;

	// Write
	inline void write(bool v);
	inline void write(int8 v);
	inline void write(uint8 v);
	inline void write(int16 v);
	inline void write(uint16 v);
	inline void write(int32 v);
	inline void write(uint32 v);
	inline void write(int64 v);
	inline void write(uint64 v);
	inline void write(float v);
	inline void write(double v);
	inline void write(const std::string& v);
	inline void write(const std::wstring& v);
	inline void write(const ISerializable& v);
	template <typename T, typename Alloc>
	inline std::enable_if_t<is_trivial_serializable<T>::value, void>
	write(const std::vector<T, Alloc>& vec);
	template <typename T, typename Alloc>
	inline std::enable_if_t<!is_trivial_serializable<T>::value, void>
	write(const std::vector<T, Alloc>& vec);
	template <typename T1, typename T2>
	inline void write(const std::unordered_map<T1, T2>& map);
	template <typename Scalar, int Rows, int Cols, int Options>
	inline void write(const Eigen::Matrix<Scalar, Rows, Cols, Options>& v);

	// Read
	inline void read(bool& v);
	inline void read(int8& v);
	inline void read(uint8& v);
	inline void read(int16& v);
	inline void read(uint16& v);
	inline void read(int32& v);
	inline void read(uint32& v);
	inline void read(int64& v);
	inline void read(uint64& v);
	inline void read(float& v);
	inline void read(double& v);
	inline void read(std::string& v);
	inline void read(std::wstring& v);
	inline void read(ISerializable& v);
	template <typename T, typename Alloc>
	inline std::enable_if_t<is_trivial_serializable<T>::value, void>
	read(std::vector<T, Alloc>& vec);
	template <typename T, typename Alloc>
	inline std::enable_if_t<!is_trivial_serializable<T>::value, void>
	read(std::vector<T, Alloc>& vec);
	template <typename T1, typename T2>
	inline void read(std::unordered_map<T1, T2>& map);
	template <typename Scalar, int Rows, int Cols, int Options>
	inline void read(Eigen::Matrix<Scalar, Rows, Cols, Options>& v);

	// Interface
	virtual bool isValid() const											= 0;
	virtual void writeRaw(const uint8* data, size_t elems, size_t elemSize) = 0;
	virtual void readRaw(uint8* data, size_t elems, size_t elemSize)		= 0;

protected:
	inline void setReadMode(bool b) { mReadMode = b; };

private:
	bool mReadMode;
};

inline Serializer& operator|(Serializer& ser, bool& v);
inline Serializer& operator|(Serializer& ser, int8& v);
inline Serializer& operator|(Serializer& ser, uint8& v);
inline Serializer& operator|(Serializer& ser, int16& v);
inline Serializer& operator|(Serializer& ser, uint16& v);
inline Serializer& operator|(Serializer& ser, int32& v);
inline Serializer& operator|(Serializer& ser, uint32& v);
inline Serializer& operator|(Serializer& ser, int64& v);
inline Serializer& operator|(Serializer& ser, uint64& v);
inline Serializer& operator|(Serializer& ser, float& v);
inline Serializer& operator|(Serializer& ser, double& v);
inline Serializer& operator|(Serializer& ser, std::string& v);
inline Serializer& operator|(Serializer& ser, ISerializable& v);
template <typename T, typename Alloc>
inline Serializer& operator|(Serializer& ser, std::vector<T, Alloc>& v);
template <typename T1, typename T2>
inline Serializer& operator|(Serializer& ser, std::unordered_map<T1, T2>& v);
template <typename Scalar, int Rows, int Cols, int Options>
inline Serializer& operator|(Serializer& ser, Eigen::Matrix<Scalar, Rows, Cols, Options>& v);
} // namespace PR

#include "Serializer.inl"