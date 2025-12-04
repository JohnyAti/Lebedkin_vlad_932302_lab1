FROM alpine:latest
RUN apk add --no-cache bash util-linux
WORKDIR /app
COPY container_internals.sh .
RUN chmod +x container_internals.sh
VOLUME /shared_volume
CMD ["/bin/sh","container_internals.sh"]
