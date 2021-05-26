import matplotlib.pyplot as plt
import numpy as np

Ts = 30;   # signal interval
end = 150; # signal end point
n = int(end/Ts)+1;

x = np.linspace(0, end, num=n) # signal vector

# TODO: revise this array to your results
# clockwise
y = np.array([0.000, 6.378, 12.118, 12.995, 12.835, 13.154]) # speed vector

# counterclockwise
# y = np.array([0.000, 4.943, 11.002, 12.596, 12.676, 12.516]) 

z = np.polyfit(x, y, 2) # Least squares polynomial fit, and return the coefficients.

goal = 5             # if we want to let the servo run at 7 cm/sec
                     # equation : z[0]*x^2 + z[1]*x + z[2] = goal
z[2] -= goal         # z[0]*x^2 + z[1]*x + z[2] - goal = 0

result = np.roots(z) # Return the roots of a polynomial with coefficients given

# output the correct one
if (0 <= result[0]) and (result[0] <= end):
    print(result[0])
else:
    print(result[1])
