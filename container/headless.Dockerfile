# buidls only headless version without webapp
FROM ubuntu AS compiler
COPY ./container/install_wxwidgets.sh ./
RUN ./install_wxwidgets.sh
RM install_wxwidgets.sh

# hack to create a headless x server, does not work when set in dockerfile?
RUN apt-get install xvfb -y
ENV DISPLAY=:1.0

#compile molovol

#FROM compiler AS builder
RUN apt update; apt install pip -y
RUN apt purge --auto-remove cmake -y; pip install cmake --upgrade
WORKDIR /
COPY src/ src/
COPY include/ include/
COPY CMakeLists.txt /
COPY inputfile/ inputfile/
RUN mkdir cmake
WORKDIR cmake
RUN cmake .. -DCMAKE_BUILD_TYPE=RELEASE
RUN make
#launch.sh is expecting that molovol is residing in bin
RUN mkdir /build/ && mv MoloVol /bin/ && mv inputfile /bin/

WORKDIR /
COPY launch_headless.sh launch.sh
RUN chmod +x launch.sh
ENTRYPOINT ["./launch.sh"]
CMD ["-r", "1.2", "-g", "0.2", "-fs", "/inputfile/isobutane.xyz", "-q", "-o", "time,vol"]