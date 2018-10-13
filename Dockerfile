FROM ubuntu:16.04

RUN apt-get update && apt-get install -y \
    build-essential \
    libgflags-dev \
    libsnappy-dev \
    zlib1g-dev \
    libbz2-dev \
    liblz4-dev

COPY . /rocksdb/
RUN ["ls", "/rocksdb"]
WORKDIR /rocksdb
RUN ["make", "static_lib"]

