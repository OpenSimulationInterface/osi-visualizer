#!/bin/sh
test -d osi_pb || mkdir osi_pb
cat > osi_pb/README <<EOF 
This folder will be mounted as a volume in the docker image.
-Add pb files to visualize here.
EOF

docker run --rm --runtime=nvidia -v /tmp/.X11-unix:/tmp/.X11-unix \
           -v `pwd`/osi_pb:/osi_pb -e DISPLAY=$DISPLAY $* \
           osi-standard/osi-visualizer-nvidia
