# See doc/docker.rst.

# TODO(#12): Upgrade to 22.04, LLVM 15
ARG BASE=ubuntu:20.04
FROM $BASE
SHELL ["/bin/bash", "-c", "-o", "pipefail"]
RUN mkdir -p /opt/cclyzer++/{bin,lib}
COPY ./build/factgen-exe /opt/cclyzer++/bin/
COPY ./build/libSoufflePA.so /opt/cclyzer++/lib/
COPY ./build/libPAPass.so /opt/cclyzer++/lib/
