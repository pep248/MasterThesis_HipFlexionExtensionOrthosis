% File to open
wkvFull = wkv_load('log_00243_vars.wkv');

% Before using this script, make sure the "left foot load" variable in your
% wkv structure (called as "wkvFull" below) is named like this:
% 'controller/left_foot_load' (if the name is different, rename it manually
%                               in MATLAB before running the script).

wkvFull(16).name = 'controller/left_foot_load';
wkvFull(17).name = 'controller/right_foot_load';

%% Cropping the whole log file
% crop the WKV log to include only one walking bout under constant
% conditions. '30' is the number of the variable based on which you want to
% do the cropping of the data. The best variable for this is typically left
% hip angle or left foot load.
wkvCroppedStruct = wkv_gsubset(wkvFull, 16);

%% Splitting into single gait cycles
% split the walking bout into gait cycles based on heel-strike detection
% from the left foot load data. '5' is the threshold in the foot load for
% detecting heel-strike.
singleCycleWkvs = wkv_split_cycles_leftFootLoad(wkvCroppedStruct,20);

%% Plot the mean and standard deviation over the gait cycles
% based on the stacked vectors of single gait cycles, plot the mean and
% standard deviation of a variable (chosen using varToPlotIndex) over the
% gait cycle.


%Torque
varToPlotIndex = 21; %number of the variable to plot
[stackedVecs, timeVec] = wkv_stack_cycles(singleCycleWkvs, varToPlotIndex);
percentGaitCycleVec = timeVec / timeVec(end) * 100; %convert the time to % gait cycle
figure();
str = '#0000FF'; %blue
%str = '#FF0000'; %red
color_line = sscanf(str(2:end),'%2x%2x%2x',[1 3])/255;
wkv_plot_mean_std(-stackedVecs, percentGaitCycleVec, 0, color_line);
xlabel('Percent Gait Cycle');


%Position
varToPlotIndex = 5; %number of the variable to plot
[stackedVecs, timeVec] = wkv_stack_cycles(singleCycleWkvs, varToPlotIndex);
percentGaitCycleVec = timeVec / timeVec(end) * 100; %convert the time to % gait cycle
figure();
str = '#0000FF'; %blue
%str = '#009900'; %green
color_line = sscanf(str(2:end),'%2x%2x%2x',[1 3])/255;
wkv_plot_position(stackedVecs, percentGaitCycleVec, 0, color_line);
xlabel('Percent Gait Cycle');


%Velocity
varToPlotIndex = 7; %number of the variable to plot
[stackedVecs, timeVec] = wkv_stack_cycles(singleCycleWkvs, varToPlotIndex);
percentGaitCycleVec = timeVec / timeVec(end) * 100; %convert the time to % gait cycle
figure();
str = '#0000FF'; %blue
%str = '#FF0000'; %red
color_line = sscanf(str(2:end),'%2x%2x%2x',[1 3])/255;
wkv_plot_mean_std(stackedVecs, percentGaitCycleVec, 0, color_line);
xlabel('Percent Gait Cycle');


%Acceleration
varToPlotIndex = 7; %number of the variable to plot
[stackedVecs, timeVec] = wkv_stack_cycles(singleCycleWkvs, varToPlotIndex);
percentGaitCycleVec = timeVec / timeVec(end) * 100; %convert the time to % gait cycle
str = '#0000FF'; %blue
%str = '#009900'; %green
color_line = sscanf(str(2:end),'%2x%2x%2x',[1 3])/255;
wkv_plot_acceleration(stackedVecs, percentGaitCycleVec, 0, color_line);
xlabel('Percent Gait Cycle');
