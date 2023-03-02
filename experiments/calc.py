import statistics
import sys


data = []
inputfile = sys.argv[1]
outputfile = sys.argv[2]
with open(inputfile, "r") as f:
    for line in f.readlines():
        data.append(float(line))

with open(outputfile, "w") as f:
    stdev = 0
    if len(data) > 1:
        stdev = statistics.stdev(data)
    f.write(f"{statistics.mean(data)}, {stdev}\n")
