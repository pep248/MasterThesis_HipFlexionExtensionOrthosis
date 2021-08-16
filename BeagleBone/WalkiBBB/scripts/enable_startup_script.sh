#!/bin/bash
chmod +x /root/WalkiSoftware/BeagleBone/scripts/walki_keep_running.sh
cp /root/WalkiSoftware/BeagleBone/scripts/walki.service /lib/systemd/
rm -f /etc/systemd/system/walki.service
ln /lib/systemd/walki.service /etc/systemd/system/

systemctl daemon-reload
systemctl enable walki.service
