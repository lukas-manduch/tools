FROM debian:bookworm
RUN apt-get update \
  && apt-get install vim curl bash net-tools mc emacs netcat-openbsd nmap -y
CMD exec /bin/bash -c "trap : TERM INT; sleep infinity & wait"
