import cv2
import argparse
import pandas as pd
import numpy as np
import os


data_dir = 'Images'      # the directory containing the images
results_dir = 'Data'   # the directory for dumping results


def normalize(points):
    # want to scale everything uniformally to be within (-1, 1)
    scale = 1.0 / max(points.T[0].max(), abs(points.T[0].min()),
                      points.T[1].max(), abs(points.T[1].min()))
    points.T[0] *= scale
    points.T[1] *= scale


def center(points):
    # make center-of-mass to 0
    center_of_mass = np.sum(points, axis=0) / len(points)
    points -= center_of_mass


def new_csv(x, y, name):
    df2 = pd.DataFrame({"dataX": x,
                        "dataY": y})
    df2.to_csv(name + ".csv", index=False)


def find_good_connections(contours, path):
    # basically completes a greedy-traveling-salesman walk which is
    # much (MUCH) faster than a complete traveling-salesman approach but may
    # produce more errors since it is only approximate
    if len(contours) == 0:
        # base case, no more contours to account for
        return path
    # find terminal points (where contours end)
    terminals = [(C[0], C[-1]) for C in contours]
    # find closest terminal point to this one
    if len(path) == 0:
        path.append(contours.pop(0))  # starting at 0
    else:
        best_idx = 1
        best_dist = np.inf
        end = path[-1][-1]
        reverse = False  # only need to reverse if closest point is the END of a contour
        for i, (cS, cE) in enumerate(terminals):
            # considering starts of this contour
            dist = ((cS[0] - end[0])**2 + (cS[1] - end[1])**2)**0.5
            if dist < best_dist:
                best_dist = dist
                best_idx = i
                reverse = False
            # also considering ends of this contour (would require reversing)
            dist = ((cE[0] - end[0])**2 + (cE[1] - end[1])**2)**0.5
            if dist < best_dist:
                best_dist = dist
                best_idx = i
                reverse = True

        # remove from contours (future considerations), append to output
        new_contour = contours.pop(best_idx)
        if reverse:
            np.flip(new_contour)
        path.append(new_contour)
    return find_good_connections(contours, path)


# TODO: add parser arg for show_edges_and_contours
def run_trace_finder(file, show_edges_and_contours=False):

    # run on all images
    if not (file.endswith(".jpg") or file.endswith(".png")):
        print("Not working on %s" % file)
        return
    file = os.path.splitext(file)[0]

    # read in images
    img = cv2.imread('%s/%s.jpg' % (data_dir, file))

    # blur the image to
    img = cv2.GaussianBlur(img, (3, 3), 0)

    # actual Hough line code function calls
    threshold1 = 100
    threshold2 = 200

    # run cv2 edge detection on the image
    img_edge = cv2.Canny(img, threshold1, threshold2, (5, 5))

    # get contours to trace along the edges
    contours, _ = cv2.findContours(img_edge, cv2.RETR_EXTERNAL,
                                   cv2.CHAIN_APPROX_NONE)

    # show edges and contours
    if show_edges_and_contours:
        cv2.imshow("Edges", img_edge)
        cv2.waitKey(0)
        cv2.destroyAllWindows()
        cv2.drawContours(img, contours, -1, (255, 0, 0), 3)
        cv2.imshow('Contours', img)
        cv2.waitKey(0)
        cv2.destroyAllWindows()

    # combine all contours together (naive)
    contours = [np.squeeze(C) for C in contours if len(C) > 5]
    points = find_good_connections(contours, [])

    # flip y values to be right-side-up
    points = np.squeeze(np.vstack(points)).astype(np.float32)
    points.T[1] *= -1

    # make sure the "center of mass" is in the middle
    center(points)

    # make sure the points are all normalized to be within -1, 1 just to not have to scale much
    normalize(points)
    # save to csv
    new_csv(points.T[0], points.T[1], os.path.join(results_dir, file))
    print("Saved %s.csv (%d datapoints)" %
          (file, len(points)))


if __name__ == "__main__":
    # read and write files
    argparser = argparse.ArgumentParser(description=__doc__)
    argparser.add_argument(
        '-n', '--name',
        metavar='N',
        default="*",  # cwd
        type=str,
        help='Name of csv data file')

    args = argparser.parse_args()
    name = args.name

    if name == "*":  # all images
        for f in os.listdir(data_dir):
            run_trace_finder(f)
    else:
        run_trace_finder(name)
