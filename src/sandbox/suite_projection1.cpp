#include "Logger.h"

#include "math/Projection.h"

#include <boost/filesystem.hpp>
#include <fstream>

using namespace PR;
namespace bf = boost::filesystem;

void suite_projection1()
{
    bf::create_directory("results/projection1");

    std::ofstream outfile("results/projection1/cos_hemi.cvs");

    constexpr float step=0.01f;

    constexpr int N = 1/step + 1;
    for(int i = 0; i < N; ++i)
    {
        for(int j = 0; j < N; ++j)
        {
            const float u1 = i*step;
            const float u2 = j*step;

            outfile << u1 << " " << u2;
            for(int k = 0; k < 10; ++k)
            {
                float pdf;
                Projection::cos_hemi(u1,u2,k,pdf);
                outfile << " " << pdf;
            }
            outfile << std::endl;
        }
    }
}