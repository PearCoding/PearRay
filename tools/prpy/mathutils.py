def trapz(f, dx):
    s = 0
    for i in range(0, len(f)-1):
        s += f[i]+f[i+1]
           
    return s * 0.5 * dx