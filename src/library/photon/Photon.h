#pragma once

#include "spectral/Spectrum.h"
#include "PearMath.h"

#define PR_USE_PHOTON_RGB

namespace PR
{
	namespace Photon
	{
		struct Photon
		{
			float Position[3]; // Non compressed positions
			uint8 Phi, Theta; // Support of 65536 possible directions is enough
			uint8 KDFlags;// Flags for the KD-Tree
#ifdef PR_USE_PHOTON_RGB
			float Power[3];
			uint8 _Padding;// Approx: 20 bytes?
#else
			float Power[Spectrum::SAMPLING_COUNT];// Approx: 370 bytes
#endif
		};
	}
}