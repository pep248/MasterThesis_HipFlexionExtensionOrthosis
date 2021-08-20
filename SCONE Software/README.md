# SCONE Software
This folder realates to the files regarding the simulator and the tests performed with it. It includes a folder with simulation results along with some MATLAB Scripts to evaluate them.  

## Prerequisites
This repo assumes the user is on a Windows system and has OpenSim Software and Scone Software installed on his/her machine.

To install OpenSim Software follow the instructions here: [OpenSim](https://simtk.org/frs/index.php?group_id=91).
To install SCONE Software follow the instructions here: [SCONE](https://scone.software/doku.php).

In order to evaluate the provided simulations, we have to properly define the SCONE Software working directory.
This can be achieved following the next steps:

1) Downloading this repository.
2) Open SCONE Software.
3) Open "Tools > Preferences" sub-menu.

<p align="center">
<img src="https://github.com/pep248/MasterThesis_HipFlexionExtensionOrthosis/blob/main/SCONE%20Software/scone_tools.png">
</p>

4) In the "SCONE Scenarios" field, set the "Scone Software" directory of our recently downloaded repository:
"...\MasterThesis_HipFlexionExtensionOrthosis\SCONE Software".

5) In the "Optimization results" field, set the "Scone Sotware\results" directory of our recently downloaded repository:
"...\MasterThesis_HipFlexionExtensionOrthosis\SCONE Software\results".

<p align="center">
<img src="https://github.com/pep248/MasterThesis_HipFlexionExtensionOrthosis/blob/main/SCONE%20Software/scone_config.png">
</p>


## Folders
* Inside the folder [MATLAB Scripts](https://github.com/pep248/MasterThesis_HipFlexionExtensionOrthosis/tree/main/SCONE%20Software/MATLAB%20Scripts), you can find a set of MATLAB Scripts that can be used to evaluate the data and Graphs from OpenSim. You can find aditional information regarding the Scripts inside the foler.
* Inside the folder [controllers](https://github.com/pep248/MasterThesis_HipFlexionExtensionOrthosis/tree/main/SCONE%20Software/controllers), you can find the controllers that we used to command the SCONE model. The file [hip_actuator_controller.lua](https://github.com/pep248/MasterThesis_HipFlexionExtensionOrthosis/blob/main/SCONE%20Software/controllers/hip_actuator_controller.lua) includes the particular algorithm that governs the orthosis actuators.
* Inside the folder [measures](https://github.com/pep248/MasterThesis_HipFlexionExtensionOrthosis/tree/main/SCONE%20Software/measures), you can find the goals and measurements that SCONES uses to give scores to each simulation.
* Inside the folder [model](https://github.com/pep248/MasterThesis_HipFlexionExtensionOrthosis/tree/main/SCONE%20Software/models), we can find the used SCONE model, based on the Geyer's model.
* Inside the folder [parameters](https://github.com/pep248/MasterThesis_HipFlexionExtensionOrthosis/tree/main/SCONE%20Software/parameters), we can find the files with the initial parameters for our simulations. These initial aparameters are the initial values of the variables we defined as "optimization variables", as long as their initial mean value and their standard deviation.
* Inside the folder [results](https://github.com/pep248/MasterThesis_HipFlexionExtensionOrthosis/tree/main/SCONE%20Software/results), we can find the results of the optimizations performed with SCONE. Each particular folder has several files displaying the behaviour of the model and they can be easily opened by simply double clicking them within the SCONE interface. They can evaluate any particular variable within the simulation using the OpenSim simulator plotting tool. Each simulation is heavy in matter of disk space, so as a matter of practicality, only the most relevant simulations have been uploaded.
* Inside the folder [states](https://github.com/pep248/MasterThesis_HipFlexionExtensionOrthosis/tree/main/SCONE%20Software/states), we can find  the files with the initial states for our simulations. These initial states include all the initial values of any varaible within the simulation, inlcuding any body position and orientation.
