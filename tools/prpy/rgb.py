import numpy as np
import parallel
import spectral
import mathutils

PHO_K = 683.002
SCO_K = 1699

ILL_SCALE = spectral.SAMPLING_COUNT + 1

# CIE 1931 standard colormetric observer
PHO_X = [
	0.001368, 0.002236, 0.004243, 0.007650, 0.014310,
    0.023190, 0.043510, 0.077630, 0.134380, 0.214770,
    0.283900, 0.328500, 0.348280, 0.348060, 0.336200,
    0.318700, 0.290800, 0.251100, 0.195360, 0.142100,
    0.095640, 0.057950, 0.032010, 0.014700, 0.004900,
    0.002400, 0.009300, 0.029100, 0.063270, 0.109600,
    0.165500, 0.225750, 0.290400, 0.359700, 0.433450,
    0.512050, 0.594500, 0.678400, 0.762100, 0.842500,
    0.916300, 0.978600, 1.026300, 1.056700, 1.062200,
    1.045600, 1.002600, 0.938400, 0.854450, 0.751400,
    0.642400, 0.541900, 0.447900, 0.360800, 0.283500,
    0.218700, 0.164900, 0.121200, 0.087400, 0.063600,
    0.046770, 0.032900, 0.022700, 0.015840, 0.011359,
    0.008111, 0.005790, 0.004109, 0.002899, 0.002049,
    0.001440, 0.001000, 0.000690, 0.000476, 0.000332,
    0.000235, 0.000166, 0.000117, 0.000083, 0.000059,
    0.000042,]

PHO_Y = [
	0.000039, 0.000064, 0.000120, 0.000217, 0.000396,
    0.000640, 0.001210, 0.002180, 0.004000, 0.007300,
    0.011600, 0.016840, 0.023000, 0.029800, 0.038000,
    0.048000, 0.060000, 0.073900, 0.090980, 0.112600,
    0.139020, 0.169300, 0.208020, 0.258600, 0.323000,
    0.407300, 0.503000, 0.608200, 0.710000, 0.793200,
    0.862000, 0.914850, 0.954000, 0.980300, 0.994950,
    1.000000, 0.995000, 0.978600, 0.952000, 0.915400,
    0.870000, 0.816300, 0.757000, 0.694900, 0.631000,
    0.566800, 0.503000, 0.441200, 0.381000, 0.321000,
    0.265000, 0.217000, 0.175000, 0.138200, 0.107000,
    0.081600, 0.061000, 0.044580, 0.032000, 0.023200,
    0.017000, 0.011920, 0.008210, 0.005723, 0.004102,
    0.002929, 0.002091, 0.001484, 0.001047, 0.000740,
    0.000520, 0.000361, 0.000249, 0.000172, 0.000120,
    0.000085, 0.000060, 0.000042, 0.000030, 0.000021,
    0.000015,]

PHO_Z = [
    0.006450, 0.010550, 0.020050, 0.036210, 0.067850,
	0.110200, 0.207400, 0.371300, 0.645600, 1.039050,
	1.385600, 1.622960, 1.747060, 1.782600, 1.772110,
	1.744100, 1.669200, 1.528100, 1.287640, 1.041900,
	0.812950, 0.616200, 0.465180, 0.353300, 0.272000,
	0.212300, 0.158200, 0.111700, 0.078250, 0.057250,
	0.042160, 0.029840, 0.020300, 0.013400, 0.008750,
	0.005750, 0.003900, 0.002750, 0.002100, 0.001800,
	0.001650, 0.001400, 0.001100, 0.001000, 0.000800,
	0.000600, 0.000340, 0.000240, 0.000190, 0.000100,
	0.000050, 0.000030, 0.000020, 0.000010, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 
    0,]

PHO_Y_SUM = sum(PHO_Y)

assert(len(PHO_X) == spectral.SAMPLING_COUNT)
assert(len(PHO_Y) == spectral.SAMPLING_COUNT)
assert(len(PHO_Z) == spectral.SAMPLING_COUNT)

