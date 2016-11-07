import sys
import time
import random
from real_world_interface import RealWoldInterface



api = RealWoldInterface(sys.argv[1])
api.run()

try:
    """ do whatever """
    time.sleep(2)

    print "===================="
    print "retrieving direct neighbors of 0"
    print api.get_direct_neighbors(0)
    print "===================="
    print "retrieving multi-hop neighbors of 0"
    print api.get_multihop_neighbors(0)
    print "===================="
    print "retrieving position of robot 0"
    print api.robot_get_position(0)
    print "===================="
    print "retrieving position of target 0"
    print api.target_get_position(0)
    print "===================="
    px,py=random.uniform(1,9),random.uniform(1,9)
    print "sending robot to ",px,py
    api.robot_go_to(0,px,py)
    while True:
        (x,y) = api.robot_get_position(0)
        dist_to_wp = ((x-px)**2+(y-py)**2)**0.5
        if dist_to_wp > 0.5:
            print "robot 0 has not reached wp: dist ", dist_to_wp
        else:
            break
        time.sleep(1)

    print "===================="
    px,py=random.uniform(1,9),random.uniform(1,9)
    print "sending target  to ",px,py
    api.target_go_to(0,px,py)
    while True:
        (x,y) = api.target_get_position(0)
        dist_to_wp = ((x-px)**2+(y-py)**2)**0.5
        if dist_to_wp > 0.5:
            print "target 0 has not reached wp: dist ", dist_to_wp
        else:
            break
        time.sleep(1)
    print "===================="

        
    api.stop()
except KeyboardInterrupt:
    print "ending"
    """ to stop the interface use the next two calls """
    api.stop()

print "done"

