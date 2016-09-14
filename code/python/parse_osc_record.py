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
    # print "total time: {}".format(total_time)

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
    std_time = np.std(time_diff)

    # % of values less than mean + 1 x std
    std_1_percent = np.sum(time_diff < (mean_time + std_time))/float(len(time_diff))
    # % of values less than mean + 2 x std
    std_2_percent = np.sum(time_diff < (mean_time + (2 * std_time)))/float(len(time_diff))

    
    # print "time diff:"
    # print "Min: {}".format(min_time)
    # print "Max: {}".format(max_time)
    # print "Mean: {}".format(mean_time)
    # print "Median: {}".format(median_time)
    # print "Std: {}".format(std_time)

    # time_hist = np.bincount(time_diff.astype('int64'))
    # largest_val = np.argmax(time_hist)
    # time_error_hist =  time_hist / float(largest_val) #the error

    #how many packets lost
    temp_packet = data['packet'] - data['packet'][0]  #first packet starts at zero, adjust the rest
    packet_diff = np.diff(temp_packet)
    max_pack = np.max(packet_diff)
    min_pack = np.min(packet_diff)
    mean_pack = np.mean(packet_diff)
    median_pack = np.median(packet_diff)
    std_pack = np.std(packet_diff)

    # % of values less than mean + 1 x std
    std_1_percent_pack = np.sum(packet_diff < (mean_pack + (1 * std_pack)))/float(len(packet_diff))
    # % of values less than mean + 2 x std
    std_2_percent_pack = np.sum(packet_diff < (mean_pack + (2 * std_pack)))/float(len(packet_diff))
    
    print "{}, {}, {}, {}, {}".format(args.filename, std_1_percent, std_2_percent, std_1_percent_pack, std_2_percent_pack)
    # print "packet diff:"
    # print "Min: {}".format(min_pack)
    # print "Max: {}".format(max_pack)
    # print "Mean: {}".format(mean_pack)
    # print "Median: {}".format(median_pack)
    # print "Std: {}".format(std_pack)

    #compute the packet error (% of packets lost)
    #error = np.sum(packet_diff > 1.) / float(len(packet_diff))
    #print "Error: {}".format(error)

    # packet_hist = np.bincount(packet_diff.astype('int64'))
    # packet_largest_val = np.argmax(packet_hist)
    # packet_error_hist =  packet_hist / float(len(packet_diff)) #the error

    #print "{}, {}, {}, {}, {}, {}, {}".format(args.filename, total_time, min_time, max_time, mean_time, median_time, std_time)#, min_pack, max_pack, mean_pack, median_pack, std_pack, error)


