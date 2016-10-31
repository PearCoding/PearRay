from joblib import Parallel, delayed
import multiprocessing
import itertools

def range2D(sx,ex,sy,ey):
    return itertools.product(range(sx,ex), range(sy,ey))
    
def run(f,r,args=None, threads=0, verbose=0):
    if threads == 0:
        threads = multiprocessing.cpu_count()
    
    if args:
        return Parallel(n_jobs=threads, verbose=verbose)(delayed(f)(i, *args) for i in r)
    else:
        return Parallel(n_jobs=threads, verbose=verbose)(delayed(f)(i) for i in r)
        