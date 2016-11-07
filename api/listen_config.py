import lcm
import sys
from solve.lcm_defs import *

def my_handler(channel, data):
    msg = config_msg_t.decode(data)
#    print("Received message on channel \"%s\"" % channel)
#    print("   timestamp   = %d" % msg.timestamp)
    print("   robot    = {}".format(msg.robotid)),
    print("   timestamp    = {}".format(msg.timestamp)),
    print("   msg    = {}".format(msg.msg))
#    print("   n    = %d" % msg.n)

lc = lcm.LCM()
subscription = lc.subscribe(sys.argv[1], my_handler)

try:
    while True:
        lc.handle()
except KeyboardInterrupt:
    pass

lc.unsubscribe(subscription)
