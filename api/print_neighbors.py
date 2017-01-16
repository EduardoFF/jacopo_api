import sys
import time
from real_world_interface import RealWoldInterface



api = RealWoldInterface(sys.argv[1])
api.run()


try:
    """ do whatever """
    time.sleep(2)

    while True:
        for rid in api.robots():
            print "===================="
            print "retrieving direct neighbors of ",rid
            print api.get_direct_neighbors(rid)
            print "===================="
            print "retrieving multi-hop neighbors of ",rid
            print api.get_multihop_neighbors(rid)
            print "===================="
            time.sleep(1)

        
    api.stop()
except KeyboardInterrupt:
    print "ending"
    """ to stop the interface use the next two calls """
    api.stop()

print "done"

