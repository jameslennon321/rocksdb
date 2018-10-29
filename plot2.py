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
filenames = {"bout" : "B-Tree", "rout": "LSM Tree"}
ideal_data = np.ones(3900) * 100.
for f in filenames:
	fname = "experiments/{}.txt".format(f)
	data = normalized(smoothed(np.loadtxt(fname)[4000:8000]))
	ideal_data = np.minimum(data, ideal_data)
	plt.plot(data, label=filenames[f])

ideal_data = [np.mean(ideal_data[:2000])] * 2000 + [np.mean(ideal_data[2000:])] * 1900

plt.plot(ideal_data, linestyle="--", label="Ideal Data Structure")

plt.xlabel("Query")
plt.ylabel("Latency (normalized)")
plt.title("")

plt.legend()
plt.show()

