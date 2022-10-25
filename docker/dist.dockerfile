# See doc/docker.rst.

# TODO(#12): Upgrade to Ubuntu 22.04 (jammy), Clang 15, LLVM 15
ARG UBUNTU_NAME=focal
ARG UBUNTU_VERSION=20.04
FROM ubuntu:$UBUNTU_VERSION as dist
SHELL ["/bin/bash", "-c", "-o", "pipefail"]
COPY ./build/cclyzer++*.deb /tmp/cclyzer++.deb
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update -qq && \
    apt install -y /tmp/cclyzer++.deb && \
    rm -f /tmp/cclyzer++.deb
