FROM drogonframework/drogon:latest

# добавим инструменты
RUN apt-get update && apt-get install -y \
    cmake ninja-build g++ make libpq-dev libhiredis-dev libjsoncpp-dev \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /work

CMD ["/bin/bash"]
