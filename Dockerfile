FROM ubuntu:18.04

RUN apt-get update && apt-get install -y gcc && apt-get install -y make && apt-get install -y git && apt-get install -y binutils && apt-get install -y libc6-dev
COPY . /9cc
RUN cd 9cc && make compile && chmod a+x test.sh
WORKDIR /9cc