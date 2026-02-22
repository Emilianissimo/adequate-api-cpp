FROM ubuntu:24.04

# Docker compose V2
RUN apt-get update && apt-get install -y \
  ca-certificates curl gnupg lsb-release \
  && rm -rf /var/lib/apt/lists/*

RUN install -m 0755 -d /etc/apt/keyrings \
  && curl -fsSL https://download.docker.com/linux/ubuntu/gpg | gpg --dearmor -o /etc/apt/keyrings/docker.gpg \
  && chmod a+r /etc/apt/keyrings/docker.gpg

RUN echo \
  "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/docker.gpg] https://download.docker.com/linux/ubuntu \
  $(. /etc/os-release && echo $VERSION_CODENAME) stable" \
  > /etc/apt/sources.list.d/docker.list

RUN apt-get update && apt-get install -y \
  docker-ce-cli docker-compose-plugin \
  && rm -rf /var/lib/apt/lists/*

# App

RUN apt-get update && apt-get install -y \
    build-essential g++ cmake ninja-build make git curl ca-certificates \
    pkg-config \
    libssl-dev zlib1g-dev libpq-dev \
    nlohmann-json3-dev \
    libpoco-dev \
    libmagic-dev \
    libsodium-dev \
 && rm -rf /var/lib/apt/lists/*

# --- Boost 1.84+ ---
WORKDIR /tmp
ENV BOOST_VERSION=1.84.0
ENV BOOST_DIR=boost_${BOOST_VERSION//./_}
RUN curl -L https://archives.boost.io/release/${BOOST_VERSION}/source/${BOOST_DIR}.tar.bz2 \
    -o ${BOOST_DIR}.tar.bz2 \
 && tar xf ${BOOST_DIR}.tar.bz2 \
 && cd ${BOOST_DIR} \
 && ./bootstrap.sh --with-libraries=system,filesystem,thread,url \
 && ./b2 -j$(nproc) \
    variant=release \
    link=shared,static \
    threading=multi \
    address-model=64 \
    cxxflags="-std=c++20 -fPIC" \
    install \
 && ldconfig \
 && cd / && rm -rf /tmp/${BOOST_DIR} /tmp/${BOOST_DIR}.tar.bz2

WORKDIR /work
