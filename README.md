#

# create docker images

## Build RoboNetSim image
```
git clone https://github.com/EduardoFF/RoboNetSim.git
cd RoboNetSim
docker build -t robonetsim/argos .
```

## Build image with the API controllers

```
cd ../sim
docker build -t argos_sim .
```

# Run simulation

adjust the permissions of the X server

```
xhost +local:root
```

then, run the simulator:
```
bash run_sim.sh argos_sim bash launch_argos.sh SIMTIME NUMBER_OF_ROBOTS SEED
```

where:
SIMTIME: simulation time in seconds

NUMBER_OF_ROBOTS: number of robots in the simulation

SEED: integer, random seed

you should see the ARGoS gui, click play


remember after you are finished using the simulator, return access control that were disabled

```
xhost -local:root
```

# Test API

```
cd api
python api.py example_config.xml
```

you should run the python script while the simulation is running, so
choose a long time for sim_time e.g., 3600 seconds.

The python script shows how to use the api, it retrieves some data
(like neighbors, positions), and sends robot 0, and target 0 to random
points. You should see an output like this one:

init_done
====================
retrieving direct neighbors of 0
[2]
====================
retrieving multi-hop neighbors of 0
[6]
====================
retrieving position of robot 0
(6.495, 8.79)
====================
retrieving position of target 0
(5.032, 8.658)
====================
sending robot to  1.20476207735 8.86676155247
robot 0 has not reached wp: dist  5.29079479985
robot 0 has not reached wp: dist  5.29079479985
robot 0 has not reached wp: dist  5.29079479985
robot 0 has not reached wp: dist  5.24180000551
robot 0 has not reached wp: dist  4.99753289839
robot 0 has not reached wp: dist  4.75362627008
robot 0 has not reached wp: dist  4.50727697032
robot 0 has not reached wp: dist  4.25824589214
robot 0 has not reached wp: dist  4.01576787558
robot 0 has not reached wp: dist  3.77315421809
robot 0 has not reached wp: dist  3.52435841279
robot 0 has not reached wp: dist  3.27477239585
robot 0 has not reached wp: dist  3.02486624604
....
....


you can see that robot 0 is moving because the distance to the
waypoint is decreasing.

In the ARGoS GUI you should also be able to see the robot moving.

The XML contains the map between robot id and robot/target.  The field
'robotid' corresponds to the id of the footbot, in simulation, or in
real.  The robot and target id's (the id's that you use) are assigned
according to the order of appareance in the XML, starting from 0.


In the example, robot 0 corresponds to the footbot with id 1, and
target 0 to the footbot with id 2.  The ids of the footbots start from
1 in the simulation.

 