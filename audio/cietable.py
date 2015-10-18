def cie1931(L):
    L = L*100.0
    if L <= 8:
        return (L/902.3)
    else:
        return ((L+16.0)/116.0)**3

d = [int(round(cie1931(x / 4095.0) * 255)) for x in range(4096)]
print '\n'.join(
    [' '.join(
        [('%3d' % (x,)) + ',' for x in d[i * 8:i * 8 + 8]])
        for i in range((len(d) + 7) / 8)])
