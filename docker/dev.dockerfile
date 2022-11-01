# See doc/docker.rst.

# Image with all cclyzer++ development tools, i.e., everything needed by
# cclyzer++ developers to build and test cclyzer++.

ARG UBUNTU_NAME=jammy
ARG UBUNTU_VERSION=22.04
FROM ubuntu:$UBUNTU_VERSION as dev
# See the documentation for supported versions.
#
# See NOTE[Clang+LLVM] in ci.yml.
#
# TODO(#12, #113): Upgrade to Clang 15, LLVM 15.
ARG CLANG_VERSION=14
ARG LLVM_MAJOR_VERSION=14
# https://docs.docker.com/engine/reference/builder/#understand-how-arg-and-from-interact
ARG UBUNTU_NAME
ARG UBUNTU_VERSION
SHELL ["/bin/bash", "-c", "-o", "pipefail"]
ENV DEBIAN_FRONTEND=noninteractive

# cmake: Build system
# git: Later steps of Dockerfile
# gnupg: Later steps of Dockerfile
# ninja-build: Build system
# python3.10: Tests
# python3-pip: Later steps of Dockerfile
# libboost-system-dev: Library dependency of cclyzer++
# libboost-filesystem-dev: Library dependency of cclyzer++
# libboost-iostreams-dev: Library dependency of cclyzer++
# libboost-program-options-dev: Library dependency of cclyzer++
# ruby: fpm, for deb packaging
# wget: Later steps of Dockerfile
RUN apt-get update && \
    apt-get --yes install --no-install-recommends \
      cmake \
      git \
      gnupg \
      ninja-build \
      python3.10 \
      python3-pip \
      libboost-system-dev \
      libboost-filesystem-dev \
      libboost-iostreams-dev \
      libboost-program-options-dev \
      ruby \
      wget && \
    wget --no-verbose https://souffle-lang.github.io/ppa/souffle-key.public -O /usr/share/keyrings/souffle-archive-keyring.gpg && \
    echo "deb [signed-by=/usr/share/keyrings/souffle-archive-keyring.gpg] https://souffle-lang.github.io/ppa/ubuntu/ stable main" | tee /etc/apt/sources.list.d/souffle.list && \
    apt-get update && \
    apt-get --yes install --no-install-recommends souffle
RUN wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add - && \
    if [[ ${LLVM_MAJOR_VERSION} -lt 13 || ${LLVM_MAJOR_VERSION} -ge 16 ]]; then \
      echo "deb http://apt.llvm.org/${UBUNTU_NAME}/ llvm-toolchain-${UBUNTU_NAME} main" | tee /etc/apt/sources.list.d/llvm.list; \
    else \
      echo "deb http://apt.llvm.org/${UBUNTU_NAME}/ llvm-toolchain-${UBUNTU_NAME}-${LLVM_MAJOR_VERSION} main" | tee /etc/apt/sources.list.d/llvm.list; \
    fi && \
    apt-get update && \
    apt-get --yes install --no-install-recommends \
      clang-${CLANG_VERSION} \
      clang-format-${CLANG_VERSION} \
      clang-tidy-${CLANG_VERSION} \
      libomp-${CLANG_VERSION}-dev \
      llvm-${LLVM_MAJOR_VERSION} \
      llvm-${LLVM_MAJOR_VERSION}-dev && \
    update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang++-${CLANG_VERSION} 60 && \
    update-alternatives --install /usr/bin/cc cc /usr/bin/clang-${CLANG_VERSION} 60 && \
    update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-${CLANG_VERSION} 60 && \
    update-alternatives --install /usr/bin/clang clang /usr/bin/clang-${CLANG_VERSION} 60 && \
    update-alternatives --install /usr/bin/opt opt /usr/bin/opt-${LLVM_MAJOR_VERSION} 60 && \
    rm -rf /var/lib/apt/lists/*
RUN gem install fpm -v 1.14.2
RUN pip install \ mypy==0.982 \
      pytest==7.1.3 \
      pytest-xdist==2.5.0
# TODO(lb): Sphinx
