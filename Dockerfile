# ---- Stage 1: Build ----
    FROM alpine:latest AS build

    WORKDIR /app
    
    # Install build dependencies
    RUN apk add \
        g++ cmake make libpq-dev git linux-headers
    
    COPY . /app
    
    RUN mkdir build && cd build && cmake .. && make
    