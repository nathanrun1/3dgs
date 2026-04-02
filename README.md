# 3DGS
The goal of this project is to create a tool that can render pretrained 3DGS (3D Gaussian Splatting) models. This is a fork of my "water engine" project, which is a more general material viewer that will include FFT water rendering and state-of-the-art rendering techniques.

The stack of this project is C++ and OpenGL

# 2026-04-02
Successfully rendered 2D splats, here's some with different covariance matrices (a lot of this step was understanding how the multivariate gaussian p.d.f. is derived, and then implementing it). Examples are below.

The shape is correct, though the density doesn't seem to be. Likely an issue with the normalization $1/\sqrt{det(\Sigma)}$ (removing it increases density even with higher correlation).
<img width="1010" height="808" alt="image" src="https://github.com/user-attachments/assets/a6b5a970-6ba7-4399-b18f-b87406ac493e" />
<img width="1579" height="1150" alt="image" src="https://github.com/user-attachments/assets/bcea3935-914f-495d-a06e-cdf4c8c76f88" />



# 2026-03-29
Rendered some points finally. These are used as the ground truth for splats. Below is 1000 points with an [identity](https://en.wikipedia.org/wiki/Identity_matrix) [covariance matrix](https://www.geeksforgeeks.org/maths/covariance-matrix/):

https://github.com/user-attachments/assets/71b6fe05-a6ef-416d-a11f-59664d7178d2
