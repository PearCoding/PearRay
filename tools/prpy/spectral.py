import numpy as np
import math

WAVELENGTH_START = 360# nm
WAVELENGTH_END = 800# nm 
WAVELENGTH_AREA_SIZE = (WAVELENGTH_END - WAVELENGTH_START)
WAVELENGTH_STEP = 5;# nm
SAMPLING_COUNT = int(WAVELENGTH_AREA_SIZE / WAVELENGTH_STEP + 1)

def blackbody_equation(temp, wavelength):
    C = 299792458
    H = 6.626070040e-34
    kB = 1.38064852e-23

    c1 = 2*H*C*C
    c2 = H*C/kB
    
    return (c1 / (wavelength**5)) / (math.exp(c2 / (wavelength * temp)) - 1)

        
class Spectrum:
    def __init__(self, data):
        self.data = data
        
    
    def avg(self):
        return np.average(self.data)
        
    
    def max(self):
        return np.nanmax(self.data)
        
    
    def min(self):
        return np.nanmin(self.data)
    
    
    def energy(self):#trapz
        s = 0
        for i in range(0, SAMPLING_COUNT-1):
            s += self.data[i]+self.data[i+1]
           
        return s * 0.5 * WAVELENGTH_STEP / (SAMPLING_COUNT-1)
    
    
    # Blackbody equation
    def from_temperature(temp):
        spec = Spectrum(np.zeros(SAMPLING_COUNT))

        for i in range(0, SAMPLING_COUNT):
            lambda_nm = (WAVELENGTH_START + i * WAVELENGTH_STEP)*1e-9
            spec.data[i] = blackbody_equation(temp, lambda_nm)
        
        return spec
    
    
    def wavelength_range():
        return range(WAVELENGTH_START, WAVELENGTH_END+WAVELENGTH_STEP, WAVELENGTH_STEP)
    