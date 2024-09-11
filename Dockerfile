FROM ubuntu:latest as cmr_cache

USER root

RUN apt update -y && \
    apt upgrade -y && \
    apt install -y libboost-all-dev clang && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /usr/cmr-cache/

COPY . /tmp/build-cmr-cache/

RUN cd /tmp/build-cmr-cache/ && \
    clang++ -o cmr_cache main.cpp -I./vendor/yaml -I/usr/include/boost -L/usr/lib/x86_64-linux-gnu -lboost_system -lpthread -std=c++17 && \
    cp cmr_cache /usr/cmr-cache/ && \
    cp -r config/ /usr/cmr-cache/ && \
    rm -rf /tmp/build-cmr-cache/

ENTRYPOINT [ "./cmr_cache" ]
