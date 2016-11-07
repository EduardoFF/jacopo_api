IMAGE=$1
CMD="${@:2}"
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
docker run -it  --rm \
       -w /RoboNetSim/argos/scripts/ \
       -e DISPLAY=$DISPLAY \
       --env="QT_X11_NO_MITSHM=1" \
       --volume="/tmp/.X11-unix:/tmp/.X11-unix:rw" \
       --net="host" \
       $IMAGE \
       $CMD 

#       --device /dev/ttyUSB0 \
#       -t -i \
 #       --privileged -v /dev/bus/usb:/dev/bus/usb \
