# GCC support can be specified at major, minor, or micro version
# (e.g. 8, 8.2 or 8.2.0).
# See https://hub.docker.com/r/library/gcc/ for all supported GCC
# tags from Docker Hub.
# See https://docs.docker.com/samples/library/gcc/ for more on how to use this image
FROM gcc:9.3.0

# Install raylib
RUN apt-get update && \
    apt-get install -y \
    build-essential \
    git \
    libasound2-dev \
    libgl1-mesa-dev \
    libglu1-mesa-dev \
    libx11-dev \
    libxi-dev \
    libxrandr-dev \
    mesa-common-dev \
    xorg-dev
WORKDIR /usr/src/raylib
RUN git clone --depth 1 --branch 4.0.0 https://github.com/raysan5/raylib.git .
WORKDIR /usr/src/raylib/src/
RUN make PLATFORM=PLATFORM_DESKTOP && \
    make install

# Copy project into image and set working location
COPY . /usr/src/there-is-no-bug
WORKDIR /usr/src/there-is-no-bug

# Compile
RUN make clean && \
    make && \
    cp ./build/game ./game

# Run
CMD ["./game"]
LABEL Name=thereisnobug Version=0.0.1
