FROM robonetsim/argos
MAINTAINER Eduardo Feo Flushing <eduardo@idsia.ch>
LABEL Description="Simulation of multi-robot scenario using ARGoS" Vendor="IDSIA" Version="1.0"


RUN apt-get update && apt-get install -y \
autoconf \
libtool \
libglib2.0-dev \
pkg-config &&  rm -rf /var/lib/apt/lists/*

RUN set -x \
&& git clone --branch v1.3.1 https://github.com/lcm-proj/lcm.git \
&& ( cd lcm && git checkout ) \
&& cd lcm && ./bootstrap.sh \
&& ./configure --prefix=/usr/ && make install \
&& cd ../ && rm -rf lcm \
&& echo "/usr/lib" >> /etc/ld.so.conf.d/lib.conf \
&& ldconfig



COPY argos /RoboNetSim/argos

RUN cd /RoboNetSim/argos \
&& mkdir -p build && cd build && cmake -DCMAKE_BUILD_TYPE=Release ../ \
&& make

