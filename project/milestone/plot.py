#!/usr/bin/python
import matplotlib
from matplotlib import colors
from matplotlib import pyplot
import numpy as np


my_color = {"grey": colors.colorConverter.to_rgb("#4D4D4D"),
            "blue": colors.colorConverter.to_rgb("#5DA5DA"),
            "orange": colors.colorConverter.to_rgb("#FAA43A"),
            "green": colors.colorConverter.to_rgb("#60BD68"),
            "pink": colors.colorConverter.to_rgb("#F17CB0"),
            "brown": colors.colorConverter.to_rgb("#B2912F"),
            "purple": colors.colorConverter.to_rgb("#B276B2"),
            "yellow": colors.colorConverter.to_rgb("#DECF3F"),
            "red": colors.colorConverter.to_rgb("#F15854")}
x = np.log2(np.array([128, 256, 512, 1024, 2048, 4096, 8192, 16384]))
x1 = np.array([128, 256, 512, 1024, 2048, 4096, 8192, 16384])
y1 = np.log10(1000000 * np.array([0.0000004,0.0000008,0.0000017,0.0000028,0.0000136,0.0000115,0.0000282,0.0000425]))
y2 = np.log10(1000000 * np.array([0.0000412,0.0000848,0.0001875,0.0008761,0.0028076,0.0057515,0.0150475,0.0351845]))

# y2x = np.array(15640273.7048, 32010003.1260, 55351351.3514, 69189189.1892)
# y3x = np.array([10427486.2019, 20830790.5122, 38143485.0630, 65565373.2872])
# y2 = np.array([696694.1231, 2781131.335, 9905468.6984, 32230220.92])
fig, ax1 = pyplot.subplots()
ax1.plot(x, y1, color=my_color["blue"],
         label='Row sum', linewidth=2.0, marker="o")

ax1.plot(x, y2, color=my_color["orange"],
         label='Column sum', linewidth=2.0, marker="^")

lines, labels = ax1.get_legend_handles_labels()
ax1.set_xlabel("Matrix Dimension", fontsize=20)
ax1.set_ylabel("Average task completion time (us)", fontsize=20)
ax1.legend(lines, labels, loc="upper left")
ax1.set_xticks(x)


#pyplot.savefig("ingestion.eps")
pyplot.savefig("rowmajor.pdf")
