import argparse
import numpy as np
import json
from plyfile import PlyData, PlyElement
from scipy.spatial.transform import Rotation as R


PLY_DTYPE = [
    ('x', 'f4'), ('y', 'f4'), ('z', 'f4'),
    ('r_0', 'f4'), ('r_1', 'f4'), ('r_2', 'f4'), ('r_3', 'f4'),
    ('s_0', 'f4'), ('s_1', 'f4'), ('s_2', 'f4'),
    ('r', 'f4'), ('g', 'f4'), ('b', 'f4'), ('a', 'f4'),
]


def rand_cov_batch(n, std_range=(0.1, 0.5)):
    """Generate n random 3x3 covariance matrices via random rotation + random scale."""
    low, high = std_range
    stds = np.random.uniform(low, high, (n, 3))
    # Random rotation matrices — much faster than random_correlation.rvs
    rot_mats = R.random(n).as_matrix()
    # Cov = R @ diag(std^2) @ R^T
    scaled = rot_mats * (stds[:, np.newaxis, :] ** 2)  # (n,3,3) * (n,1,3) broadcasts
    covs = scaled @ np.swapaxes(rot_mats, -1, -2)
    return covs


def generate_splat_array(n, splat_range=5.0, std_range=(0.1, 0.5)):
    positions = np.random.uniform(-splat_range, splat_range, (n, 3))
    colors = np.column_stack([
        np.random.uniform(0, 1, (n, 3)),
        np.ones(n),
    ])
    covs = rand_cov_batch(n, std_range)

    # Eigendecompose all covariance matrices at once
    eigenvalues, eigenvectors = np.linalg.eigh(covs)  # eigh since covs are symmetric
    scales = np.sqrt(np.clip(eigenvalues, 0, None))  # (n, 3)
    quats = R.from_matrix(eigenvectors, assume_valid=True).as_quat()     # (n, 4)

    # Pack into structured array
    raw = np.column_stack([positions, quats, scales, colors]).astype(np.float32)
    result = np.empty(n, dtype=PLY_DTYPE)
    for i, (name, _) in enumerate(PLY_DTYPE):
        result[name] = raw[:, i]
    return result


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--num-splats", "-p", type=int, default=100)
    parser.add_argument("--num-samples", "-s", type=int, default=0)
    parser.add_argument("--splat-range", "-r", type=float, default=5.0)
    args = parser.parse_args()
    assert args.splat_range >= 0

    splat_data = generate_splat_array(args.num_splats, args.splat_range)

    if args.num_samples > 0:
        # Regenerate covs for sampling (or refactor to keep them)
        positions = np.column_stack([splat_data['x'], splat_data['y'], splat_data['z']])
        covs = rand_cov_batch(args.num_splats)
        samples = np.vstack([
            np.random.multivariate_normal(positions[i], covs[i], args.num_samples)
            for i in range(args.num_splats)
        ])
        with open("../res/data/samples.json", "w") as f:
            json.dump(samples.tolist(), f, indent=4)

    PlyData([PlyElement.describe(splat_data, 'splat')], text=True).write("../res/data/splat.ply")