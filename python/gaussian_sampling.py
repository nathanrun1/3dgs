import numpy as np
import json
from plyfile import PlyData, PlyElement

from scipy.spatial.transform import Rotation as R

color = [1, 0, 0, 1]
mean = [5, 0, 0]

corr = np.array([[1  , 0.5 , 0   ],
                 [0.5, 1   , -0.5],
                 [0  , -0.5, 1   ]])
std = np.array([[.5, 0, 0],
                [0, .5, 0],
                [0, 0, .5]])

cov = std @ corr @ std
N = 100

samples = np.random.multivariate_normal(mean, cov, N)

with open("../res/data/samples.json", "w") as f:
    f.write(json.dumps(samples.tolist(), indent=4))

def _diagonalize(matrix):
    """Diagonalizes the given matrix as M = PDP^-1, returns P, D"""
    eigenvalues, eigenvectors = np.linalg.eig(cov)
    return eigenvectors, np.diag(eigenvalues)


rot_mat, scale_mat = _diagonalize(cov)

rot = R.from_matrix(rot_mat)
print(rot.as_quat())
print(np.diag(scale_mat))

splats = np.array([tuple(mean + rot.as_quat().tolist() + np.diag(scale_mat).tolist() + color)],
                  dtype=[('x', 'f4'), ('y', 'f4'), ('z', 'f4'), ('r_0', 'f4'), ('r_1', 'f4'), ('r_2', 'f4'),
                         ('r_3', 'f4'), ('s_0', 'f4'), ('s_1', 'f4'), ('s_2', 'f4'), ('r', 'f4'), ('g', 'f4'),
                         ('b', 'f4'), ('a', 'f4')])

e1 = PlyElement.describe(splats, 'splat')

PlyData([e1], text=True).write("../res/data/splat.ply")