SCO_Y = [
    5.890e-004, 1.108e-003, 2.209e-003, 4.530e-003, 9.290e-003,
    1.852e-002, 3.484e-002, 6.040e-002, 9.660e-002, 1.436e-001,
    1.998e-001, 2.625e-001, 3.281e-001, 3.931e-001, 4.550e-001,
    5.130e-001, 5.670e-001, 6.200e-001, 6.760e-001, 7.340e-001,
    7.930e-001, 8.510e-001, 9.040e-001, 9.490e-001, 9.820e-001,
    9.980e-001, 9.970e-001, 9.750e-001, 9.350e-001, 8.800e-001,
    8.110e-001, 7.330e-001, 6.500e-001, 5.640e-001, 4.810e-001,
    4.020e-001, 3.288e-001, 2.639e-001, 2.076e-001, 1.602e-001,
    1.212e-001, 8.990e-002, 6.550e-002, 4.690e-002, 3.315e-002,
    2.312e-002, 1.593e-002, 1.088e-002, 7.370e-003, 4.970e-003,
    3.335e-003, 2.235e-003, 1.497e-003, 1.005e-003, 6.770e-004,
    4.590e-004, 3.129e-004, 2.146e-004, 1.480e-004, 1.026e-004,
    7.150e-005, 5.010e-005, 3.533e-005, 2.501e-005, 1.780e-005,
    1.273e-005, 9.140e-006, 6.600e-006, 4.780e-006, 3.482e-006,
    2.546e-006, 1.870e-006, 1.379e-006, 1.022e-006, 7.600e-007,
    5.670e-007, 4.250e-007, 3.196e-007, 2.413e-007, 1.829e-007,
    1.390e-007,]

SCO_Y_SUM = sum(SCO_Y)
assert(len(SCO_Y) == spectral.SAMPLING_COUNT)

M_D65 = np.matrix([[ 0.9531874, -0.0265906, 0.0238731],
                   [-0.0382467,  1.0288406, 0.0094060],
                   [ 0.0026068, -0.0030332, 1.0892565]])

M_RGB = np.matrix([[ 3.240479, -1.537150, -0.498535],
                   [-0.969256,  1.875991,  0.041556],
                   [ 0.055648, -0.204043,  1.057311]])

M_F_RGB = M_RGB * M_D65
#print(M_F_RGB)

def specToEnergy(spec_data):
    return spectral.Spectrum(spec_data).energy()

def specToLum(spec_data):
    y_data = spec_data * PHO_Y      
    return mathutils.trapz(y_data, spectral.WAVELENGTH_STEP*1e-9) * PHO_K

def specToLum_Sco(spec_data):
    y_data = spec_data * SCO_Y      
    return mathutils.trapz(y_data, spectral.WAVELENGTH_STEP*1e-9) * SCO_K

def specToXYZ(spec_data):    
    return np.array([mathutils.trapz(spec_data * PHO_X, spectral.WAVELENGTH_STEP*1e-9),
            mathutils.trapz(spec_data * PHO_Y, spectral.WAVELENGTH_STEP*1e-9),
            mathutils.trapz(spec_data * PHO_Z, spectral.WAVELENGTH_STEP*1e-9)])

def specToRGB(spec_data):
    return np.fmax(M_F_RGB.dot(specToXYZ(spec_data)), [0,0,0])

# Parallel functions
def p_imageToGrayLine(r, mapper, width, full_data):
    data = np.zeros(width)
    for i in range(0, width):
        data[i] = mapper(full_data[r,i,:])
    return data

def p_imageToColorLine(r, mapper, width, full_data):
    data = np.zeros((width, 3))
    for i in range(0, width):
        data[i,:] = mapper(full_data[r,i,:])
    return data

VERBOSE_LEVEL=5
# Image functions
def imageToLum(width, height, full_data, do_parallel=True):
    if do_parallel:
        return parallel.run(p_imageToGrayLine, range(0, height), [specToLum, width, full_data], verbose=VERBOSE_LEVEL)
    else:
        return [p_imageToGrayLine(i, specToLum, width, full_data) for i in range(0, height)]

def imageToLum_Sco(width, height, full_data, do_parallel=True):
    if do_parallel:
        return parallel.run(p_imageToGrayLine, range(0, height), [specToLum_Sco, width, full_data], verbose=VERBOSE_LEVEL)
    else:
        return [p_imageToGrayLine(i, specToLum_Sco, width, full_data) for i in range(0, height)]

def imageToEnergy(width, height, full_data, do_parallel=True):
    if do_parallel:
        return parallel.run(p_imageToGrayLine, range(0, height), [specToEnergy, width, full_data], verbose=VERBOSE_LEVEL)
    else:
        return [p_imageToGrayLine(i, specToEnergy, width, full_data) for i in range(0, height)]
    
def imageToXYZ(width, height, full_data, do_parallel=True):
    if do_parallel:
        return parallel.run(p_imageToColorLine, range(0, height), [specToXYZ, width, full_data], verbose=VERBOSE_LEVEL)
    else:
        return [p_imageToColorLine(i, specToXYZ, width, full_data) for i in range(0, height)]
    
def imageToRGB(width, height, full_data, do_parallel=True):
    if do_parallel:
        return parallel.run(p_imageToColorLine, range(0, height), [specToRGB, width, full_data], verbose=VERBOSE_LEVEL)
    else:
        return [p_imageToColorLine(i, specToRGB, width, full_data) for i in range(0, height)]