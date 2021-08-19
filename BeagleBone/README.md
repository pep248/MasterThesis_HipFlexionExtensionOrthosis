# BeagleBone
Here you can find the software used to control the orthosis, some log results obtained during the experiments and a small pdf with instructions on how to upload it into the robot.

## Logs
Inside the folder [logs](https://github.com/pep248/MasterThesis_HipFlexionExtensionOrthosis/tree/main/BeagleBone/logs) you can find the results obtained during the experiments as long as some usefull MATLAB Scripts to evaluate all the data. The file [main_script.m](https://github.com/pep248/MasterThesis_HipFlexionExtensionOrthosis/blob/main/BeagleBone/logs/MATLAB%20Scripts/main_script.m) has all the needed comments and instructions to extract the data from the logs.

This repo assumes the user is on a Windows system and has Scone Software installed on their machine.

To install SCONE Software follow the instructions [here](https://scone.software/doku.php).

In order to evaluate the provided simulations, we have to properly define the SCONE Software working directory.
This can be achieved following the next steps:

1) Downloading this repository.
2) Open SCONE Software.
3) Open "Tools > Preferences" sub-menu.

<p align="center">
<img src="https://github.com/pep248/MasterThesis_HipFlexionExtensionOrthosis/blob/main/scone_tools.png">
</p>

4) In the "SCONE Scenarios" field, set the "Scone Sotware" directory of our recently downloaded repository:
"...\MasterThesis_HipFlexionExtensionOrthosis\SCONE Software".

5) In the "Optimization results" field, set the "Scone Sotware\results" directory of our recently downloaded repository:
"...\MasterThesis_HipFlexionExtensionOrthosis\SCONE Software\results".

<p align="center">
<img src="https://github.com/pep248/MasterThesis_HipFlexionExtensionOrthosis/blob/main/scone_config.png">
</p>


