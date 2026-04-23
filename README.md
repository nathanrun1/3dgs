# 3DGS
The goal of this project is to create a tool that can render pretrained 3DGS (3D Gaussian Splatting) models. This is a fork of my "water engine" project, which is a more general material viewer that will include FFT water rendering and state-of-the-art rendering techniques.

The stack of this project is C++ and OpenGL

# 2026-04-23
Sorted on the CPU, once. Looks inaccurate from the back, but with alpha blending and if I don't move, looks pretty good! This is a good reference until we add spherical harmonics.


https://github.com/user-attachments/assets/847a1b9f-7489-45b3-a35b-6957abfedee3



# 2026-04-22
# 2
Generated 100 random splats... this is what it looks like without any sorting. Also those grazing angle artifacts seem to come up again (notice the flashing).


https://github.com/user-attachments/assets/36195432-fc37-4492-b8f3-43a352842ce6


# 1
A proper splat, no duplicate behind me (added depth culling to the compute shader). Also fixed an issue where the splat covered the screen if you look at around a 90 degree angle, this is due to the splat's x/y coordinate deviation being high as well as viewspace depth (distance along camera's axis) being low. Since the Jacobian only linearly approximates projection, it drastically overestimates the splat's actual extent on the screen, having it cover your screen as viewspace z approaches 0. So, you clamp the viewspace position used for the Jacobian calculation within some range of the view frustum to avoid the viewspace x/z or y/z becoming too large, which although inaccurate at grazing angles (which barely matters), fixes the issue unless you're unnecessarily close to the splat. Sick.
Next up:
- Multiple splats at once (will need Z sorting, ideally on the compute shader)
- Actual SH rather than a solid color

https://github.com/user-attachments/assets/ffa9edd4-2e27-49cb-8a6e-d3a042a9a83a

# 2026-04-16
## 11PM
OKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKK LFGG. Turns out you aren't supposed to normalize the bivariate normal p.d.f., i.e. you aren't supposed to use the p.d.f. directly as the opacity. My intuition wrongly assumed that the total radiance from the splat should be invariant with distance, but I forgot about the inverse square law. The total radiance per solid angle from the splat is proportional to r^2 (where r is distance to the splat), yes, but I forgot that the radiance received from that splat is proportional to 1/r^2, cancelling out that effect. So, we remove the `1/sqrt(determinant(cov))` normalization factor, and just use the transformed standard p.d.f.'s value as is. This way, the radiance (brightness) that we receive from any location on the splat does not vary with distance, just like irl.

Now just gotta get rid of this mirrored one behind me.
<video src="https://github.com/user-attachments/assets/f04f8a2d-c7d2-4c0e-8dd6-c6db8f0544f2" controls="controls" style="max-width: 100%;">
</video>

## 10PM
Niceeeeee, now the shape is correct, bright as hell tho. Density distribution is clearly off.
<video src="https://github.com/user-attachments/assets/b81de7bd-803c-400c-b863-97361bc79419" controls="controls" style="max-width: 100%;">
</video>

## 8PM
Lots of progress. Got the full pipeline drafted, but ofc did not work initially. A few issues, and turns out I wasn't even passing uniforms to the compute shader, but seems to be doing something now. Looks like the scaling is completely off.
<video src="https://github.com/user-attachments/assets/b7a4ae2f-d16e-4846-a857-db0c8c1dd237" controls="controls" style="max-width: 100%;">
</video>

# 2026-04-02
Successfully rendered 2D splats, here's some with different covariance matrices (a lot of this step was understanding how the multivariate gaussian p.d.f. is derived, and then implementing it). Examples are below.

The shape is correct, though the density doesn't seem to be. Likely an issue with the normalization $1/\sqrt{det(\Sigma)}$ (removing it increases density even with higher correlation).
<img width="1010" height="808" alt="image" src="https://github.com/user-attachments/assets/a6b5a970-6ba7-4399-b18f-b87406ac493e" />
<img width="1579" height="1150" alt="image" src="https://github.com/user-attachments/assets/bcea3935-914f-495d-a06e-cdf4c8c76f88" />



# 2026-03-29
Rendered some points finally. These are used as the ground truth for splats. Below is 1000 points with an [identity](https://en.wikipedia.org/wiki/Identity_matrix) [covariance matrix](https://www.geeksforgeeks.org/maths/covariance-matrix/):

https://github.com/user-attachments/assets/71b6fe05-a6ef-416d-a11f-59664d7178d2
