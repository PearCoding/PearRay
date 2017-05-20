#pragma once

#include "spectral/Spectrum.h"
#include <Eigen/Dense>

/*
 0 - Photon hitpoints are spectral
 1 - Photon hitpoints are rgb
 */
#ifndef PR_PHOTON_RGB_MODE
# define PR_PHOTON_RGB_MODE 1
#endif

namespace PR
{
	namespace Photon
	{
		struct Photon
		{
			EIGEN_MAKE_ALIGNED_OPERATOR_NEW

			Eigen::Vector3f Position;
			uint8 Phi, Theta; // Support of 65536 possible directions is enough
			//uint8 KDFlags;// Flags for the KD-Tree
#if PR_PHOTON_RGB_MODE >= 1
			float Power[3];
#else
			Spectrum Power;// Approx: 370 bytes
#endif
		};
	}
}
