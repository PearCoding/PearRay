#pragma once

#include <string>
#include <utility>
#include <iostream>
#include <sstream>

template<class T>
class ContainerBase
{
};

template<class T>
class EnumOptionBase
{
public:
	typedef ContainerBase<T> container_base;
	typedef std::pair<const char*, T> pair_type;

	static inline std::string get_name(const T& t) 
	{
		size_t i = 0;
		while(container_base::container[i].first) { 
			if(container_base::container[i].second == t)
				return container_base::container[i].first;
			++i;
		}
		return "";
	}

	static inline T get_value(const std::string& name)
	{
		size_t i = 0;
		while(container_base::container[i].first) { 
			if(container_base::container[i].first == name)
				return container_base::container[i].second;
			++i;
		}
		return get_default();
	}

	static inline bool has(const std::string& name)
	{
		size_t i = 0;
		while(container_base::container[i].first) { 
			if(container_base::container[i].first == name)
				return true;
			++i;
		}
		return false;
	}

	static inline T get_default()
	{
		size_t i = 0;
		while(container_base::container[i].first)
			++i;
		return container_base::container[i].second;
	}

	static inline std::string get_names()
	{
		std::stringstream stream;
		stream << container_base::container[0].first;
		size_t i = 1;
		while(container_base::container[i].first)
		{
			stream << ", " << container_base::container[i].first;
			++i;
		}
		return stream.str();
	}
};

template<class T>
class EnumOption : public EnumOptionBase<T>
{
private:
	T mValue;

public:
	EnumOption(const T& t) :
		mValue(t)
	{}

	inline EnumOption<T>& operator = (const T& t)
	{
		mValue = t;
		return *this;
	}

	inline operator T() const
	{
		return mValue;
	}
};

template<class T>
std::ostream& operator << (std::ostream& stream, const EnumOption<T>& m) {
	stream << EnumOption<T>::get_name(m);
	return stream;
}

template<class T>
std::istream& operator >> (std::istream& stream, EnumOption<T>& m) {
	std::string name;
	if(!(stream >> name))
		return stream;
	m = EnumOption<T>::get_value(name);
	return stream;
}

/* ATTENTION! Should have atleast two elements! */
#define BEGIN_ENUM_OPTION(type) \
template<> \
class ContainerBase<type> { \
public: \
	static std::pair<const char*, type> container[]; \
}; \
std::pair<const char*, type> ContainerBase<type>::container[] = 

