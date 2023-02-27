import statistics
import sys


data = []
inputfile = sys.argv[1]
outputfile = sys.argv[2]
with open(inputfile, "r") as f:
    for line in f.readlines():
        data.append(float(line))

with open(outputfile, "w") as f:
    f.write(f"{statistics.mean(data)}, {statistics.stdev(data)}\n")
