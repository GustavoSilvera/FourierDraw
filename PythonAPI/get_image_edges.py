import cv2
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
    center_of_mass = np.sum(points, axis=0) / len(points)
    # make center-of-mass to 0
    points -= center_of_mass


def new_csv(x, y, name):
    df2 = pd.DataFrame({"dataX": x,
                        "dataY": y})
    df2.to_csv(name + ".csv", index=False)


def run_edge_filter():
    for file in os.listdir(data_dir):
        if file.endswith('.jpg'):
            print("Working on", file)
            file = os.path.splitext(file)[0]

            # read in images
            img = cv2.imread('%s/%s.jpg' % (data_dir, file))

            img = cv2.GaussianBlur(img, (3, 3), 0)

            # actual Hough line code function calls
            threshold1 = 0
            threshold2 = 200
            # 255 is edge, 0 is none, no values in bw
            img_edge = cv2.Canny(img, threshold1, threshold2, (5, 5))

            # get contours to trace along the edges
            contours, _ = cv2.findContours(img_edge, cv2.RETR_EXTERNAL,
                                           cv2.CHAIN_APPROX_NONE)

            # show image
            if True:
                cv2.imshow("Edges", img_edge)
                cv2.waitKey(0)
                cv2.destroyAllWindows()
                cv2.drawContours(img, contours, -1, (255, 0, 0), 3)
                cv2.imshow('Contours', img)
                cv2.waitKey(0)
                cv2.destroyAllWindows()

            # combine all contours together
            points = np.squeeze(np.vstack(contours)).astype(np.float32)

            # flip y values to be right-side-up
            points.T[1] *= -1

            import matplotlib.pyplot as plt
            plt.clf()
            plt.plot(points.T[0], points.T[1])
            plt.show()

            center(points)

            normalize(points)

            # save to csv
            new_csv(points.T[0], points.T[1], os.path.join(results_dir, file))


if __name__ == "__main__":
    run_edge_filter()
