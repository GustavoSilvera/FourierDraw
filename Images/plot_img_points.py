#!/usr/bin/env python

import argparse
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import os


def new_csv(x, y, name):
    df2 = pd.DataFrame({"dataX": x,
                        "dataY": y})
    df2.to_csv(name + ".csv", index=False)


def normalize(points):
    # want to scale everything uniformally to be within (-1, 1)
    scale = 1.0 / max(points.T[0].max(), abs(points.T[0].min()),
                      points.T[1].max(), abs(points.T[1].min()))
    points.T[0] *= scale
    points.T[1] *= scale


def center(points):
    center_of_mass = np.sum(points, axis=0) / len(points)
    # make center-of-mass to 0
    points -= center_of_mass


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
    name = args.name
    data_path = os.path.join(os.getcwd(), name + ".csv")
    with open(data_path, 'r') as data_csv:
        df = pd.read_csv(data_csv)
    points_x = []
    points_y = []
    for x in df["dataX"]:
        points_x.append(x)
    for y in df["dataY"]:
        points_y.append(y)

    points = np.vstack([points_x, points_y]).T

    # center(points)

    # normalize(points)

    # new_csv(points.T[0], points.T[1], name + "_norm")

    plt.clf()
    plt.plot(points.T[0], points.T[1])
    plt.savefig(name + ".png")
    plt.show()


if __name__ == '__main__':
    draw_points()
