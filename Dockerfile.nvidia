FROM nvidia/opengl:1.0-glvnd-devel

# protobuf and osi-visualizer dependencies
RUN apt-get update && apt-get install -y \
    autoconf \
    automake \
    cmake \
    curl \
    doxygen \
    g++ \
    gcc \
    git \
    libtool \
    libprotobuf-dev \
    libzmq3-dev \
    make \
    protobuf-compiler \
    qtbase5-dev \
    unzip \
    vim \
    wget \
    zip

# install FMI Library
RUN mkdir -p /build
WORKDIR /build
ENV FMI_VERSION="2.0.3"
RUN wget https://jmodelica.org/fmil/FMILibrary-${FMI_VERSION}-src.zip \
    && unzip FMILibrary-${FMI_VERSION}-src.zip \
    && mkdir build-fmil \
    && cd build-fmil \
    && cmake -DFMILIB_INSTALL_PREFIX=/usr/local ../FMILibrary-${FMI_VERSION} \
    && make install

# build open-simulation interface and osi-visualizer
WORKDIR /opt/osi-visualizer

COPY ./ /opt/osi-visualizer/

RUN mkdir build \
    && cd build \
    && cmake .. \
    && make \
    && cp osi-visualizer ../osi-visualizer

# add osi-visualizer to path
ENV PATH="/opt/osi-visualizer:$PATH"

# set workdir so that default command works
WORKDIR /opt/osi-visualizer
CMD /opt/osi-visualizer/osi-visualizer
