FROM ubuntu:18.04

RUN apt-get update \
    && apt-get install -y \
        cmake \
        g++ \
        gcc \
    && rm -rf /var/lib/apt/lists/*

# Add the spatial wrapper script to the PATH.
ENV PATH "$PATH:/build/project/ci"

COPY ./ ./build/project
WORKDIR /build/project
