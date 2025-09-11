FROM ubuntu:22.04

# Deps
RUN apt-get update && apt-get install -y \
    build-essential g++ cmake ninja-build make git curl ca-certificates \
    libssl-dev zlib1g-dev libpq-dev \
    nlohmann-json3-dev \
 && rm -rf /var/lib/apt/lists/*

# --- Boost 1.84+ (for http::message_generator and co_await) ---
WORKDIR /tmp
ENV BOOST_VERSION=1.84.0
ENV BOOST_DIR=boost_${BOOST_VERSION//./_}
RUN curl -L https://archives.boost.io/release/${BOOST_VERSION}/source/${BOOST_DIR}.tar.bz2 \
    -o ${BOOST_DIR}.tar.bz2 \
 && tar xf ${BOOST_DIR}.tar.bz2 \
 && cd ${BOOST_DIR} \
 && ./bootstrap.sh --with-libraries=system,filesystem,thread,url \
 && ./b2 -j$(nproc) cxxflags="-std=c++20" install \
 && ldconfig \
 && cd / && rm -rf /tmp/${BOOST_DIR} /tmp/${BOOST_DIR}.tar.bz2

# --- Build ---
WORKDIR /work
COPY CMakeLists.txt ./
COPY src ./src

RUN cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build

# Params (rewrites by .env/compose)
ENV APP_HOST=0.0.0.0
ENV APP_PORT=8080

EXPOSE 8080
CMD ["./build/app"]
