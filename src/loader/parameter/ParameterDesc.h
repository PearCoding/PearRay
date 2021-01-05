#pragma once

#include "Enum.h"
#include "Parameter.h"

namespace PR {
enum class ParameterFlag {
	Optional  = 0x1,
	NoDefault = 0x2
};
PR_MAKE_FLAGS(ParameterFlag, ParameterFlags)

template <typename T>
struct ParameterDescBase {
	std::vector<std::string> Names; // In a normalized parameter group (as passed for example to a plugin create() call) the first entry will be used!
	std::string Description;
	T Default;
	ParameterFlags Flags;
};

using ParameterBoolDesc = ParameterDescBase<bool>;

template <typename T>
struct ParameterMinMaxDescBase : public ParameterDescBase<T> {
	T SoftMin;
	T SoftMax;
	T Min = std::numeric_limits<T>::min();
	T Max = std::numeric_limits<T>::max();
};

using ParameterIntDesc	  = ParameterMinMaxDescBase<Parameter::Int>;
using ParameterUIntDesc	  = ParameterMinMaxDescBase<Parameter::UInt>;
using ParameterNumberDesc = ParameterMinMaxDescBase<Parameter::Number>;

struct ParameterVectorDesc : public ParameterDescBase<Vector3f> {
};

struct ParameterStringDesc : public ParameterDescBase<std::string> {
};

struct ParameterOptionDesc : public ParameterDescBase<std::string> {
	std::vector<std::string> Options;
};

struct ParameterFilenameDesc : public ParameterDescBase<std::string> {
};

struct ParameterScalarNodeDesc : public ParameterDescBase<float> {
};

struct ParameterSpectralNodeDesc : public ParameterDescBase<float> {
};

struct ParameterVectorNodeDesc : public ParameterDescBase<Vector3f> {
};

enum class ParameterReferenceType {
	Mesh,
	Material,
	Emission,
	Displacement,
	Unknown
};

struct ParameterReferenceDesc : public ParameterDescBase<uint32> {
	ParameterReferenceType ReferenceType;
};

class ParameterDesc {
public:
	using Variant = std::variant<ParameterBoolDesc, ParameterIntDesc, ParameterUIntDesc, ParameterNumberDesc,
								 ParameterVectorDesc, ParameterStringDesc, ParameterOptionDesc, ParameterFilenameDesc,
								 ParameterScalarNodeDesc, ParameterSpectralNodeDesc, ParameterVectorNodeDesc,
								 ParameterReferenceDesc>;

	Variant Value;

	inline /*implicit*/ ParameterDesc(const Variant& value)
		: Value(value)
	{
	}

	inline constexpr bool isBool() const { return std::holds_alternative<ParameterBoolDesc>(Value); }
	inline constexpr bool isInt() const { return std::holds_alternative<ParameterIntDesc>(Value); }
	inline constexpr bool isUInt() const { return std::holds_alternative<ParameterUIntDesc>(Value); }
	inline constexpr bool isNumber() const { return std::holds_alternative<ParameterNumberDesc>(Value); }
	inline constexpr bool canBeNumber() const { return isInt() || isUInt() || isNumber(); }

	inline constexpr bool isVector() const { return std::holds_alternative<ParameterVectorDesc>(Value); }

	inline constexpr bool isString() const { return std::holds_alternative<ParameterStringDesc>(Value); }
	inline constexpr bool isOptions() const { return std::holds_alternative<ParameterOptionDesc>(Value); }
	inline constexpr bool isFilename() const { return std::holds_alternative<ParameterFilenameDesc>(Value); }
	inline constexpr bool canBeString() const { return isString() || isOptions() || isFilename(); }

	inline constexpr bool isScalarNode() const { return std::holds_alternative<ParameterScalarNodeDesc>(Value); }
	inline constexpr bool isSpectralNode() const { return std::holds_alternative<ParameterSpectralNodeDesc>(Value); }
	inline constexpr bool isVectorNode() const { return std::holds_alternative<ParameterVectorNodeDesc>(Value); }
	inline constexpr bool isNode() const { return isScalarNode() || isSpectralNode() || isVectorNode(); }
	inline constexpr bool isReference() const { return std::holds_alternative<ParameterReferenceDesc>(Value); }

	inline constexpr ParameterFlags flags() const
	{
		return std::visit([](const auto& v) { return v.Flags; }, Value);
	}

	inline bool hasAlternativeNames() const
	{
		return std::visit([](const auto& v) { return v.Names.size() > 1; }, Value);
	}

	inline std::string normalizedName() const
	{
		return std::visit([](const auto& v) { return v.Names.front(); }, Value);
	}

	inline std::string description() const
	{
		return std::visit([](const auto& v) { return v.Description; }, Value);
	}

	template <typename T>
	inline constexpr T get() const { return std::get<T>(Value); }
};

} // namespace PR
