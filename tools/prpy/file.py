import zlib
import math
import numpy as np
import rgb
import spectral


class SpectralFile:
    def __init__(self):
        self.samples = 0
        self.width = 0
        self.height = 0
        self.spectrals = None
        
        
    def open(self, path):
        with open(path, mode='rb') as f:
            if not f:
                print('Couldn\' open file %s' % path)
                return False

            raw = f.read()
        
        if not raw:
            print('Empty file!')
            return False
        
        data = zlib.decompress(raw)
        
        if not data:
            print('Empty PearRay spectral file!')
            return False
            
        if not(data[0:5] == b'PRS42'):
            print('Invalid PearRay spectral file!')
            return False
        
        self.samples = int.from_bytes(data[5:9], byteorder='little')
        
        if not(self.samples == spectral.SAMPLING_COUNT):
            print('Sample count missmatch. Has to be %i!' % spectral.SAMPLING_COUNT)
            return False
            
        self.width = int.from_bytes(data[9:13], byteorder='little')
        self.height = int.from_bytes(data[13:17], byteorder='little')
        #print("%i %i %i" % (self.samples, self.width, self.height))
        
        bf = np.frombuffer(data, dtype=np.dtype('<f4'), offset=17)
        #print("%i %i" % (len(bf), self.samples * self.width * self.height))
        self.spectrals = np.reshape(bf, newshape=(self.height, self.width, self.samples), order='C')
        
        return True
    
   
    def get(self, x, y):
        return spectral.Spectrum(self.spectrals[y,x,:])
    
    
    def avg(self):
        arr = np.zeros((self.height, self.width))
        for j in range(0, self.height):
            for i in range(0, self.width):
                arr[j, i] += self.get(i,j).avg()
        return arr
                
    
    def max(self):
        arr = np.zeros((self.height, self.width))
        for j in range(0, self.height):
            for i in range(0, self.width):
                arr[j, i] += self.get(i,j).max()
        return arr
                
    
    def min(self):
        arr = np.zeros((self.height, self.width))
        for j in range(0, self.height):
            for i in range(0, self.width):
                arr[j, i] += self.get(i,j).min()
        return arr
                
    
    def favg(self):
        return np.average(self.spectrals)
                
    
    def fmax(self):
        return np.nanmax(self.spectrals)
                
    
    def fmin(self):
        return np.nanmin(self.spectrals)
    
    
    def normalized(self):
        f = SpectralFile()
        f.samples = self.samples
        f.width = self.width
        f.height = self.height
        f.spectrals = np.zeros((self.height, self.width, self.samples))
        
        m = self.fmax()        
        im = 1.0/m;
        if not math.isfinite(im):
            return f
        
        for j in range(0, self.height):
            for i in range(0, self.width):
                for k in range(0, self.samples):
                    f.spectrals[j,i,k] = self.spectrals[j,i,k] * im
        return f
        