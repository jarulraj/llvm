#!/usr/bin/python
import matplotlib
matplotlib.use('Agg')

from matplotlib import colors
from matplotlib import pyplot as plt
import numpy as np
import re
import sys

if (len(sys.argv) != 6):
    print "%s outfile ylabel workload1:label:data,workload2:label:data... baselineinput,input2,input3... baseline_desc,desc2,desc3 " % sys.argv[0]
    sys.exit(0)

outfile = sys.argv[1]
y_label = sys.argv[2]
workloads = sys.argv[3].split(",")
input_files = sys.argv[4].split(",")
descs = sys.argv[5].split(",")
perfs = {}
workload_data = {}
workload_label = {}
workload_names = []
print sys.argv
for i,w in enumerate(workloads):
    tokens = w.split(":")
    name = tokens[0]
    data = tokens[2]
    label = tokens[1]
    perfs[name] = {}
    workload_data[name] = data
    workload_label[name] = label
    workload_names.append(name)

for input_file_prefix in input_files:
    for w in workload_data.keys():
        input_file = "%s%s" % (input_file_prefix, w)
        with open(input_file, 'r') as f:
            perfs[w][input_file_prefix] = {}
            print "w=%s input=%s\n" % (w, input_file_prefix)
            for line in f:
                if re.match('.+D1  miss rate:\W+(\d+.\d+)\W+\(\W+(\d+.\d+)\%\W+\+\W+(\d+.\d+).+', line):
                    tokens = re.split('.+D1  miss rate:\W+(\d+.\d+)\W+\(\W+(\d+.\d+)\%\W+\+\W+(\d+.\d+).+', line)
                    perfs[w][input_file_prefix]['d1_overall'] = float(tokens[1])
                    perfs[w][input_file_prefix]['d1_read']= float(tokens[2])
                    perfs[w][input_file_prefix]['d1_write'] = float(tokens[3])
                elif re.match('.+LLd miss rate:\W+(\d+.\d+)\W+\(\W+(\d+.\d+)\%\W+\+\W+(\d+.\d+).+', line):
                    tokens = re.split('.+LLd miss rate:\W+(\d+.\d+)\W+\(\W+(\d+.\d+)\%\W+\+\W+(\d+.\d+).+', line)
                    print tokens
                    perfs[w][input_file_prefix]['lld_overall'] = float(tokens[1])
                    perfs[w][input_file_prefix]['lld_read'] = float(tokens[2])
                    perfs[w][input_file_prefix]['lld_write'] = float(tokens[3])
                elif re.match('Duration :: (\d+.\d+).+', line):
                    tokens = re.split('Duration :: (\d+.\d+).+', line)[1]
                    print tokens
                    perfs[w][input_file_prefix]['runtime'] = float(tokens)

print perfs

my_color = {"grey": colors.colorConverter.to_rgb("#4D4D4D"),
            "blue": colors.colorConverter.to_rgb("#5DA5DA"),
            "orange": colors.colorConverter.to_rgb("#FAA43A"),
            "green": colors.colorConverter.to_rgb("#60BD68"),
            "pink": colors.colorConverter.to_rgb("#F17CB0"),
            "brown": colors.colorConverter.to_rgb("#B2912F"),
            "purple": colors.colorConverter.to_rgb("#B276B2"),
            "yellow": colors.colorConverter.to_rgb("#DECF3F"),
            "red": colors.colorConverter.to_rgb("#F15854")}

N = len(input_files)
ind = np.arange(N)
width = (1.0 - 0.2) / N

fig, ax = plt.subplots()

bar_handles = []
bar_legendhandles = []
bar_labels = []

real_data_for_plot = []
for w in workload_names:
    data = []
    for input_file in input_files:
        #print w
        #print input_file
        #print workload_data[w]
        data.append(perfs[w][input_file][workload_data[w]])
    data = np.array(data)
    real_data_for_plot.append(data)

print real_data_for_plot
for i,w in enumerate(workload_names):
    data = real_data_for_plot[i]
    baseline = real_data_for_plot[0][0]
    print data
    data = np.array(data)
    data = data/baseline
    #data = np.log(data)
    idx = len(bar_handles)
    handle = ax.bar(ind + idx * width, data, width, color=my_color[my_color.keys()[idx]])
    bar_handles.append(handle)
    bar_legendhandles.append(handle[0])
    bar_labels.append(workload_label[w])

ax.set_ylabel(y_label)
ax.set_xticks(ind + 0.8/2)
ax.set_xticklabels(descs)
ax.legend(bar_legendhandles, bar_labels, loc='upper left')

plt.savefig(outfile)


