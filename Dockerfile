FROM ubuntu:22.04

WORKDIR /app

ARG DEBIAN_FRONTEND=noninteractive
ARG ZEPHYR_TOOLCHAIN_VERSION=1.0.1
ARG AXON_SDK_NAME=sdk-axons-ml-lib-AXONS-ML-RELEASE-2P0P1
ENV AXON_PATH=/opt/sdk-axon-ml-lib

# APT packages (Python 3.12 comes from the deadsnakes PPA)
RUN apt update && apt install -y --no-install-recommends \
        ca-certificates curl gnupg software-properties-common && \
    add-apt-repository -y ppa:deadsnakes/ppa && \
    apt update && apt install -y --no-install-recommends \
        git ninja-build gperf ccache dfu-util device-tree-compiler wget unzip xz-utils file \
        make gcc g++ cmake libsdl2-dev \
        python3.12 python3.12-dev python3.12-venv && \
    rm -rf /var/lib/apt/lists/*

# Python virtual environment (holds west and the Zephyr build requirements)
RUN python3.12 -m venv /opt/venv
ENV PATH="/opt/venv/bin:${PATH}"
RUN pip install --upgrade pip && \
    pip install west==1.4.0 ecdsa==0.17.0

# Zephyr SDK
RUN if [ $(uname -m) = "x86_64" ]; then export ARCH=x86_64; else export ARCH=aarch64; fi && \
    cd /opt && \
    wget https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v${ZEPHYR_TOOLCHAIN_VERSION}/zephyr-sdk-${ZEPHYR_TOOLCHAIN_VERSION}_linux-${ARCH}_minimal.tar.xz && \
    tar xf zephyr-sdk-${ZEPHYR_TOOLCHAIN_VERSION}_linux-${ARCH}_minimal.tar.xz && \
    cd zephyr-sdk-${ZEPHYR_TOOLCHAIN_VERSION} && \
    ./setup.sh -t arm-zephyr-eabi -h -c && \
    rm -rf /opt/zephyr-sdk-${ZEPHYR_TOOLCHAIN_VERSION}_linux-${ARCH}_minimal.tar.xz

ENV ZEPHYR_TOOLCHAIN_VARIANT=zephyr
ENV ZEPHYR_SDK_INSTALL_DIR=/opt/zephyr-sdk-${ZEPHYR_TOOLCHAIN_VERSION}

# Nordic Axon ML library (its west.yml pins the matching nRF Connect SDK)
RUN mkdir ${AXON_PATH} && \
    wget https://cdn.edgeimpulse.com/build-system/${AXON_SDK_NAME}.zip && \
    unzip ${AXON_SDK_NAME}.zip -d /opt/ && rm ${AXON_SDK_NAME}.zip && \
    mv /opt/sdk-axons-ml-lib-*/* ${AXON_PATH}/ && \
    rm -rf /opt/sdk-axons-ml-lib-*

# Install the nRF Connect SDK (west workspace is rooted at /opt)
RUN cd ${AXON_PATH} && \
    west init -l ./ && \
    cd /opt && \
    west update && \
    west zephyr-export && \
    sed -i "/spsdk >= 3.0.0/d" /opt/zephyr/scripts/requirements-extras.txt && \
    sed -i "/lpc_checksum/d" /opt/zephyr/scripts/requirements-extras.txt && \
    sed -i "s/semver/semver>=2.1.2/g" /opt/zephyr/scripts/requirements-base.txt && \
    pip install --upgrade license-expression && \
    pip install -r /opt/zephyr/scripts/requirements.txt

ENV ZEPHYR_BASE="/opt/zephyr"
