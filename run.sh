#!/usr/bin/env bash
IMAGE="deepstream-tasks:8.0"
xhost +local:docker
mkdir -p "$PWD/output"

docker run --gpus all -it --rm \
  --network host \
  -w /app \
  -v "$PWD/output":/app/output \
  -e DISPLAY=$DISPLAY \
  -v /tmp/.X11-unix:/tmp/.X11-unix \
  -e PULSE_SERVER=unix:${XDG_RUNTIME_DIR}/pulse/native \
  -v ${XDG_RUNTIME_DIR}/pulse/native:${XDG_RUNTIME_DIR}/pulse/native \
  -v ~/.config/pulse/cookie:/root/.config/pulse/cookie \
  --group-add $(getent group audio | cut -d: -f3) \
  $IMAGE
