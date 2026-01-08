# Dockerfile
FROM alpine:3.20

# Install build tools, doxygen, graphviz, git, and LaTeX for PDF generation
RUN apk add --no-cache \
    build-base \
    gcc \
    make \
    git \
    doxygen \
    graphviz \
    texlive \
    texlive-dvi \
    texmf-dist-latexextra \
    texmf-dist-fontsextra \
    perl

WORKDIR /workspace

CMD ["/bin/sh"]
