#include "RenderStatus.h"

namespace PR {
RenderStatus::RenderStatus()
	: mPercentage(-1)
{
}

void RenderStatus::setPercentage(double f)
{
	mPercentage = f;
}

double RenderStatus::percentage() const
{
	return mPercentage;
}

void RenderStatus::setField(const std::string& unique_name, const Field& v)
{
	mFields[unique_name] = v;
}

bool RenderStatus::hasField(const std::string& unique_name) const
{
	return mFields.count(unique_name) != 0;
}

RenderStatus::Field RenderStatus::getField(const std::string& unique_name) const
{
	PR_ASSERT(hasField(unique_name), "Couldn't return status field due to missing entry.");
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
