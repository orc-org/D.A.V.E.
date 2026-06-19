import numpy as np
from pykalman import AdditiveUnscentedKalmanFilter
import matplotlib.pyplot as plt

#measuring speed of the IMU, 10Hz which is 10 measurements per second (can be set to anything), higher speed means more ram alocated
dt = 0.1 

#this matrix feeds into the filter and predicts future values of the data. 15x15
def transition_function(state): 
    
    new_state = np.copy(state)
    
    new_state[0:3] += state[3:6] * dt + 0.5 * state[6:9] * dt ** 2
    new_state[3:6] += state[6:9] * dt
    
    new_state[9:12] += state[12:15] * dt
    
    return new_state

#this matrix takes the actual data recorded, observations matrix, and uses it to predict future data. 1x6
def observation_function(state):
    
    accel_part = state[6:9]
    
    gyro_part = state[12:15]
    
    return np.concatenate([accel_part, gyro_part])

#this matrix tells us how certain we are with the relationship between our predicted values. 15 x 15
transition_covariance = np.eye(15) * 10**(-2)

#this matrix tells us how certain we are in our observations. 6x6, about 5-10% of the range and then squared to find variance.

observation_covariance = np.diag([1,1,1,0.04,0.04,0.04])

#this matrix is the inital measurements for all our values. 1x15
initial_state_mean = np.zeros(15)

#this matrix tells us how certain we are in our initial measurements. 15x15
initial_state_covariance = np.eye(15) * 10**(2)

#this inializes the filter. Has a hidden 6x15 matrix that bridges all the matrices together
aukf = AdditiveUnscentedKalmanFilter(
    transition_functions = transition_function,
    observation_functions = observation_function,
    initial_state_mean = initial_state_mean,
    initial_state_covariance = initial_state_covariance,
    transition_covariance = transition_covariance,
    observation_covariance = observation_covariance 
)

#this is the data being feed into the filter. 1x6
n = 90

c1 = -10 + (np.arange(n) * 7 % 30)
c2 = -10 + (np.arange(n) * 11 % 30)
c3 = -10 + (np.arange(n) * 13 % 30)

base = np.linspace(-2, 2, n)
c4 = base
c5 = base + 0.03 * np.sin(np.arange(n))
c6 = base - 0.04 * np.cos(np.arange(n))

observations = np.column_stack([c1, c2, c3, c4, c5, c6])

#this filters the observations we have
filtered_state_means_ekf, filtered_state_covariances_ekf = aukf.filter(observations)


#should probably maybe perhaps show the finished data
plt.plot(observations[:,0], label='Observations', linestyle='dotted')
plt.plot(filtered_state_means_ekf[:,6], label='EKF Filtered Position')
plt.title('Extended Kalman Filter: True Position, Observations, and Filtered Position')
plt.xlabel('Time')
plt.ylabel('Value')
plt.legend()
plt.show()
