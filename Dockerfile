FROM alpine:3.22.2
RUN apk add --no-cache gcc make
COPY . /opt/project
WORKDIR /opt/project
