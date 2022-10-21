# See doc/docker.rst.

# TODO(#12): Upgrade to 22.04, LLVM 15
ARG BASE=ubuntu:20.04
FROM $BASE
SHELL ["/bin/bash", "-c", "-o", "pipefail"]
COPY ./build/cclyzer++*.deb /tmp/cclyzer++.deb
RUN dpkg -i /tmp/cclyzer++.deb && \
    rm -f /tmp/cclyzer++.deb
