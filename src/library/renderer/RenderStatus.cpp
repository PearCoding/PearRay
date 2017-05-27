#include "RenderStatus.h"

namespace PR {
RenderStatus::RenderStatus()
	: mPercentage(0)
{
}

void RenderStatus::setPercentage(float f)
{
	mPercentage = f;
}

float RenderStatus::percentage() const
{
	return mPercentage;
}

void RenderStatus::setField(const std::string& unique_name, const Variant& v)
{
	mFields[unique_name] = v;
}

bool RenderStatus::hasField(const std::string& unique_name) const
{
	return mFields.count(unique_name) != 0;
}

const Variant& RenderStatus::getField(const std::string& unique_name) const
{
	return mFields.at(unique_name);
}

RenderStatus::map_t::const_iterator RenderStatus::begin() const
{
	return mFields.begin();
}

RenderStatus::map_t::const_iterator RenderStatus::end() const
{
	return mFields.end();
}
}
