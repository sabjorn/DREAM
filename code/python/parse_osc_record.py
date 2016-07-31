#a script to automatically parse an OSC csv recording
import argparse
import numpy as np

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("filename", type=str, 
                        help='the name of the csv file to be loaded.')
    
    args = parser.parse_args()

    data = np.genfromtxt(args.filename, delimiter=',', names=True)

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

    time_hist = np.bincount(time_diff.astype('int64'))
    largest_val = np.argmax(time_hist)
    time_error_hist =  time_hist / float(len(time_diff)) #the error

    #how many packets lost
    packet_diff = np.diff(data['packet'])
    max_pack = np.max(packet_diff)
    min_pack = np.min(packet_diff)
    mean_pack = np.mean(packet_diff)
    median_pack = np.median(packet_diff)

    #compute the packet error (% of packets lost)
    error = np.sum(packet_diff > 1.) / float(len(packet_diff))

    packet_hist = np.bincount(packet_diff.astype('int64'))
    packet_largest_val = np.argmax(packet_hist)
    packet_error_hist =  packet_hist / float(len(packet_diff)) #the error