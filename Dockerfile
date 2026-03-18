FROM nvcr.io/nvidia/deepstream:8.0-gc-triton-devel
RUN bash /opt/nvidia/deepstream/deepstream/user_additional_install.sh

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    wget \
    tar \
    pkg-config \
    libopencv-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . /app/
RUN chmod +x /app/build_tasks.sh && /app/build_tasks.sh
CMD ["/bin/bash"]
