#!/bin/bash

Xvfb :1 -screen 0 1280x960x24 &
while ! xset -display :1 q; do sleep 1; done;
DISPLAY=:1 xfce4-session &
export SDL_VIDEODRIVER=dummy;
DISPLAY=:1 xset r on &
DISPLAY=:1 x11vnc -display :1 -nopw -forever -o /var/log/x11vnc.log &
/opt/noVNC/utils/novnc_proxy --vnc localhost:5900 --listen 16080 &
DISPLAY=:1 /app/brogue -G &
wait