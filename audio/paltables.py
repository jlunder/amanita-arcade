def lintab(r0, g0, b0, r1, g1, b1, steps):
    return [
        (r0 + ((r1 - r0) * i) / (steps),
        g0 + ((g1 - g0) * i) / (steps),
        b0 + ((b1 - b0) * i) / (steps),)
        for i in range(steps)]

def printtab(d):
    print '\n'.join(
        [' '.join(
            [('{%4d, %4d, %4d},' % x) for x in d[i * 4:i * 4 + 4]])
            for i in range((len(d) + 3) / 4)])

orangepink = (
    lintab(   0,   0,   0,  256, 256,   0,  64) +
    lintab( 256, 256,   0,  256, 512, 256, 128) +
    lintab(2048, 512, 256, 4096, 256, 512,  64)
    )

printtab(orangepink)
