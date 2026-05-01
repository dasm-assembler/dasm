FROM ubuntu:16.04

# Install packages
RUN apt update && apt upgrade -y &&\
    apt install -y curl &&\
    apt install -y make &&\
    apt install -y unzip &&\
    apt install -y git &&\
    apt install -y gcc &&\
    apt install -y gcc-multilib &&\
    apt install -y mingw-w64

#download darwin build packages
RUN curl -O http://security.ubuntu.com/ubuntu/pool/universe/o/openssl098/libssl0.9.8_0.9.8o-7ubuntu3.2.14.04.1_amd64.deb &&\
    curl -O http://ppa.launchpad.net/flosoft/cross-apple/ubuntu/pool/main/a/apple-uni-sdk-10.5/apple-uni-sdk-10.5_20110407-0.flosoft1_amd64.deb &&\
    curl -O http://ppa.launchpad.net/flosoft/cross-apple/ubuntu/pool/main/a/apple-x86-odcctools/apple-x86-odcctools_758.159-0flosoft11_amd64.deb &&\
    curl -O http://ppa.launchpad.net/flosoft/cross-apple/ubuntu/pool/main/a/apple-x86-gcc/apple-x86-gcc_4.2.1~5646.1flosoft2_amd64.deb &&\
    export DEBIAN_FRONTEND=noninteractive &&\
    dpkg -i libssl0.9.8_0.9.8o-7ubuntu3.2.14.04.1_amd64.deb &&\
    dpkg -i apple-uni-sdk-10.5_20110407-0.flosoft1_amd64.deb &&\
    dpkg -i apple-x86-odcctools_758.159-0flosoft11_amd64.deb &&\
    dpkg -i apple-x86-gcc_4.2.1~5646.1flosoft2_amd64.deb &&\
    rm *.deb

CMD /bin/bash
