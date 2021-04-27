#!/usr/bin/env python

import pandas as pd
import matplotlib.pyplot as plt
import os


def draw_points():
    # read and write files
    data_path = os.path.join(os.getcwd(), "Data.csv")
    with open(data_path, 'r') as data_csv:
        df = pd.read_csv(data_csv)
    points_x = []
    points_y = []
    for x in df["dataX"]:
        points_x.append(x)
    for y in df["dataY"]:
        points_y.append(y)

    plt.plot(points_x, points_y)
    plt.savefig("train.png")
    plt.show()


if __name__ == '__main__':
    draw_points()
