# BeagleBone
Here you can fin the software used to control the orthosis, some log results obtained during the experiments and a small pdf with instructions on how to upload it into the robot.

Inside the folder "logs"(/pep248/MasterThesis_HipFlexionExtensionOrthosis/tree/main/BeagleBone/logs) you can find the results obtained during the experiments.
## Prerequistes
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


## Tutorial Files
You can follow the tutorial [here](doc/ROS2_Tutorial.pdf).

## Installation

Clone the repo:

```bash
cd ~
git clone https://github.com/FilipHesse/ROS2_Tutorial.git --recurse-submodules
```

Build the packages inside ros1_ws:

```bash
cd ~/ROS2_Tutorial/ros1_ws
source /opt/ros/<ros1_distro>/setup.bash
catkin_make
catkin_make_isolated --install
```
Build the packages inside ros1_ws:

```bash
cd ~/ROS2_Tutorial/ros2_ws
source /opt/ros/<ros2_distro>/setup.bash
colcon build --symlink-install
```

Build the ros1_bridge package inside bridge_ws:

```bash
cd ~/ROS2_Tutorial
source /opt/ros/<ros1_distro>/setup.bash
source /opt/ros/<ros2_distro>/setup.bash
source ros1_ws/install_isolated/setup.bash
source ros2_ws/install/local_setup.bash
cd bridge_ws
colcon build --symlink-install --packages-select ros1_bridge --cmake-force-configure
```

## Covered Subjects:

 1. Introduction
 2. ROS2 Basic Elements
 3. ROS2 Application Management
 4. Hands on: Create a basic Publisher and Subscriber
 5. Building and compiling nodes
 6. ROS Bridge
 7. Real Time
 8. SROS

## Authors
* Roberto Canale: roberto.canale.work@gmail.com
* Filip Hesse: filip_hesse@yahoo.de
* Justin Lee: leej1996@gmail.com
* Daniel Nieto: danieto98@gmail.com
* Steven Palma: imstevenpm.study@gmail.com
* Josep Rueda: rueda_999@hotmail.com
