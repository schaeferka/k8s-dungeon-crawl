# Start from an amd64 Ubuntu base image
FROM --platform=linux/amd64 ubuntu:22.04

# Avoid prompts from apt-get asking for input during build
ARG DEBIAN_FRONTEND=noninteractive

# Install C build tools, VNC server, and xfce
RUN apt-get update && apt-get install -y \
    build-essential \
    libjansson-dev \
    libmicrohttpd-dev \
    libncurses5-dev \
    libsdl2-dev \
    libsdl2-image-dev \
    libpng-dev \
    libjpeg-dev \
    libtiff-dev \
    libwebp-dev \
    xfce4 \
    xfce4-goodies \
    x11vnc \
    xvfb \
    curl \
    net-tools \
    x11-xserver-utils \
    libcurl4-openssl-dev \
    git \
    python3 \
    python3-pip \
    && rm -rf /var/lib/apt/lists/*

# Install Node.js
RUN curl -fsSL https://deb.nodesource.com/setup_18.x | bash - \
    && apt-get install -y nodejs

# Remove unnecessary components and clean up
RUN apt-get remove -y xfce4-power-manager && \
    apt-get autoremove -y && \
    rm -rf /var/lib/apt/lists/*

# Download kubectl
RUN curl -LO "https://dl.k8s.io/release/$(curl -L -s https://dl.k8s.io/release/stable.txt)/bin/linux/amd64/kubectl" \
    && chmod +x ./kubectl \
    && mv ./kubectl /usr/local/bin/kubectl

# Install noVNC and websockify
RUN pip3 install websockify && \
    git clone https://github.com/novnc/noVNC.git /opt/noVNC

# Link the noVNC HTML files to a convenient path
RUN ln -s /opt/noVNC/vnc_lite.html /opt/noVNC/index.html

# Set the working directory
WORKDIR /app

# Copy the entire project, including Makefile and source files
COPY . /app

# Compile the Brogue game and metrics server inside the container
RUN make

# Configure VNC
ARG VNC_PASSWORD=password
RUN mkdir ~/.vnc && \
    x11vnc -storepasswd $VNC_PASSWORD ~/.vnc/passwd

# Expose ports for VNC, noVNC, and metrics
EXPOSE 15900 16080 18000

# Set environment variables for SDL
ENV SDL_VIDEO_WINDOW_POS=center
ENV SDL_VIDEO_CENTERED=1
ENV SDL_VIDEO_FULLSCREEN_DISPLAY=1

# Start all necessary services
CMD bash -c "\
    Xvfb :1 -screen 0 1280x960x24 & \
    echo 'Starting Xvfb' && \
    sleep 2 && \
    DISPLAY=:1 xfce4-session & \
    echo 'Starting xfce4-session' && \
    export SDL_VIDEODRIVER=dummy; \
    DISPLAY=:1 xset r on & \
    DISPLAY=:1 x11vnc -display :1 -usepw -forever -o /var/log/x11vnc.log & \
    echo 'Starting x11vnc' && \
    /opt/noVNC/utils/novnc_proxy --vnc localhost:5900 --listen 16080 & \
    echo 'Starting noVNC' && \
    DISPLAY=:1 /app/brogue -G & \
    echo 'Starting Brogue' && \
    wait"
