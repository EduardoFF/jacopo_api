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


after you are finished using the simulator, return access control that were disabled

```
xhost -local:root
```


 