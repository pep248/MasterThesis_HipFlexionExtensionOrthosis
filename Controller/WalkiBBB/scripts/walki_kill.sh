#!/bin/bash
killall -SIGTERM walki_keep_running.sh
killall -SIGKILL walki_keep_running.sh
killall -SIGTERM WalkiBBB
killall -SIGKILL WalkiBBB