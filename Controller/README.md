# BeagleBone
Here you can find the software used to control the orthosis and some log results obtained during the experiments along with some Scripts to evaluate it.
This file [walki_software_user_manual.pdf ](https://github.com/pep248/MasterThesis_HipFlexionExtensionOrthosis/blob/main/BeagleBone/walki_software_user_manual.pdf)
contains instructions on how to upload controllers into the robot as long as the Software needed to do so.

## RemoteControl
Inside the folder [RemoteControl](https://github.com/pep248/MasterThesis_HipFlexionExtensionOrthosis/tree/main/BeagleBone/RemoteControl) you can find the program used to connect to the orthosis during the experiments, and send and receive data from it. The file [RemoteControl.exe](https://github.com/pep248/MasterThesis_HipFlexionExtensionOrthosis/blob/main/BeagleBone/RemoteControl/RemoteControl.exe) is the executable file used to turn it on.

## WalkiBBB
Inside the folder [WalkiBBB](https://github.com/pep248/MasterThesis_HipFlexionExtensionOrthosis/tree/main/BeagleBone/WalkiBBB) all the Software that has been uploaded into the Orthosis. The file [ewalkcontroller.cpp](https://github.com/pep248/MasterThesis_HipFlexionExtensionOrthosis/blob/main/BeagleBone/WalkiBBB/controllers/ewalk/ewalkcontroller.cpp) is the particular file where we uploaded our specific developed algorithms to actually control the orthosis as we intend.

## Logs
Inside the folder [logs](https://github.com/pep248/MasterThesis_HipFlexionExtensionOrthosis/tree/main/BeagleBone/logs) you can find the results obtained during the experiments as long as some useful MATLAB Scripts to evaluate all the data. The file [main_script.m](https://github.com/pep248/MasterThesis_HipFlexionExtensionOrthosis/blob/main/BeagleBone/logs/MATLAB%20Scripts/main_script.m) has all the needed comments and instructions to extract the data from the logs.
