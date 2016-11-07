SIMLEN=$1
NROBOTS=$2
SEED=$3
cp ../xml/sim_argos_gui.xml scene.xml
sed -i 's/PAR_SIMLEN/'"${SIMLEN}"'/' scene.xml
sed -i 's/PAR_NROBOTS/'"${NROBOTS}"'/' scene.xml
sed -i 's/PAR_RANDOMSEED/'"${SEED}"'/' scene.xml
scenario_xml=$( readlink -f scene.xml)
cd ../build
launch_argos -c \
  ${scenario_xml} 2>&1 | tee ~/tmp/argos.out
