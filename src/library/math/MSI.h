#pragma once

#include "PearMath.h"
#include "spectral/Spectrum.h"

namespace PR
{
	namespace MSI
	{
		inline float power(float& out_pdf, float in_pdf)
		{
			PR_ASSERT(out_pdf >= 0);
			PR_ASSERT(in_pdf >= 0);

			float w;
			if (out_pdf < in_pdf)
			{
				const float r = out_pdf / in_pdf;
				w = 1 / (1 + r * r);
			}
			else if (in_pdf < out_pdf)
			{
				const float r = in_pdf / out_pdf;
				w = 1 - 1 / (1 + r * r);
			}
			else
			{
				w = 0.5f;
			}

			out_pdf += in_pdf;

			return w;
		}

		inline void power(Spectrum& out_weight, float& out_pdf, const Spectrum& in_weight, float in_pdf)
		{
			float w = power(out_pdf, in_pdf);
			out_weight = out_weight * (1 - w) + in_weight * w;
		}

		inline float power(float& out_pdf, float in_pdf, float beta)
		{
			PR_ASSERT(out_pdf >= 0);
			PR_ASSERT(in_pdf >= 0);

			float w;
			if (out_pdf < in_pdf)
			{
				const float r = out_pdf / in_pdf;
				w = 1 / (1 + std::pow(r, beta));
			}
			else if (in_pdf < out_pdf)
			{
				const float r = in_pdf / out_pdf;
				w = 1 - 1 / (1 + std::pow(r, beta));
			}
			else
			{
				w = 0.5f;
			}

			out_pdf += in_pdf;

			return w;
		}

		inline void power(Spectrum& out_weight, float& out_pdf, const Spectrum& in_weight, float in_pdf, float beta)
		{
			float w = power(out_pdf, in_pdf, beta);
			out_weight = out_weight * (1 - w) + in_weight * w;
		}

		inline float balance(float& out_pdf, float in_pdf)
		{
			PR_ASSERT(out_pdf >= 0);
			PR_ASSERT(in_pdf >= 0);

			float w;
			if (out_pdf < in_pdf)
				w = 1 / (1 + out_pdf / in_pdf);
			else if (in_pdf < out_pdf)
				w = 1 - 1 / (1 + in_pdf / out_pdf);
			else
				w = 0.5f;

			out_pdf += in_pdf;

			return w;
		}
		
		inline void balance(Spectrum& out_weight, float& out_pdf, const Spectrum& in_weight, float in_pdf)
		{
			float w = balance(out_pdf, in_pdf);
			out_weight = out_weight * (1 - w) + in_weight * w;
		}

        // Casts
		inline float toSolidAngle(float pdf_area, float dist_sqr, float cosine)
		{
			return pdf_area * cosine / dist_sqr;
		}

		inline float toArea(float pdf_solidangle, float dist_sqr, float cosine)
		{
			return pdf_solidangle * dist_sqr / cosine;
		}
	}
}