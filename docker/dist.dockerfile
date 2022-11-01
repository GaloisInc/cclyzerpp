# See doc/docker.rst.

ARG UBUNTU_NAME=jammy
ARG UBUNTU_VERSION=22.04
FROM ubuntu:$UBUNTU_VERSION as dist
# See the documentation for supported versions.
#
# See NOTE[Clang+LLVM] in ci.yml.
#
# TODO(#12, #113): Upgrade to Clang 15, LLVM 15.
ARG CLANG_VERSION=14
ARG LLVM_MAJOR_VERSION=14
#
# https://docs.docker.com/engine/reference/builder/#understand-how-arg-and-from-interact
ARG UBUNTU_NAME
ARG UBUNTU_VERSION
SHELL ["/bin/bash", "-c", "-o", "pipefail"]
ENV DEBIAN_FRONTEND=noninteractive

COPY ./build/cclyzer++*.deb /tmp/cclyzer++.deb
RUN apt-get update -qq && \
    apt install -y --no-install-recommends ca-certificates gnupg wget && \
    wget --no-verbose -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add - && \
    if [[ ${LLVM_MAJOR_VERSION} -lt 13 || ${LLVM_MAJOR_VERSION} -ge 16 ]]; then \
      echo "deb http://apt.llvm.org/${UBUNTU_NAME}/ llvm-toolchain-${UBUNTU_NAME} main" | tee /etc/apt/sources.list.d/llvm.list; \
    else \
      echo "deb http://apt.llvm.org/${UBUNTU_NAME}/ llvm-toolchain-${UBUNTU_NAME}-${LLVM_MAJOR_VERSION} main" | tee /etc/apt/sources.list.d/llvm.list; \
    fi && \
    apt-get update -qq && \
    apt install -y /tmp/cclyzer++.deb && \
    apt-get remove -y gnupg wget && \
    apt-get -y autoremove && \
    rm -rf /var/lib/apt/lists/* && \
    rm -f /tmp/cclyzer++.deb
