def trapz(data, dx):
    N = len(data)-1
    s = 0
    for i in range(0, N):
        s += data[i]+data[i+1]
           
    return s * 0.5 * dx