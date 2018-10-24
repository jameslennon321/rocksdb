import sys
import numpy as np
import matplotlib.pyplot as plt


def smoothed(data, amt=50):
	result = []
	for i in range(len(data) - amt):
		end = min(i + amt, len(data))
		result.append(np.mean(data[i:end]))
	return result

filename = sys.argv[1]
data = np.loadtxt(filename)

plt.plot(smoothed(data))
plt.show()


