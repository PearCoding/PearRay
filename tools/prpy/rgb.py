import numpy as np
import parallel
import spectral

CANDELA = 683.002
ILL_SCALE = spectral.SAMPLING_COUNT + 1

NM_TO_X = [
	0.00013, 0.00023, 0.00041, 0.00074, 0.00137,
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
	0.00017, 0.00012, 8e-05, 6e-05, 4.1e-05, 
    2.9e-05, 2e-05, 1.4e-05, 1e-05]

NM_TO_Y = [
	0, 0, 1e-05, 2e-05, 4e-05,
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
	6e-05, 4e-05, 3e-05, 2e-05, 1.4e-05,
    1e-05, 7e-06, 5e-06, 3e-06]

NM_TO_Z = [
	0,0,0,0.006450, 0.010550, 0.020050, 0.036210, 0.067850,
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
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0]

Y_SUM = sum(NM_TO_Y)

assert(len(NM_TO_X) == spectral.SAMPLING_COUNT)
assert(len(NM_TO_Y) == spectral.SAMPLING_COUNT)
assert(len(NM_TO_Z) == spectral.SAMPLING_COUNT)

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
    lum = 0
    for idx, val in enumerate(spec_data):
        lum += val * NM_TO_Y[idx]
        
    return lum * (ILL_SCALE * CANDELA)

def specToXYZ(spec_data):
    rgb = np.zeros(3)
    for idx, val in enumerate(spec_data):
        rgb[0] += val * NM_TO_X[idx]
        rgb[1] += val * NM_TO_Y[idx]
        rgb[2] += val * NM_TO_Z[idx]
    
    return rgb * (ILL_SCALE / Y_SUM)

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

# Image functions
def imageToLum(width, height, full_data):
    return parallel.run(p_imageToGrayLine, range(0, height), [specToLum, width, full_data], verbose=5)

def imageToEnergy(width, height, full_data):
    return parallel.run(p_imageToGrayLine, range(0, height), [specToEnergy, width, full_data], verbose=5)
    
def imageToXYZ(width, height, full_data):
    return parallel.run(p_imageToColorLine, range(0, height), [specToXYZ, width, full_data], verbose=5)
    
def imageToRGB(width, height, full_data):
    return parallel.run(p_imageToColorLine, range(0, height), [specToRGB, width, full_data], verbose=5)