FROM ubuntu:22.04

ARG DEBIAN_FRONTEND=noninteractive

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
    git \
    python3 \
    python3-pip \
    && rm -rf /var/lib/apt/lists/*

RUN curl -fsSL https://deb.nodesource.com/setup_18.x | bash - \
    && apt-get install -y nodejs

RUN apt-get remove -y xfce4-power-manager && \
    apt-get autoremove -y && \
    rm -rf /var/lib/apt/lists/*

RUN curl -LO "https://dl.k8s.io/release/$(curl -L -s https://dl.k8s.io/release/stable.txt)/bin/linux/amd64/kubectl" \
    && chmod +x ./kubectl \
    && mv ./kubectl /usr/local/bin/kubectl

RUN apt update && apt install -y git python3 python3-pip && \
    pip3 install websockify && \
    git clone https://github.com/novnc/noVNC.git /opt/noVNC

RUN ln -s /opt/noVNC/vnc_lite.html /opt/noVNC/index.html

# Install Mongoose library
RUN curl -LO https://github.com/cesanta/mongoose/archive/refs/tags/7.16.tar.gz \
    && tar -xzf 7.16.tar.gz \
    && mkdir -p /app/src/third_party/mongoose \
    && mv mongoose-7.16/mongoose.c /app/src/third_party/mongoose/mongoose.c \
    && mv mongoose-7.16/mongoose.h /app/src/third_party/mongoose/mongoose.h \
    && rm -rf mongoose-7.16 7.16.tar.gz

WORKDIR /app

COPY . /app

# Create a desktop entry for the autostart
RUN mkdir -p /root/.config/autostart && \
    echo '[Desktop Entry]\nType=Application\nName=StartBrogue\nExec=/app/brogue -G\nX-GNOME-Autostart-enabled=true' > /root/.config/autostart/startbrogue.desktop

# Disable xfwm4 compositing
RUN mkdir -p /root/.config/xfce4/xfconf/xfce-perchannel-xml && \
    echo '<?xml version="1.0" encoding="UTF-8"?><channel name="xfwm4" version="1.0"><property name="general" type="empty"><property name="use_compositing" type="bool" value="false"/></property></channel>' > /root/.config/xfce4/xfconf/xfce-perchannel-xml/xfwm4.xml

RUN make clean && make

# Expose the VNC, noVNC, and HTTP ports
EXPOSE 38010 35910 16080 8000

# Set environment variables to center the Brogue window
ENV SDL_VIDEO_WINDOW_POS=0,0
ENV SDL_VIDEO_CENTERED=0
ENV SDL_VIDEO_FULLSCREEN_DISPLAY=1
ENV SDL_WINDOW_WIDTH=800
ENV SDL_WINDOW_HEIGHT=600

COPY start.sh /app/start.sh

RUN chmod +x /app/start.sh

CMD ["/app/start.sh"]
