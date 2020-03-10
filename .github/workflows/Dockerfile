FROM ubuntu
ARG LIBFUSE_VERSION=fuse-3.9.0

# Installs LibFUSE so we dont end up with errors
# in coreos when trying to mount rclone.
# https://github.com/libfuse/libfuse
# https://github.com/libfuse/libfuse/releases
RUN apt-get update && apt-get upgrade -y && \
    apt-get -y install \
        build-essential \
        wget \
        meson \
        pkg-config \
        libudev-dev \
        udev

RUN wget -O "fuse.tar.xz" "https://github.com/libfuse/libfuse/releases/download/${LIBFUSE_VERSION}/${LIBFUSE_VERSION}.tar.xz" && \
    tar -xf fuse.tar.xz && \
    rm -f fuse.tar.xz && mv fuse* fuse

# Installing util/fusermount3 to /usr/local/bin/fusermount3
# Installing util/mount.fuse3 to /usr/local/sbin/mount.fuse3
RUN cd fuse && \
    mkdir build && cd build && \
    meson .. && ninja install

CMD ["/bin/bash"]


FROM ubuntu:18.04

# start from above 

RUN apt-get update
RUN apt-cache search gnutls
RUN apt-get install -y \
  libncurses5-dev \
  libreadline-dev \
  libfuse-dev \
  libjsoncpp-dev \
  nettle-dev \
  libgnutls28-dev \
  librestbed-dev \
  libjsoncpp-dev \
  libboost-dev \
  cython3 \
  python3-dev \
  python3-setuptools \
  build-essential \
  git \
  cmake \
  vim \
  pkg-config \
  autoconf \
  libpthread-stubs0-dev \
  wget

RUN wget https://github.com/aberaud/asio/archive/b2b7a1c166390459e1c169c8ae9ef3234b361e3f.tar.gz && \
    tar -xvf b2b7a1c166390459e1c169c8ae9ef3234b361e3f.tar.gz && cd asio-b2b7a1c166390459e1c169c8ae9ef3234b361e3f/asio && \
    ./autogen.sh && ./configure --prefix=/usr --without-boost --disable-examples --disable-tests && \
    make install 

RUN wget https://github.com/msgpack/msgpack-c/releases/download/cpp-3.2.1/msgpack-3.2.1.tar.gz && \
    tar -xzf msgpack-3.2.1.tar.gz && \
    cd msgpack-3.2.1 && mkdir build && cd build && \
    cmake -DMSGPACK_CXX11=ON -DMSGPACK_BUILD_EXAMPLES=OFF -DCMAKE_INSTALL_PREFIX=/usr .. && \
    make -j4 && \
    make install

RUN git clone https://github.com/savoirfairelinux/opendht.git && \
  cd opendht && \
  mkdir build && cd build && \
  cmake -DOPENDHT_PYTHON=ON -DCMAKE_INSTALL_PREFIX=/usr .. && \
  make -j4 && \
  make install 


