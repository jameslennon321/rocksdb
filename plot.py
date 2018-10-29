import sys
import numpy as np
import matplotlib.pyplot as plt


def smoothed(data, amt=100):
	result = []
	for i in range(len(data) - amt):
		end = min(i + amt, len(data))
		result.append(np.mean(data[i:end]))
	return result

def normalized(data):
    return np.array(data) / np.mean(data)

# filename = sys.argv[1]
filenames = ("bout", "rout")
for f in filenames:
	fname = "experiments/{}.txt".format(f)
	data = np.loadtxt(fname)
	plt.plot(normalized(smoothed(data)), label=f)
plt.legend()
plt.show()

