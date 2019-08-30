#!/usr/bin/python


import sys
from mpl_toolkits.mplot3d import Axes3D
import matplotlib.pyplot as plt
from matplotlib import cm
import numpy as np

def main():
    if (len(sys.argv) < 2):
        print("usage: python plotter.py input.txt")
        exit(0)
    cps = np.loadtxt(sys.argv[1])

    fig = plt.figure()
    ax = fig.gca(projection='3d')

    X = np.arange(-cps.shape[1]/2, cps.shape[1]/2, 1.0)
    Y = np.arange(-cps.shape[0]/2, cps.shape[0]/2, 1.0)

    X, Y = np.meshgrid(X, Y)
    Z = cps

    max_loc = np.unravel_index(np.argmax(cps), cps.shape)
    x_shift = max_loc[1] - (cps.shape[1] / 2)
    y_shift = max_loc[0] - (cps.shape[0] / 2)

    print('Max location ', x_shift, y_shift)

    surf = ax.plot_surface(X, Y, Z, color='gray', rstride=1, cstride=1,
                           linewidth=0, antialiased=False)
    ax.text(x_shift, y_shift, np.max(cps), '(%d, %d)' % (x_shift, y_shift), size=20, zorder=1, color='k')
    plt.show()


if __name__ == '__main__':
    main()

