import numpy as np
import parallel
import spectral
import mathutils

PHO_K = 683.002
SCO_K = 1699

ILL_SCALE = spectral.SAMPLING_COUNT + 1

PHO_X = [
	0.00074, 0.00137,
    0.00223, 0.00424, 0.00765, 0.01431, 0.02319,
	0.04351, 0.07763, 0.13438, 0.21477, 0.2839,
    0.3285, 0.34828, 0.34806, 0.3362, 0.3187,
	0.2908, 0.2511, 0.19536, 0.1421, 0.09564,
    0.05795, 0.03201, 0.0147, 0.0049, 0.0024,
	0.0093, 0.0291, 0.06327, 0.1096, 0.1655,
    0.22575, 0.2904, 0.3597, 0.43345, 0.51205,
	0.5945, 0.6784, 0.7621, 0.8425, 0.9163,
    0.9786, 1.0263, 1.0567, 1.0622, 1.0456,
	1.0026, 0.9384, 0.85445, 0.7514, 0.6424, 
    0.5419, 0.4479, 0.3608, 0.2835, 0.2187,
	0.1649, 0.1212, 0.0874, 0.0636, 0.04677,
    0.0329, 0.0227, 0.01584, 0.01136, 0.00811,
	0.00579, 0.00411, 0.00289, 0.00205, 0.00144, 
    0.001, 0.00069, 0.00048, 0.00033, 0.00023,
	0.00017, 0.00012, 8e-05, 6e-05,]

PHO_Y = [
	2e-05, 4e-05,
    6e-05, 0.00012, 0.00022, 0.0004, 0.00064,
	0.0012, 0.00218, 0.004, 0.0073, 0.0116,
    0.01684, 0.023, 0.0298, 0.038, 0.048,
	0.06, 0.0739, 0.09098, 0.1126, 0.13902,
    0.1693, 0.20802, 0.2586, 0.323, 0.4073,
	0.503, 0.6082, 0.71, 0.7932, 0.862,
    0.91485, 0.954, 0.9803, 0.99495, 1,
	0.995, 0.9786, 0.952, 0.9154, 0.87,
    0.8163, 0.757, 0.6949, 0.631, 0.5668,
	0.503, 0.4412, 0.381, 0.321, 0.265, 
    0.217, 0.175, 0.1382, 0.107, 0.0816,
	0.061, 0.04458, 0.032, 0.0232, 0.017,
    0.01192, 0.00821, 0.00573, 0.0041, 0.00293,
	0.00209, 0.00105, 0.00105, 0.00074, 0.00052,
    0.00036, 0.00025, 0.00017, 0.00012, 8e-05,
	6e-05, 4e-05, 3e-05, 2e-05,]

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
	0.000050, 0.000030, 0.000020, 0.000010, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,]

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
    rgb = np.zeros(3)
    for idx, val in enumerate(spec_data):
        rgb[0] += val * PHO_X[idx]
        rgb[1] += val * PHO_Y[idx]
        rgb[2] += val * PHO_Z[idx]
    
    return rgb * (ILL_SCALE / PHO_Y_SUM)

def specToRGB(spec_data):
    return specToXYZ(spec_data) * M_F_RGB

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
def imageToLum(width, height, full_data):
    return parallel.run(p_imageToGrayLine, range(0, height), [specToLum, width, full_data], verbose=VERBOSE_LEVEL)

def imageToLum_Sco(width, height, full_data):
    return parallel.run(p_imageToGrayLine, range(0, height), [specToLum_Sco, width, full_data], verbose=VERBOSE_LEVEL)

def imageToEnergy(width, height, full_data):
    return parallel.run(p_imageToGrayLine, range(0, height), [specToEnergy, width, full_data], verbose=VERBOSE_LEVEL)
    
def imageToXYZ(width, height, full_data):
    return parallel.run(p_imageToColorLine, range(0, height), [specToXYZ, width, full_data], verbose=VERBOSE_LEVEL)
    
def imageToRGB(width, height, full_data):
    return parallel.run(p_imageToColorLine, range(0, height), [specToRGB, width, full_data], verbose=VERBOSE_LEVEL)