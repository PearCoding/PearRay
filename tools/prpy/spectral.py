import numpy as np

WAVELENGTH_START = 360# nm
WAVELENGTH_END = 800# nm 
WAVELENGTH_AREA_SIZE = (WAVELENGTH_END - WAVELENGTH_START)
WAVELENGTH_STEP = 5;# nm
SAMPLING_COUNT = int(WAVELENGTH_AREA_SIZE / WAVELENGTH_STEP + 1)
        
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