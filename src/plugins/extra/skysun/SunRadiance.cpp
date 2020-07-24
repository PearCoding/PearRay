#include "SunRadiance.h"
#include "spectral/OrderedSpectrum.h"

/* The following is from the implementation of "A Practical Analytic Model for
   Daylight" by A.J. Preetham, Peter Shirley, and Brian Smits */

/* All data lifted from MI. Units are either [] or cm^-1. refer when in doubt MI */

// This is based on Mitsuba code (https://github.com/mitsuba-renderer/mitsuba/blob/master/src/emitters/sunsky/sunmodel.h)
namespace PR {
// k_o Spectrum table from pg 127, MI.
static float k_oWavelengths[64] = {
	300, 305, 310, 315, 320, 325, 330, 335, 340, 345,
	350, 355, 445, 450, 455, 460, 465, 470, 475, 480,
	485, 490, 495, 500, 505, 510, 515, 520, 525, 530,
	535, 540, 545, 550, 555, 560, 565, 570, 575, 580,
	585, 590, 595, 600, 605, 610, 620, 630, 640, 650,
	660, 670, 680, 690, 700, 710, 720, 730, 740, 750,
	760, 770, 780, 790
};

static float k_oAmplitudes[64] = {
	10.0, 4.8, 2.7, 1.35, .8, .380, .160, .075, .04, .019, .007,
	.0, .003, .003, .004, .006, .008, .009, .012, .014, .017,
	.021, .025, .03, .035, .04, .045, .048, .057, .063, .07,
	.075, .08, .085, .095, .103, .110, .12, .122, .12, .118,
	.115, .12, .125, .130, .12, .105, .09, .079, .067, .057,
	.048, .036, .028, .023, .018, .014, .011, .010, .009,
	.007, .004, .0
};

// k_g Spectrum table from pg 130, MI.
static float k_gWavelengths[4] = {
	759, 760, 770, 771
};

static float k_gAmplitudes[4] = {
	0, 3.0, 0.210, 0
};

// k_wa Spectrum table from pg 130, MI.
static float k_waWavelengths[13] = {
	689, 690, 700, 710, 720,
	730, 740, 750, 760, 770,
	780, 790, 800
};

static float k_waAmplitudes[13] = {
	0, 0.160e-1, 0.240e-1, 0.125e-1,
	0.100e+1, 0.870, 0.610e-1, 0.100e-2,
	0.100e-4, 0.100e-4, 0.600e-3,
	0.175e-1, 0.360e-1
};

/* Wavelengths corresponding to the table below */
static float solWavelengths[38] = {
	380, 390, 400, 410, 420, 430, 440, 450,
	460, 470, 480, 490, 500, 510, 520, 530,
	540, 550, 560, 570, 580, 590, 600, 610,
	620, 630, 640, 650, 660, 670, 680, 690,
	700, 710, 720, 730, 740, 750
};

/* Solar amplitude in watts / (m^2 * nm * sr) */
static float solAmplitudes[38] = {
	16559.0, 16233.7, 21127.5, 25888.2, 25829.1,
	24232.3, 26760.5, 29658.3, 30545.4, 30057.5,
	30663.7, 28830.4, 28712.1, 27825.0, 27100.6,
	27233.6, 26361.3, 25503.8, 25060.2, 25311.6,
	25355.9, 25134.2, 24631.5, 24173.2, 23685.3,
	23212.1, 22827.7, 22339.8, 21970.2, 21526.7,
	21097.9, 20728.3, 20240.4, 19870.8, 19427.2,
	19072.4, 18628.9, 18259.2
};

float computeSunRadiance(float wavelength, float theta, float turbidity)
{
	static OrderedSpectrumView k_oCurve(k_oAmplitudes, k_oWavelengths, 64);
	static OrderedSpectrumView k_gCurve(k_gAmplitudes, k_gWavelengths, 4);
	static OrderedSpectrumView k_waCurve(k_waAmplitudes, k_waWavelengths, 13);
	static OrderedSpectrumView solCurve(solAmplitudes, solWavelengths, 38);

	const float beta = 0.04608365822050f * turbidity - 0.04586025928522f;

	// Relative Optical Mass
	const float m = 1.0f / (std::cos(theta) + 0.15f * std::pow(93.885f - theta / PR_PI * 180.0f, -1.253f));

	// Rayleigh Scattering
	// Results agree with the graph (pg 115, MI) */
	float tauR = std::exp(-m * 0.008735f * std::pow(wavelength / 1000.0f, -4.08));

	// Aerosol (water + dust) attenuation
	// beta - amount of aerosols present
	// alpha - ratio of small to large particle sizes. (0:4,usually 1.3)
	// Results agree with the graph (pg 121, MI)
	constexpr float alpha = 1.3f;
	float tauA			  = std::exp(-m * beta * std::pow(wavelength / 1000.0f, -alpha)); // wavelength should be in um

	// Attenuation due to ozone absorption
	// lOzone - amount of ozone in cm(NTP)
	// Results agree with the graph (pg 128, MI)
	constexpr float lOzone = 0.35f;
	float tauO			   = std::exp(-m * k_oCurve.lookup(wavelength) * lOzone);

	// Attenuation due to mixed gases absorption
	// Results agree with the graph (pg 131, MI)
	float tauG = std::exp(-1.41f * k_gCurve.lookup(wavelength) * m / std::pow(1 + 118.93f * k_gCurve.lookup(wavelength) * m, 0.45f));

	// Attenuation due to water vapor absorbtion
	// w - precipitable water vapor in centimeters (standard = 2)
	// Results agree with the graph (pg 132, MI)
	constexpr float w = 2.0;
	float tauWA		  = std::exp(-0.2385f * k_waCurve.lookup(wavelength) * w * m / std::pow(1 + 20.07f * k_waCurve.lookup(wavelength) * w * m, 0.45f));

	return std::max(0.0f, solCurve.lookup(wavelength) * tauR * tauA * tauO * tauG * tauWA);
}
} // namespace PR