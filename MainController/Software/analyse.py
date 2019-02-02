data = [480, 248, 277, 228, 728, 514, 1950, 505, 1896, 556, 1894, 507, 967, 507, 1947, 503, 971, 505, 918, 556, 917, 558, 916, 510, 1915, 556, 918, 556, 405, 556, 3850, 558, 916, 507, 967, 505, 1949, 505, 1896, 556, 1895, 558, 1894, 507, 1945, 505, 1909, 556, 1896, 556, 918, 505, 969, 507, 968, 504, 971, 505, 918, 556, 918, 505, 969, 507, 1946, 505, 969, 505, 1896, 556, 916, 560, 916, 505, 1911, 556, 1896, 556, 1898, 503, 1947, 507, 1894, 559, 1894, 556, 1896, 556, 918, 505, 1945, 507, 918, 558, 916, 556, 918, 556, 930, 505, 1946, 507, 960, 521, 443, 509, 3864, 558, 920, 505, 967, 507, 1946, 506, 1895, 556, 1896, 556, 1896, 556, 1894, 543, 1895, 507, 1945, 505, 919, 556, 918, 556, 916, 558, 916, 507, 969, 503, 920, 556, 918, 556, 1895, 558, 916, 558, 1894, 505, 969, 505, 918, 556, 1907, 558, 1894, 507, 1896, 556, 1896, 556, 1897, 556, 1894, 558, 1894, 507, 916, 556, 1898, 556, 918, 556, 917, 558, 916, 507, 929, 556, 1896, 556, 918, 556, 403, 558, 3850, 558, 918, 556, 918, 505, 1896, 557, 1894, 558, 1894, 558, 1894, 505, 1908, 556, 1896, 556, 1896, 545, 914, 520, 970, 508, 918, 556, 918, 556, 918, 558, 916, 507, 917, 556, 1896, 556, 918, 556, 1894, 558, 918, 558, 865, 591, 1858, 556, 1896, 556, 1895, 558, 1894, 558, 1896, 503, 1898, 556, 1896, 556, 917, 558, 1894, 558, 865, 556, 918, 556, 931, 556, 916, 558, 1896, 558, 865, 554, 178307]

size = len(data)
print("size:", size)

print("high values:")
highs = []
for count in range(size):
    if data[count] > 3000:
        highs.append(count)
        print(f" {count}/{data[count]}", end='')
        if len(highs) > 1:
            diff = highs[-1] - highs[-2]
            print(f"/{diff}", end='')
print()

# Grab the first sequence between (and including the last) two long pulses.
if len(highs) > 1:
    data = data[highs[0]+1:highs[1]+1]
    print(data)
    size = len(data)
    print("size:", size)

# Try a simple high/low - low/hihgh code
cutof = 500
code = ""
for count in range(0, size, 2):
    if data[count] >= cutof and data[count+1] < cutof:
        code += "1"
    elif data[count] < cutof and data[count+1] >= cutof:
        code += "0"
    else:
        code += "?"
print(code)


# Try a tri-state code
cutof1 = 700
cutof2 = 1200
code = ""
for count in range(0, size, 2):
    if data[count] < cutof1 and data[count+1] < cutof2:
        code += "0"
    elif data[count] < cutof1 and data[count+1] >= cutof2:
        code += "1"
    else:
        code += "?"
print(code)

#print(data[66:132])