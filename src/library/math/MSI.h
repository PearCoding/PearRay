#pragma once

#include "PearMath.h"
#include "spectral/Spectrum.h"

namespace PR
{
	class MSI
	{
	public:
		static inline float power(float pdf1, float pdf2)
		{
			PR_ASSERT(pdf1 >= 0);
			PR_ASSERT(pdf2 >= 0);

			// TODO:
			return 0;
		}

		// Sums both weights together based on the pdf.
		static inline void balance(Spectrum& out_weight, float& out_pdf, Spectrum in_weight, float in_pdf, float balance = 1)
		{
			PR_ASSERT(out_pdf >= 0);
			PR_ASSERT(in_pdf >= 0);
			PR_ASSERT(balance > PM_EPSILON);
			PR_ASSERT(balance <= 1);

			in_pdf *= balance;
			in_weight *= 1 / balance;

			float w;
			if (out_pdf < in_pdf)
			{
				w = 1 / (1 + out_pdf / in_pdf);
			}
			else if (in_pdf < out_pdf)
			{
				w = 1 - 1 / (1 + in_pdf / out_pdf);
			}
			else
			{
				w = 0.5f;
			}

			out_weight = out_weight * (1 - w) + in_weight * w;
			out_pdf += in_pdf;

			PR_ASSERT(out_pdf >= 0);
		}
	};
}