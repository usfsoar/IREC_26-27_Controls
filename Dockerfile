# Use Ubuntu 24.04 as the Linux operating system for our container.
FROM ubuntu:24.04

# Prevent Ubuntu from asking interactive questions during installation.
# This allows Docker to automatically install packages without waiting
# for the user to enter information.
ENV DEBIAN_FRONTEND=noninteractive

# Update Ubuntu's package list and install the tools we need.
RUN apt-get update && apt-get install -y \
    python3 \
    python3-pip \
    gcc \
    g++ \
    make \
    git \
    && rm -rf /var/lib/apt/lists/*

# python3-pip is used to install conan
# The final command removes the downloaded package lists to make
# the Docker image smaller.

# Install Conan using Python's package manager (pip).
RUN pip3 install conan --break-system-packages

# Check that the required programs were installed successfully.
RUN python3 --version && \
    gcc --version && \
    conan --version

# Set the default working directory inside the container.
WORKDIR /workspace

# Start a Bash terminal when the container is launched.
CMD ["/bin/bash"]