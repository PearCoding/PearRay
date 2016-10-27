import numpy as np


class Spectrum:
    def __init__(self, data):
        self.data = data
        
    
    def avg(self):
        return np.average(self.data)
        
    
    def max(self):
        return np.nanmax(self.data)
        
    
    def min(self):
        return np.nanmin(self.data)