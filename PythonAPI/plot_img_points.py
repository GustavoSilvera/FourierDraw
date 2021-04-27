#!/usr/bin/env python

import argparse
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import os

from get_image_edges import new_csv, center, normalize


def draw_points():
    # read and write files
    argparser = argparse.ArgumentParser(description=__doc__)
    argparser.add_argument(
        '-n', '--name',
        metavar='N',
        default="Scotty",  # cwd
        type=str,
        help='Name of csv data file')
    args = argparser.parse_args()
    directory = "Data/"
    name = args.name
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

    center(points)

    normalize(points)

    new_csv(points.T[0], points.T[1], os.path.join(directory, name + "_norm"))

    plt.clf()
    plt.plot(points.T[0], points.T[1])
    plt.savefig(name + ".png")
    plt.show()


if __name__ == '__main__':
    draw_points()
