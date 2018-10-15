def cie1931(L):
    L = L*100.0
    if L <= 8:
        return (L/902.3)
    else:
        return ((L+16.0)/116.0)**3

d = [cie1931(x / 16.0) for x in range(17)]
print '\n'.join(
    [' '.join(
        [('%.6g' % (x,)) + ',' for x in d[i * 1:i * 1 + 1]])
        for i in range((len(d) + 0) / 1)])
