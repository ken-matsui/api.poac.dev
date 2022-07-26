FROM ubuntu:22.10

ENV TZ=America/Los_Angeles
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN apt-get update && apt-get install -y \
    git \
    gcc \
    g++ \
    cmake \
    libjsoncpp-dev \
    uuid-dev \
    openssl \
    libssl-dev \
    zlib1g-dev \
    postgresql-all \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /builder
COPY . .

RUN cmake -B build -DCMAKE_BUILD_TYPE=Release \
 && cmake --build build

WORKDIR /app
RUN cp /builder/build/poac_api /app \
 && rm -rf /builder # libdrogon.a is statically linked

CMD ["/app/poac_api"]
