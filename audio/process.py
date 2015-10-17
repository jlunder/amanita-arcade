import wave

w = wave.open("loop-crunched.wav")
f = w.readframes(w.getnframes())
#d = [ord(f[i + 1]) for i in range(0, len(f), 2)]
#d = [(ord(f[i * 2 + 1]) + 128) % 256 for i in range(len(f) / 2)]
d = [ord(f[i + 0]) + ord(f[i + 1]) * 256
    for i in range(0, len(f), 2)]
d = [x if x < 32768 else x - 65536 for x in d]
print '\n'.join(
    [' '.join(
        [('%6d' % (x,)) + ',' for x in d[i * 8:i * 8 + 8]])
        for i in range((len(d) + 7) / 8)])
