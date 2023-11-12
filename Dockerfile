FROM gcc:latest
# Set the working directory inside the container
WORKDIR /usr/src/app
# Copy the local code to the container
COPY server /usr/src/app
# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    libpthread-stubs0-dev \
    # other dependencies
    && rm -rf /var/lib/apt/lists/*
# compile the server
RUN gcc main.c -lpthread -o server
# Run the server after exposing the port
EXPOSE 8080
CMD ["./server"]