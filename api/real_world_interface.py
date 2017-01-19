'''
Created on Feb 10, 2016

@author: banfi
'''

import lcm
from configmsg import config_msg_t as config_msg
import threading
from xml.dom import minidom
###Note: both robot and target structures have an id ranging from 0 to Nrobots -1 (Ntargets -1, resp.) 
###E.g. (robot.id) 0,...,(n_robots -1), 0,...,(n_targets - 1)

#################################################
#                    ROBOTS
#################################################

####################
#ATTRIBUTES TO QUERY
####################

class RealWoldInterface():
    def __init__(self, config_file):
        self.robot_to_fb = {}
        self.target_to_fb = {}
        self.all_fbs=set()
        self.fb_pos = {}
        self.fb_to_target = {}
        self.fb_to_robot = {}

        self.lcm = lcm.LCM("udpm://239.255.76.67:7667?ttl=1") 
#        self.lcm = lcm.LCM()
        self.subscription = self.lcm.subscribe("API", self.my_handler)
        self.done = False
        self.configureScenario(config_file)
        self.tid = threading.Thread(target=self.handler)
        self.verbose = False
        self.neighbors = {}
        print "init_done"

    def stop(self):
        self.done = True
        self.join()
    def run(self):
        self.tid.start()
    def join(self):
        self.tid.join()
    def robots(self):
        return self.robot_to_fb.keys()
    def targets(self):
        return self.target_to_fb.keys()
    def handler(self):
        while True:
            self.lcm.handle_timeout(1000)
            if self.done:
                break
            
    def my_handler(self, channel, data):
        msg = config_msg.decode(data)
        if self.verbose:
            print("   robot    = {}".format(msg.robotid)),
            print("   timestamp    = {}".format(msg.timestamp)),
            print("   msg    = {}".format(msg.msg))
        s = msg.msg.split()
        if not len(s):
            return
        fid = int(msg.robotid)
        if fid not in self.all_fbs:
            return
        if s[0] == 'POS':
            x=float(s[1])
            y=float(s[2])
            if self.verbose:
                print "set position of ",fid," to ",x,y
            self.fb_pos[fid] = (float(s[1]), float(s[2]))
        if s[0] == 'NTABLE':
            if fid in self.fb_to_robot:
                rid = self.fb_to_robot[fid]
                self.neighbors[rid]=dict()
                for i in range(2,len(s),2):
                    if int(s[i]) in self.fb_to_robot:
                        nid = self.fb_to_robot[int(s[i])]
                        self.neighbors[rid][nid] = int(s[i+1])
            


    def configureScenario(self, cfile):
        xmldoc = minidom.parse(cfile)
        itemlist = xmldoc.getElementsByTagName('robot')
        n_targets=0
        n_robots=0
        for s in itemlist :
            rid = int(s.attributes['robotid'].value)
            t = s.attributes['type'].value
            if t == 'target':
                self.target_to_fb[n_targets]=rid
                self.fb_to_target[rid] = n_targets
                n_targets+=1

            elif t == 'robot':
                self.robot_to_fb[n_robots]=rid
                self.fb_to_robot[rid]=n_robots
                n_robots+=1
            else:
                print "Invalid type ",t
                exit(1)
            self.all_fbs.add(rid)


    def robot_get_position(self, rid):
        if rid in self.robot_to_fb:
            fid = self.robot_to_fb[rid]
            if fid in self.fb_pos:
                return self.fb_pos[fid]
            else:
                print "can not find position for footbot",fid
                
        else:
            print "invalid robot id"
            print "robot ids are ", self.robot_to_fb.keys()
            
    #...

    #robot.x = x
    #robot.y = y

    def get_direct_neighbors(self, rid):
        if rid in self.robot_to_fb:
            if rid not in self.neighbors:
                return []
            else:
                return [n for (n,d) in self.neighbors[rid].items() if d == 1]
        else:
            print "invalid robot id"
            return []

    #...

    #robot.direct_neighbors = direct_neighbors #list of integer IDs [robotid1, robotid2, ...]


    def get_multihop_neighbors(self, rid):
        if rid in self.robot_to_fb:
            if rid not in self.neighbors:
                return []
            else:
                return [n for (n,d) in self.neighbors[rid].items() if d > 1]
        else:
            print "invalid robot id"
            return []

    #...

    #robot.neighbors = neighbors #list of integer IDs as above

    def get_observed_targets(self, robot):
        """
            Only the list of the target that the robot is DIRECTLY observing.
            In post-processing, I will add also targets observed by a 
            multihop neighbor robot.
        """
        pass
    #...

    #robot.observing = observing #list of tuples of the kind [(target_id1, (x_target1, y_target1) ), ...]

    ###################
    #COMMANDS
    # speed in meter per second
    # max value is 0.3, if <= 0, then it is ignore
    # if None, it uses the default speed
    ###################

    def robot_go_to(self, rid,x,y,speed=None):
        """
            The robot must move to position (x,y)
        """
        if rid in self.robot_to_fb:
            fid = self.robot_to_fb[rid]
            msg = config_msg()
            msg.robotid = fid
            msg.timestamp = 0
            if speed:
                msg.msg ="FORCE_POS_VEL %.2f %.2f 0 %.2f"%(x,y,speed)
            else:
                msg.msg ="FORCE_POS %.2f %.2f 0"%(x,y)
            self.lcm.publish("CONFIG", msg.encode())
        else:
            print "invalid robot id"

        

    #################################################
    #                    TARGETS
    #################################################

    ####################
    #ATTRIBUTES TO QUERY
    ####################

    def target_get_position(self, rid):
        if rid in self.target_to_fb:
            fid = self.target_to_fb[rid]
            if fid in self.fb_pos:
                return self.fb_pos[fid]
            else:
                print "can not find position for footbot",fid
                
        else:
            print "invalid robot id"
            print "target ids are ", self.target_to_fb.keys()

        #...

        #target.x = x
        #target.y = y

    ###################
    #COMMANDS
    # speed in meter per second
    # max value is 0.3, if <= 0, then it is ignore
    # if None, it uses the default speed
    ###################

    def target_go_to(self, rid,x,y, speed=None):
        """
            The target must move to position (x,y)
        """
        if rid in self.target_to_fb:
            fid = self.target_to_fb[rid]
            msg = config_msg()
            msg.robotid = fid
            msg.timestamp = 0
            if speed:
                msg.msg ="FORCE_POS_VEL %.2f %.2f 0 %.2f"%(x,y,speed)
            else:
                msg.msg ="FORCE_POS %.2f %.2f 0"%(x,y)                
            self.lcm.publish("CONFIG", msg.encode())
        else:
            print "invalid target id"

