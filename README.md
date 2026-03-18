# DeepStream 8.0 Dockerized Tasks

This package contains Dockerized versions of DeepStream Application. The setup compiles and runs the applications within the DeepStream 8.0 Development container (`nvcr.io/nvidia/deepstream:8.0-gc-triton-devel`).

## Requirements
1. Docker installed on the host machine.
2. NVIDIA GPU && NVIDIA Container Toolkit installed.
3. (Optional but recommended) Docker Compose.

## How to Build and Run

### Using Docker Compose (Recommended)

To build the Docker image and launch the container with GPU support and X11 forwarding enabled (so you can view visual outputs like video screens if needed), run:

```bash
docker compose build
```
**Run the container**:
```bash
./run.sh
```
## Running the Applications

- **Task 1 DS (DeepStream App)**
  ```bash
  cd build
  ./deepstream_gst_app
  ```
Note: If an application requires command-line arguments (like passing an input video file), append them to the execution command.
