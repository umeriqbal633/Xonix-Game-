import numpy as np

# Define the Lagrangian
# L = -w^2 - x^2 - y^2 - z^2 + lambda1 * (4w - 3y + z + 15) + lambda2 * (-2x - y + z + 5)

# Partial derivatives of Lagrangian
# L_w = -2w + 4*lambda1
# L_x = -2x - 2*lambda2
# L_y = -2y - 3*lambda1 - lambda2
# L_z = -2z + lambda1 + lambda2
# L_lambda1 = 4w - 3y + z + 15
# L_lambda2 = -2x - y + z + 5

# Solve the system of equations numerically
def equations(vars):
    w, x, y, z, lambda1, lambda2 = vars
    return [
        -2*w + 4*lambda1,
        -2*x - 2*lambda2,
        -2*y - 3*lambda1 - lambda2,
        -2*z + lambda1 + lambda2,
        4*w - 3*y + z + 15,
        -2*x - y + z + 5
    ]

# Use scipy to find critical point
from scipy.optimize import fsolve
initial_guess = [0, 0, 0, 0, 0, 0]
critical_point = fsolve(equations, initial_guess)
w, x, y, z, lambda1, lambda2 = critical_point

# Construct the Bordered Hessian matrix
# Variables ordered as [w, x, y, z, lambda1, lambda2]
# Second derivatives of Lagrangian
bordered_hessian = np.array([
    [-2, 0, 0, 0, 4, 0],    # L_ww, L_wx, L_wy, L_wz, L_w_lambda1, L_w_lambda2
    [0, -2, 0, 0, 0, -2],   # L_xw, L_xx, L_xy, L_xz, L_x_lambda1, L_x_lambda2
    [0, 0, -2, 0, -3, -1],  # L_yw, L_yx, L_yy, L_yz, L_y_lambda1, L_y_lambda2
    [0, 0, 0, -2, 1, 1],    # L_zw, L_zx, L_zy, L_zz, L_z_lambda1, L_z_lambda2
    [4, 0, -3, 1, 0, 0],    # L_lambda1_w, L_lambda1_x, L_lambda1_y, L_lambda1_z, L_lambda1_lambda1, L_lambda1_lambda2
    [0, -2, -1, 1, 0, 0]    # L_lambda2_w, L_lambda2_x, L_lambda2_y, L_lambda2_z, L_lambda2_lambda1, L_lambda2_lambda2
])

# Extract top-left 5x5 submatrix
submatrix_5x5 = bordered_hessian[:5, :5]

# Calculate determinants
det_5x5 = np.linalg.det(submatrix_5x5)
det_full = np.linalg.det(bordered_hessian)

# Print results
print(f"Determinant of top-left 5x5 submatrix: {det_5x5}")
print(f"Determinant of full Bordered Hessian: {det_full}")

# For a maximum, with 2 constraints (m=2) and 4 variables (n=4),
# the determinant of the bordered Hessian (6x6) should be positive,
# and the top-left 5x5 should be negative (for n-m+1 = 4-2+1 = 3rd principal minor)
if det_full > 0 and det_5x5 < 0:
    print("The critical point is a maximum.")
else:
    print("The critical point is not a maximum (possibly a minimum or saddle point).")