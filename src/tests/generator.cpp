#include "math/Generator.h"

#include <set>
#include <tuple>

#include "Test.h"

using namespace PR;

constexpr int Radius = 2;
PR_BEGIN_TESTCASE(Generator)
PR_TEST("spiral 2D")
{
	MinRadiusGenerator<2> generator(Radius);
	std::set<typename MinRadiusGenerator<2>::point_type > uniqueness;
	std::set<typename MinRadiusGenerator<2>::point_type > correctness;
	for(int x = -Radius; x <= Radius; ++x)
	{
		for(int y = -Radius; y <= Radius; ++y)
		{
			correctness.insert(MinRadiusGenerator<2>::point_type{x,y});
		}
	}

	while(generator)
	{
		const auto pair = generator.next();
		std::cout << pair[0] << " " << pair[1] << std::endl;
		PR_CHECK_EQ(uniqueness.count(pair), 0);
		PR_CHECK_EQ(correctness.count(pair), 1);
		uniqueness.insert(pair);
	}
}
PR_TEST("spiral 3D")
{
	MinRadiusGenerator<3> generator(Radius);
	std::set<typename MinRadiusGenerator<3>::point_type > uniqueness;
	std::set<typename MinRadiusGenerator<3>::point_type > correctness;
	for(int x = -Radius; x <= Radius; ++x)
	{
		for(int y = -Radius; y <= Radius; ++y)
		{
			for(int z = -Radius; z <= Radius; ++z)
			{
				correctness.insert(MinRadiusGenerator<3>::point_type{x,y,z});
			}
		}
	}
	while(generator)
	{
		const auto pair = generator.next();
		std::cout << pair[0] << " " << pair[1] << " " << pair[2] << std::endl;
		PR_CHECK_EQ(uniqueness.count(pair), 0);
		PR_CHECK_EQ(correctness.count(pair), 1);
		uniqueness.insert(pair);
	}
}
PR_END_TESTCASE()

// MAIN
PRT_BEGIN_MAIN
PRT_TESTCASE(Generator);
PRT_END_MAIN