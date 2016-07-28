import socket
import argparse
import copy
from time import strftime
import threading
import csv
import OSC

UDP_IP = '0.0.0.0' #binds to all IPs.
UDP_PORT = 9999 #initial UDP port for 
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))

#temp
endpoint = 'mock'
args = {''}

# define a message-handle function for the server to call.
vals = {'time':-1, 'y':-1, 'p':-1,'r':-1, 'batt':-1,'side':-1}
out = []
buff = 256 #number of vals to collect before store
old_time = 0

# write header
today = strftime("%d-%m-%y-%s")
output_file = open('temp.csv', 'wb')
#output_file = open('datarecord_{0}_{1}_{2}_r{3}g{4}b{5}.csv'
#                   .format(today, endpoint[1:-1], args.update, args.red, args.green, args.blue) , 'wb')
dict_writer = csv.DictWriter(output_file, vals.keys())
dict_writer.writeheader()

try:
    while 1:
        data, addr = sock.recvfrom(2048)
        data = OSC.decodeOSC(data)[2:] #get rid of header
        
        vals['time'] = data[0][2]
        vals['y'] = data[1][2]
        vals['p'] = data[1][3]
        vals['r'] = data[1][4]
        vals['batt'] = data[2][2]
        vals['side'] = data[3][2]
        
        out.append(vals)
        print('recieved\tmess: {0}\ttime diff:{1}'.format(len(out),(vals['time']-old_time)))
        old_time = vals['time']

        if(len(out) > buff):
            dict_writer.writerows(out)
            out = []
            print('recorded')

except KeyboardInterrupt:
    print("\nSaving File.")
    if len(out) > 0:
        dict_writer.writerows(out)
    output_file.close()
