import numpy as np
import json

mean = [0, 0, 0]
cov = [[1, 0, 0],
       [0, 1, 0],
       [0, 0, 1]]
N = 1000

samples = np.random.multivariate_normal(mean, cov, N)

with open("res/data/samples.json", "w") as f:
    f.write(json.dumps(samples.tolist(), indent=4))

