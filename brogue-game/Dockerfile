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
RUN make

# Confirm that metrics_server was created and move it to /app if needed
#RUN if [ -f "bin/metrics_server" ]; then cp bin/metrics_server /app/metrics_server; fi

# Set up VNC
ARG VNC_PASSWORD=password
RUN mkdir ~/.vnc \
    && x11vnc -storepasswd $VNC_PASSWORD ~/.vnc/passwd

# Expose the VNC, noVNC, and metrics HTTP ports
EXPOSE 15900 16080 18000

# Set environment variables to center the Brogue window
ENV SDL_VIDEO_WINDOW_POS=center
ENV SDL_VIDEO_CENTERED=1
ENV SDL_VIDEO_FULLSCREEN_DISPLAY=1

# Set up the X virtual framebuffer (Xvfb) in the background, start XFCE, x11vnc, Brogue, noVNC, and the metrics server
CMD bash -c "Xvfb :1 -screen 0 1280x960x24 & \
             while ! xset -display :1 q; do sleep 1; done; \
             DISPLAY=:1 xfce4-session & \
             export SDL_VIDEODRIVER=dummy; \
             DISPLAY=:1 xset r on & \
             DISPLAY=:1 x11vnc -display :1 -usepw -forever -o /var/log/x11vnc.log & \
             /opt/noVNC/utils/novnc_proxy --vnc localhost:5900 --listen 16080 & \
             DISPLAY=:1 /app/brogue -G & \
             wait"