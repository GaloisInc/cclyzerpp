# Image with all cclyzer++ development tools, i.e., everything needed by
# cclyzer++ developers to build and test cclyzer++.

# TODO(#12): Upgrade to 22.04, LLVM 15
ARG BASE=ubuntu:20.04
FROM $BASE as dev
ARG LLVM_VERSION=10
SHELL ["/bin/bash", "-c", "-o", "pipefail"]
ENV DEBIAN_FRONTEND=noninteractive

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
      wget && \
    wget --no-verbose https://souffle-lang.github.io/ppa/souffle-key.public -O /usr/share/keyrings/souffle-archive-keyring.gpg && \
    echo "deb [signed-by=/usr/share/keyrings/souffle-archive-keyring.gpg] https://souffle-lang.github.io/ppa/ubuntu/ stable main" | tee /etc/apt/sources.list.d/souffle.list && \
    apt-get update && \
    apt-get --yes install --no-install-recommends souffle && \
    wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add - && \
    echo "deb http://apt.llvm.org/jammy/ llvm-toolchain-jammy main" | tee /etc/apt/sources.list.d/llvm.list && \
    apt-get update && \
    apt-get --yes install --no-install-recommends \
      clang-${LLVM_VERSION} \
      clang-format-${LLVM_VERSION} \
      clang-tidy-${LLVM_VERSION} \
      llvm-${LLVM_VERSION} \
      llvm-${LLVM_VERSION}-dev && \
    rm -rf /var/lib/apt/lists/*
# TODO(lb): Test dependencies

# Image that builds cclyzer++
FROM dev as build
SHELL ["/bin/bash", "-c", "-o", "pipefail"]
WORKDIR /build
COPY . .
RUN cmake -G Ninja -B build -S . && \
    cmake --build build -j $(nproc)

# Image with runtime dependencies and a built version of cclyzer++
#
# TODO(lb): Build and install other binaries
FROM dev as dist
SHELL ["/bin/bash", "-c", "-o", "pipefail"]
COPY --from=build /build/build/factgen-exe /usr/bin/factgen-exe
