FROM ubuntu:22.04

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
    libcmocka-dev \
    xfce4 \
    xfce4-goodies \
    x11vnc \
    xvfb \
    curl \
    net-tools \
    x11-xserver-utils \
    libcurl4-openssl-dev \
    && rm -rf /var/lib/apt/lists/*

# Install Node.js
RUN curl -fsSL https://deb.nodesource.com/setup_18.x | bash - \
    && apt-get install -y nodejs

RUN apt-get remove -y xfce4-power-manager && \
    apt-get autoremove -y && \
    rm -rf /var/lib/apt/lists/*

# Download kubectl
RUN curl -LO "https://dl.k8s.io/release/$(curl -L -s https://dl.k8s.io/release/stable.txt)/bin/linux/amd64/kubectl" \
    && chmod +x ./kubectl \
    && mv ./kubectl /usr/local/bin/kubectl

# Install noVNC and websockify
RUN apt update && apt install -y git python3 python3-pip && \
    pip3 install websockify && \
    git clone https://github.com/novnc/noVNC.git /opt/noVNC

# Link the noVNC HTML files to a convenient path
RUN ln -s /opt/noVNC/vnc_lite.html /opt/noVNC/index.html

# Set the working directory
WORKDIR /app

# Copy your Node.js and C application files
COPY . /app

# Create a desktop entry for the autostart
RUN mkdir -p /root/.config/autostart && \
    echo '[Desktop Entry]\nType=Application\nName=StartBrogue\nExec=/app/brogue -G\nX-GNOME-Autostart-enabled=true' > /root/.config/autostart/startbrogue.desktop

# Disable xfwm4 compositing
RUN mkdir -p /root/.config/xfce4/xfconf/xfce-perchannel-xml && \
    echo '<?xml version="1.0" encoding="UTF-8"?><channel name="xfwm4" version="1.0"><property name="general" type="empty"><property name="use_compositing" type="bool" value="false"/></property></channel>' > /root/.config/xfce4/xfconf/xfce-perchannel-xml/xfwm4.xml

# Build Brogue application and metrics server
RUN make clean && make

# Expose the VNC, noVNC, and metrics HTTP ports
EXPOSE 38010 35910 16080 8090 8010 5910

# Set environment variables to center the Brogue window
ENV SDL_VIDEO_WINDOW_POS=0,0
ENV SDL_VIDEO_CENTERED=0
ENV SDL_VIDEO_FULLSCREEN_DISPLAY=1
ENV SDL_WINDOW_WIDTH=800
ENV SDL_WINDOW_HEIGHT=600

# Set up the X virtual framebuffer (Xvfb) in the background, start XFCE, x11vnc, Brogue, noVNC, and the metrics server
# Copy the shell script to the container
COPY start.sh /app/start.sh

# Make it executable
RUN chmod +x /app/start.sh

# Run the shell script
CMD ["/app/start.sh"]
