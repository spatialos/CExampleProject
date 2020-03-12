FROM ubuntu:18.04

RUN ["apt-get", "update"]
RUN ["apt-get", "install", "-y", "gcc"]
RUN ["apt-get", "install", "-y", "g++"]
RUN ["apt-get", "install", "-y", "cmake"]

WORKDIR /build/
COPY ./ ./project

VOLUME /build/tools
ENV TOOLS_DIR "/build/tools"

VOLUME /build/auth
ENV AUTH_DIR "/build/auth"

# Add the spatial wrapper script to the PATH.
ENV PATH "$PATH:/build/project/ci"

WORKDIR /build/project
