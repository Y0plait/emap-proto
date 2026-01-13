# Dockerfile
FROM alpine:3.20

# Install build tools, doxygen, graphviz, git
RUN apk add --no-cache \
    build-base \
    gcc \
    make \
    git \
    doxygen \
    graphviz

WORKDIR /workspace

