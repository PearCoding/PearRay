#include "XYZConverter.h"
//#include "Logger.h"

#define PR_XYZ_LINEAR_INTERP

namespace PR
{
# include "xyz.inl"

	void XYZConverter::convertXYZ(const float* src, float &X, float &Y, float &Z)
	{
		X = 0;
		Y = 0;
		Z = 0;

# ifdef PR_XYZ_LINEAR_INTERP
		for (uint32 i = 0; i < Spectrum::SAMPLING_COUNT-1; ++i)
		{
			const float val1 = src[i];
			const float val2 = src[i + 1];

			X += val1 * NM_TO_X[i] + val2 * NM_TO_X[i + 1];
			Y += val1 * NM_TO_Y[i] + val2 * NM_TO_Y[i + 1];
			Z += val1 * NM_TO_Z[i] + val2 * NM_TO_Z[i + 1];
		}
#else
		for (uint32 i = 0; i < Spectrum::SAMPLING_COUNT-1; ++i)
		{
			const float val1 = src[i];
			X += val1 * NM_TO_X[i];
			Y += val1 * NM_TO_Y[i];
			Z += val1 * NM_TO_Z[i];
		}
# endif

		X /= Y_SUM;
		Y /= Y_SUM;
		Z /= Y_SUM;

# ifdef PR_XYZ_LINEAR_INTERP
		X *= 0.5f;
		Y *= 0.5f;
		Z *= 0.5f;
# endif
	}

	void XYZConverter::convert(const float* src, float &x, float &y, float &z)
	{
		float X, Y, Z;
		convertXYZ(src, X, Y, Z);

		float m = X + Y + Z;
		if (m != 0)
		{
			x = X / m;
			y = Y / m;
			z = 1 - x - y;
		}
		else
		{
			x = 0; y = 0; z = 0;
		}
	}

	/* The following algorithm is based on:
	 *
	 *   Johannes Meng, Florian Simon, Johannes Hanika, and Carsten Dachsbacher. 2015.
	 *   Physically meaningful rendering using tristimulus colours.
	 *   In Proceedings of the 26th Eurographics Symposium on Rendering (EGSR '15).
	 *   Eurographics Association, Aire-la-Ville, Switzerland, Switzerland, 31-40.
	 *   DOI=http://dx.doi.org/10.1111/cgf.12676
	 *
	 * with some own extensions to it.
	 * The needed data is generated by tools extern.
	 */
	namespace _xyz2spec
	{
#include "xyz2spec.inl"
	}

	// We have the right ordering
	void barycentricTriangle(double px, double py,
		double x1, double y1, double x2, double y2, double x3, double y3, double invDet,
		double& s, double& t)
	{
		s = (py*x1 - py*x3 - px*y1 + px*y3 - x1*y3 + x3*y1)*invDet;
 		t = -(py*x1 - py*x2 - px*y1 + px*y2 - x1*y2 + x2*y1)*invDet;
	}

	Spectrum XYZConverter::toSpec(float x, float y, float z)
	{
		const float b = x+y+z;// Brightness
		const float nx = x/b;
		const float ny = y/b;

		/*PR_LOGGER.logf(L_Warning, M_Internal, "XYZ [%f,%f,%f] -> [%f,%f]", x,y,z,nx,ny);*/
		const int vIndex = _xyz2spec::pointToVoxel(nx,ny);
		if(vIndex == -1)// Error?
			return Spectrum();
		
		/*PR_LOGGER.logf(L_Warning, M_Internal, "  Voxel %i -> [%i, %i]",
			vIndex, _xyz2spec::voxelTable[vIndex][0], _xyz2spec::voxelTable[vIndex][1]);*/

		// Check if point inside triangle
		double s = -1;
		double t = -1;
		uint32 triInd = 0;
		for(uint32 i = _xyz2spec::voxelTable[vIndex][0];
			i < _xyz2spec::voxelTable[vIndex][1] && (s < 0 || t < 0 || 1-s-t < 0);
			++i)
		{
			triInd = _xyz2spec::voxelEntryTable[i];
			if(_xyz2spec::triInvDetTable[triInd] == 0)
				continue;
			
			const auto tri = _xyz2spec::triTable[triInd];

			const double p0x = _xyz2spec::pointTable[tri[0]][0];
			const double p0y = _xyz2spec::pointTable[tri[0]][1];
			const double p1x = _xyz2spec::pointTable[tri[1]][0];
			const double p1y = _xyz2spec::pointTable[tri[1]][1];
			const double p2x = _xyz2spec::pointTable[tri[2]][0];
			const double p2y = _xyz2spec::pointTable[tri[2]][1];

			/*PR_LOGGER.logf(L_Warning, M_Internal, "  Tri [%f, %f]~[%f, %f]~[%f, %f]",
				p0x,p0y,p1x,p1y,p2x,p2y);*/

			barycentricTriangle(nx, ny,
				p0x, p0y, p1x, p1y, p2x, p2y,
				_xyz2spec::triInvDetTable[triInd],
				s, t);

			/*PR_LOGGER.logf(L_Warning, M_Internal, "  => [%f, %f]",
				s,t);*/
		}

		if(s < 0 || t < 0 || 1-s-t < 0)// No triangle found
			return Spectrum();

		const Spectrum& s1 = _xyz2spec::xyzTable[_xyz2spec::triTable[triInd][0]];
		const Spectrum& s2 = _xyz2spec::xyzTable[_xyz2spec::triTable[triInd][1]];
		const Spectrum& s3 = _xyz2spec::xyzTable[_xyz2spec::triTable[triInd][2]];

		const Spectrum output = s1*(1-s-t) + s2*s + s3*t;

		return output * b;
	}
}
