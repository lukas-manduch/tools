#!/bin/bash -

# Install server (debian/ubuntu machines) 
#

apt update
apt install systemd-journal-remote -y
mkdir -p /etc/systemd/system/systemd-journal-remote.service.d/
cat >  /etc/systemd/system/systemd-journal-remote.service.d/override.conf <<EOF
[Service]
ExecStart=
ExecStart=/usr/lib/systemd/systemd-journal-remote --listen-http=-3 --output=/var/log/journal/remote/
EOF
systemctl enable systemd-journal-remote.socket
systemctl start  systemd-journal-remote.socket
