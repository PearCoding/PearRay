import zlib
import math
import numpy as np
import rgb
import spectral
import struct

import OpenEXR # Sadly only for python 2 :(
import Imath

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
            
	offset = 0
	if data[0:4] == b'PR42':
		offset = 4
	elif data[0:5] == b'PRS42': # Old format --- Will be removed soon
		offset = 5
	else:
            print('Invalid PearRay spectral file!')
            return False
        
        self.samples = struct.unpack('<i', data[offset:(offset+4)])[0]
        
        if not(self.samples == spectral.SAMPLING_COUNT):
            print('Sample count missmatch. Has to be %i!' % spectral.SAMPLING_COUNT)
            return False
        
	offset += 4
        self.width = struct.unpack('<i', data[offset:(offset+4)])[0]
	offset += 4
        self.height = struct.unpack('<i', data[offset:(offset+4)])[0]
	offset += 4
        #print("%i %i %i" % (self.samples, self.width, self.height))
        
        bf = np.frombuffer(data, dtype=np.dtype('<f4'), offset=offset)
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
    
    
    def energy(self):
        arr = np.zeros((self.height, self.width))
        for j in range(0, self.height):
            for i in range(0, self.width):
                arr[j, i] = self.get(i,j).energy()
        return arr
    
    
    def normalized(self):
        f = SpectralFile()
        f.samples = self.samples
        f.width = self.width
        f.height = self.height
        f.spectrals = np.zeros((self.height, self.width, self.samples))
        
        m = self.fmax()        
        im = 1.0/m;
        if math.isnan(im) or math.isinf(im):
            return f
        
        for j in range(0, self.height):
            for i in range(0, self.width):
                for k in range(0, self.samples):
                    f.spectrals[j,i,k] = self.spectrals[j,i,k] * im
        return f
    
    
class SingleChannelFile:
    def __init__(self, channel):
        self.width = 0
        self.height = 0
        self.data = None
        self.channel = channel
        
        
    def open(self, path):
        f = OpenEXR.InputFile(path)
        if not f:
            return False
        
        dw = f.header()['dataWindow']
        sz = (dw.max.x - dw.min.x + 1, dw.max.y - dw.min.y + 1)
        
        self.width = sz[0]
        self.height = sz[1]
        bf = np.frombuffer(f.channel(self.channel, Imath.PixelType(Imath.PixelType.FLOAT)), dtype=np.dtype('<f4'))
        self.data = np.reshape(bf, newshape=(self.height, self.width), order='C')
        
        return True

   
    def get(self, x, y):
        return self.data[y,x]
    
    
    def avg(self):
        return np.average(self.data)
                
    
    def max(self):
        return np.nanmax(self.data)
                
    
    def min(self):
        return np.nanmin(self.data)
                
    
    def normalized(self):
        f = SingleChannelFile(self.channel)
        f.width = self.width
        f.height = self.height
        f.data = np.zeros((self.height, self.width))
        
        m = self.max()        
        im = 1.0/m;
        if math.isnan(im) or math.isinf(im):
            return f
        
        for j in range(0, self.height):
            for i in range(0, self.width):
                f.data[j,i] = self.data[j,i] * im
        return f
        
