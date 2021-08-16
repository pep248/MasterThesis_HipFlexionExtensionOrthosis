#!/bin/bash

echo "`date -u`: Starting WalkiBBB application" >> /root/WalkiSoftware/BeagleBone/logs/starts_log.txt

while true
do
    /root/WalkiSoftware/BeagleBone/WalkiBBB
    returnCode=$?
    
    if [ $returnCode != 0 ] ; then
        sleep 2
        echo "`date -u`: Restarting WalkiBBB application" >> /root/WalkiSoftware/BeagleBone/logs/starts_log.txt
    else
        break
    fi
done