# This command line tool records the data dumps from dreamIO board

import argparse
import copy
from time import strftime
import threading
import csv
import OSC

def timein(addr, tags, args, source):
    vals['time'] = args[0]
    #print "Time Active (s): {0}".format(args[0]/1000)

def ypr(addr, tags, args, source):
    vals['y'] = args[0]
    vals['p'] = args[1]
    vals['r'] = args[2]
    #print "Y:{0}\tP{1}\tR{2}".format(args[0], args[1], args[2])

def batt(addr, tags, args, source):
    vals['batt'] = args[0]
    #print "Battery (%): {0}".format(args[0])

def side(addr, tags, args, source):
    vals['side'] = args[0]
    #print "Current Side: {0}".format(args[0])

def debug(addr, tags, args, source):
    pass

if __name__ == "__main__":

    parser = argparse.ArgumentParser()
    parser.add_argument("endpoint", type=str, 
                        help='the dreamIO endpoint which is being recorded')
    parser.add_argument("--update", "--u", type=int, default=25,
                        help='change the broadcast update interval on dreamIO side')
    parser.add_argument("--port", "--p", type=int, default=9999,
                        help="the UDP recieve port to listen on")
    args = parser.parse_args()

    endpoint = '/{0}'.format(args.endpoint)
    
    # tupple with ip, port.
    receive_address = '', args.port #blank = 0.0.0.0, which binds to all?

    # OSC Server. there are three different types of server.
    s = OSC.OSCServer(receive_address)  # basic
    #s = OSC.ThreadingOSCServer(receive_address) # ng
    # s = OSC.ForkingOSCServer(receive_address) # forking


    s.addMsgHandler(endpoint+"/time", timein)  # adding our function
    s.addMsgHandler(endpoint+"/ypr", ypr)  # adding our function
    s.addMsgHandler(endpoint+"/batt", batt)  # adding our function
    s.addMsgHandler(endpoint+"/side", side)  # adding our function
    s.addMsgHandler(endpoint+"/debug", debug)  # adding our function

    # just checking which handlers we have added
    print("Registered Callback-functions are :")
    for addr in s.getOSCAddressSpace():
        print(addr)


    # Start OSCServer
    print("\nStarting OSCServer. Use ctrl-C to quit.")
    st = threading.Thread(target=s.serve_forever)
    st.start()

    # define a message-handle function for the server to call.
    vals = {'time':-1, 'y':-1, 'p':-1,'r':-1, 'batt':-1,'side':-1}
    out = []
    buff = 256 #number of vals to collect before store
    old_time = 0

    # write header
    today = strftime("%d-%m-%y-%s")
    output_file = open('datarecord_{0}_{1}_{2}.csv'.format(today, endpoint[1:-1], args.update) , 'wb')
    dict_writer = csv.DictWriter(output_file, vals.keys())
    dict_writer.writeheader()

    try:
        while 1:
            # if any values in dictionary are -1 then not ready to write
            if (-1 not in vals.values()):
                out.append(copy.copy(vals)) #copy to buffer
                print('recieved\tmess: {0}\ttime diff:{1}'.format(len(out),(vals['time']-old_time)))
                old_time = vals['time']
                vals = dict.fromkeys(vals, -1) #clear dict
            if(len(out) > buff):
                dict_writer.writerows(out)
                out = []
                print('recorded')

    except KeyboardInterrupt:
        print("\nSaving File.")
        if len(out) > 0:
            dict_writer.writerows(out)
        output_file.close()
        print("\nClosing OSCServer.")
        s.close()
        print("Waiting for Server-thread to finish")
        st.join()  # !!!
        print("Done")
