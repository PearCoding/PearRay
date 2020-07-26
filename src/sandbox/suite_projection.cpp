#include "Logger.h"

#include "math/Sampling.h"

#include <filesystem>
#include <fstream>

using namespace PR;
namespace sf = std::filesystem;

void suite_projection()
{
    sf::create_directory("results/projection");

    std::ofstream outfile("results/projection/cos_hemi.cvs");

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
                Sampling::cos_hemi(u1,u2,k,pdf);
                outfile << " " << pdf;
            }
            outfile << std::endl;
        }
    }
}