import OSC
from time import sleep
import threading

endpoint = "/0e2b06"
# tupple with ip, port.
receive_address = '', 9999 #blank = 0.0.0.0, which binds to all?


# OSC Server. there are three different types of server.
#s = OSC.OSCServer(receive_address)  # basic
s = OSC.ThreadingOSCServer(receive_address) # ng
# s = OSC.ForkingOSCServer(receive_address) # forking


# this registers a 'default' handler (for unmatched messages),
# an /'error' handler, an '/info' handler.
# And, if the client supports iet, a '/subscribe' & '/unsubscribe' handler
s.addDefaultHandlers()


# define a message-handle function for the server to call.

def time(addr, tags, args, source):
    print "Time Active (s): {0}".format(args[0]/1000)

def ypr(addr, tags, args, source):
    print "Y:{0}\tP{1}\tR{2}".format(args[0], args[1], args[2])

def batt(addr, tags, args, source):
    print "Battery (%): {0}".format(args[0])

def side(addr, tags, args, source):
    print "Current Side: {0}".format(args[0])

def debug(addr, tags, args, source):
    pass



s.addMsgHandler(endpoint+"/time", time)  # adding our function
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


try:
    while 1:
        sleep(1)
except KeyboardInterrupt:
    print("\nClosing OSCServer.")
    s.close()
    print("Waiting for Server-thread to finish")
    st.join()  # !!!
    print("Done")
