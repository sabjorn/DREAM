#a script to automatically parse an OSC csv recording
import numpy as np

filename = 'datarecord_31-07-16-1469949684_0e2b0_10_r0g0b0.csv'

data = np.genfromtxt(filename, delimiter=',', names=True)

#find total length of time
total_time = (data['time'][-1] - data['time'][0]) / 1000. / 60. / 60.
print total_time

#normalizae battery 
max_batt = np.max(data['batt'])
min_batt = np.min(data['batt'])
data['batt'] = (data['batt'] - min_batt) / (max_batt - min_batt)

#time between frames
time_diff = np.diff(data['time'])
mean_time = np.mean(time_diff)
median_time = np.median(time_diff)
max_time = np.max(time_diff)
min_time = np.min(time_diff)
