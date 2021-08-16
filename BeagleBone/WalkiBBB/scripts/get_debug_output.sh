#!/bin/bash
tail -f -n +1 `ls /root/WalkiSoftware/BeagleBone/logs/log_*_info.txt | sort -V | tail -n 1`