FROM emscripten/emsdk:latest AS builder

# Install make and update CMake
RUN apt-get update && apt-get install -y make wget software-properties-common && \
    wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | tee /etc/apt/keyrings/kitware.gpg >/dev/null && \
    echo 'deb [signed-by=/etc/apt/keyrings/kitware.gpg] https://apt.kitware.com/ubuntu/ jammy main' | tee /etc/apt/sources.list.d/kitware.list >/dev/null && \
    apt-get update && apt-get install -y cmake

# Clone and build raylib for web
RUN git clone https://github.com/raysan5/raylib.git /raylib && \
    cd /raylib && \
    mkdir -p build && \
    cd build && \
    emcmake cmake .. -DPLATFORM=Web -DCMAKE_BUILD_TYPE=Release -DBUILD_EXAMPLES=OFF -DBUILD_GAMES=OFF -DBUILD_SHARED_LIBS=OFF && \
    emmake make -j$(nproc) && \
    make install

# Set environment variables
ENV RAYLIB_INCLUDE_PATH=/emsdk/upstream/emscripten/cache/sysroot/include
ENV RAYLIB_LIB_PATH=/emsdk/upstream/emscripten/cache/sysroot/lib

# Copy project source
COPY . /app
WORKDIR /app

# Build the web target
RUN make web-build

# Second stage: serve with nginx
FROM nginx:alpine

# Copy built web files to nginx html directory
COPY --from=builder /app/result/ /usr/share/nginx/html/

# Expose port 80
EXPOSE 80

# Start nginx
CMD ["nginx", "-g", "daemon off;"]