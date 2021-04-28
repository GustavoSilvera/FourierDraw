#!/usr/bin/env python

import argparse
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import os


def draw(directory, name):
    data_path = os.path.join(os.getcwd(),
                             os.path.join(directory, name + ".csv"))
    with open(data_path, 'r') as data_csv:
        df = pd.read_csv(data_csv)
    points_x = []
    points_y = []
    for x in df["dataX"]:
        points_x.append(x)
    for y in df["dataY"]:
        points_y.append(y)

    points = np.vstack([points_x, points_y]).T

    plt.clf()
    plt.plot(points.T[0], points.T[1])
    plt.show()


def draw_points():
    # read and write files
    argparser = argparse.ArgumentParser(description=__doc__)
    argparser.add_argument(
        '-n', '--name',
        metavar='N',
        default="*",  # cwd
        type=str,
        help='Name of csv data file')
    args = argparser.parse_args()
    data_dir = "Data/"
    name = args.name
    if name == "*":  # all images
        for f in os.listdir(data_dir):
            draw(data_dir, os.path.splitext(f)[0])
    else:
        draw(data_dir, name)


if __name__ == '__main__':
    draw_points()
