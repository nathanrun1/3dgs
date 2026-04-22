from dataclasses import dataclass

import numpy as np
import json
from plyfile import PlyData, PlyElement

from scipy.spatial.transform import Rotation as R
from scipy.stats import random_correlation

def _diagonalize(matrix):
    """Diagonalizes the given matrix as M = PDP^-1, returns P, D"""
    eigenvalues, eigenvectors = np.linalg.eig(matrix)
    return eigenvectors, np.diag(eigenvalues)

@dataclass
class Splat:
    position: np.ndarray
    """3d vector, splat position in world space"""
    cov: np.ndarray
    """3x3 covariance matrix"""
    color: np.ndarray
    """4d vector, splat color"""

    def ply_tuple(self) -> tuple:
        """Generates a tuple laying out the splat's data in the same format as it would be placed in a .ply file"""
        rot_mat, scale_mat = _diagonalize(self.cov)
        scale_mat = np.sqrt(scale_mat)
        print(f"rot: {rot_mat}\nscale: {scale_mat}")
        rot = R.from_matrix(rot_mat, assume_valid=True)
        print(self.color)
        return tuple(np.concat([self.position, rot.as_quat().tolist(), np.diag(scale_mat).tolist(), self.color]))
        return tuple(self.position + rot.as_quat().tolist() + np.diag(scale_mat).tolist() + self.color)


    def sample(self, n = 100) -> np.ndarray:
        """Generates samples from this splat's distribution"""
        return np.random.multivariate_normal(self.position, self.cov, n)


def _rand_cov(std_range = (0.1, 0.5)) -> np.ndarray:
    """Generates a random 3D covariance matrix, with stdev for each axis generated between the given range"""
    low, high = std_range
    stds = np.random.uniform(low, high, 3)
    print("stds: " + str(stds))

    eigs = np.random.dirichlet(np.ones(3)) * 3
    corr = random_correlation.rvs(eigs)

    print("corr: " + str(corr))

    D = np.diag(stds)
    cov = D @ corr @ D
    print("cov: " + str(cov))

    return cov


def _rand_splat(min_pos: np.ndarray = np.repeat(-5.0, 3), max_pos: np.ndarray = np.repeat(5.0, 3), std_range = (0.1, 0.5)) -> Splat:
    """Generates a random 3D Gaussian Splat"""
    return Splat(
        position=np.random.uniform(min_pos, max_pos),
        color=np.concatenate([np.random.uniform([0, 0, 0], [1, 1, 1]), [1]]),
        cov=_rand_cov(std_range)
    )


NUM_SPLATS = 100
NUM_SAMPLES = 0

splats: list[Splat] = [_rand_splat() for _ in range(NUM_SPLATS)]

if NUM_SAMPLES > 0:
    samples = np.concatenate([s.sample() for s in splats])
    with open("../res/data/samples.json", "w") as f:
        f.write(json.dumps(samples.tolist(), indent=4))

splat_data = np.array([s.ply_tuple() for s in splats],
                  dtype=[('x', 'f4'), ('y', 'f4'), ('z', 'f4'), ('r_0', 'f4'), ('r_1', 'f4'), ('r_2', 'f4'),
                         ('r_3', 'f4'), ('s_0', 'f4'), ('s_1', 'f4'), ('s_2', 'f4'), ('r', 'f4'), ('g', 'f4'),
                         ('b', 'f4'), ('a', 'f4')])

e1 = PlyElement.describe(splat_data, 'splat')

PlyData([e1], text=True).write("../res/data/splat.ply")
