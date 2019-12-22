#include "Test.h"
#include "filter/FilterFactory.h"
#include "filter/IFilter.h"

using namespace PR;

constexpr int RAD_COUNT			   = 2;
constexpr size_t RADIUS[RAD_COUNT] = { 3, 0 };
constexpr size_t WIDTH[RAD_COUNT]  = { RADIUS[0] * 2 + 1, RADIUS[1] * 2 + 1 };

float sumFilter(const std::shared_ptr<IFilter>& filter)
{
	float sum;
	for (int i = -(int)filter->radius(); i <= (int)filter->radius(); ++i)
		for (int j = -(int)filter->radius(); j <= (int)filter->radius(); ++j)
			sum += filter->evalWeight(i, j);
	return sum;
}

PR_BEGIN_TESTCASE(Filter)
PR_TEST("Block")
{
	for (int i = 0; i < RAD_COUNT; ++i) {
		auto filter = FilterFactory::createFilter(FT_Block, RADIUS[i]);
		PR_CHECK_EQ(filter->radius(), RADIUS[i]);
		PR_CHECK_EQ(filter->width(), WIDTH[i]);
		PR_CHECK_EQ(filter->width(), filter->height());
		float sum = sumFilter(filter);
		PR_CHECK_NEARLY_EQ(sum, 1.0f);
	}
}
PR_TEST("Triangle")
{
	for (int i = 0; i < RAD_COUNT; ++i) {
		auto filter = FilterFactory::createFilter(FT_Triangle, RADIUS[i]);
		PR_CHECK_EQ(filter->radius(), RADIUS[i]);
		PR_CHECK_EQ(filter->width(), WIDTH[i]);
		PR_CHECK_EQ(filter->width(), filter->height());
		float sum = sumFilter(filter);
		PR_CHECK_NEARLY_EQ(sum, 1.0f);
	}
}
PR_TEST("Gaussian")
{
	for (int i = 0; i < RAD_COUNT; ++i) {
		auto filter = FilterFactory::createFilter(FT_Gaussian, RADIUS[i]);
		PR_CHECK_EQ(filter->radius(), RADIUS[i]);
		PR_CHECK_EQ(filter->width(), WIDTH[i]);
		PR_CHECK_EQ(filter->width(), filter->height());
		float sum = sumFilter(filter);
		PR_CHECK_NEARLY_EQ(sum, 1.0f);
	}
}
PR_TEST("Lanczos")
{
	for (int i = 0; i < RAD_COUNT; ++i) {
		auto filter = FilterFactory::createFilter(FT_Lanczos, RADIUS[i]);
		PR_CHECK_EQ(filter->radius(), RADIUS[i]);
		PR_CHECK_EQ(filter->width(), WIDTH[i]);
		PR_CHECK_EQ(filter->width(), filter->height());
		float sum = sumFilter(filter);
		PR_CHECK_NEARLY_EQ(sum, 1.0f);
	}
}
PR_TEST("Mitchell")
{
	for (int i = 0; i < RAD_COUNT; ++i) {
		auto filter = FilterFactory::createFilter(FT_Mitchell, RADIUS[i]);
		PR_CHECK_EQ(filter->radius(), RADIUS[i]);
		PR_CHECK_EQ(filter->width(), WIDTH[i]);
		PR_CHECK_EQ(filter->width(), filter->height());
		float sum = sumFilter(filter);
		PR_CHECK_NEARLY_EQ(sum, 1.0f);
	}
}
PR_END_TESTCASE()

// MAIN
PRT_BEGIN_MAIN
PRT_TESTCASE(Filter);
PRT_END_MAIN