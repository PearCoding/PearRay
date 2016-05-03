#pragma once

#include "spectral/Spectrum.h"

namespace PR
{
	namespace Photon
	{
		struct Photon
		{
			float Position[3]; // Non compressed positions
			uint8 Phi, Theta; // Support of 65536 possible directions is enough
			uint8 KDFlags;// Flags for the KD-Tree
			float Power[Spectrum::SAMPLING_COUNT];
		};// Approx: 370 bytes
	}
}