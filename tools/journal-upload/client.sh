#!/bin/bash -

# Install client (debian/ubuntu machines) 
#
# Change URL to your server ip address

apt update
apt install systemd-journal-remote -y
mkdir -p /etc/systemd/journal-upload.conf.d/
cat    > /etc/systemd/journal-upload.conf.d/05-forward-logs-netap.conf <<EOF
[Upload]
URL=1.1.1.1:19532
EOF
systemctl enable systemd-journal-upload.service
systemctl start  systemd-journal-upload.service
