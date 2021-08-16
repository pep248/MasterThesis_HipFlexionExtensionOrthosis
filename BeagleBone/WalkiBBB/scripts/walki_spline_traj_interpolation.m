function [percent, hip, knee] = walki_spline_traj_interpolation()
%SPLINE_TRAJ_INTERPOLATION Interpolate points and generate C-array.
%   Interpolates the given points using piecewise polynomials, then generates
%   the C-array to be copy-pasted in the WalkiBBB code.
clc
clear
close all
global mode

%% Choose desired trajectory here.
%mode = 'platform_first_step_from_fast_gait';
%mode = 'platform_first_step';
%mode = 'platform_second_step';

%mode = 'normal_gait';
%mode = 'normal_gait_first_step';
mode = 'normal_gait_last_step';

%mode = 'fast_gait';
%mode = 'fast_gait_first_step';

%mode = 'fast_stairs';
%mode = 'stairs_first_step';
%mode = 'stairs_last_step';

%mode = 'fast_stairs_small';
%mode = 'stairs_small_first_step';
%mode = 'stairs_small_last_step';

%%
switch mode  
            
    case 'slow_gait'
        knee_angle = [2 3 3.3 3.5 4.5 5 5.1 26 50 65 60 35 10 2]*0.7; % [deg].
        knee_perc  = [0 10 15 20 30 40 50 60 65 70 78 87.5 93.75 100]; % [%].
        
        hip_angle = [12 10 8 6 2 0 -3.25 4 15 25 29 25 20 12]*0.7; % [deg].
        hip_perc  = [0 7 15 20 30 40 50 60 65 70 80 87.5 93.75 100]; % [%].
        
        close all;
        hFig = figure('Name', 'Slow gait trajectories');
        figDisplay = figure('Name', 'Visualisation of the trajectories');
        
        % Hip.
        [coefs, hip_interp] = generate_traj(hip_perc, hip_angle, 1, hFig, 'Hip');
        print_c_code(hip_perc, coefs, 'Hip.', ...
            'slowGaitStridePercentsHip', 'slowGaitTrajCoefsHip');
        
        % Knee.
        [coefs, knee_interp] = generate_traj(knee_perc, knee_angle, 1, hFig, 'Knee');
        print_c_code(knee_perc, coefs, 'Knee.', ...
            'slowGaitStridePercentsKnee', 'slowGaitTrajCoefsKnee');
        
    case 'slow_gait_first_step'
        stance_knee_angle = [0 3.65 3.45 4.17 6.87 10.00 5.1]*0.7; % [deg].
        stance_knee_perc = [0 10 15 20 30 40 50]; % [%].
        
        stance_hip_angle = [0 -1 -2 -3 -4 -4.1 -4.38 -3.25]*0.7; % [deg].
        stance_hip_perc = [0 10 15 20 30 36 40 50]; % [%].
        
        swing_knee_angle = [0 37.95 68.58 82.92 76 47.78 22 2]*0.7; % [deg].
        swing_knee_perc = [50 60 65 70 78 87.5 93.75 100]-50; % [%].
        
        swing_hip_angle = [0 6 20 30 35.22 38.31 31.59 20 12]*0.7; % [deg].
        swing_hip_perc = [50 55 60 65 70 80 87.5 93.75 100]-50; % [%].
        
        %
        close all;
        figStance = figure('Name', 'Slow gait first step stance trajectories');
        figSwing = figure('Name', 'Slow gait first step swing trajectories');
        figDisplay = figure('Name', 'Visualisation of the trajectories');
        
        % Hip.
        [coefs, hip_interp_stance] = generate_traj(stance_hip_perc, stance_hip_angle, 0, figStance, 'Hip');
        print_c_code(stance_hip_perc, coefs, ...
            'First (half step), hip, stance leg.', ...
            'slowGaitFirstStepHipStancePercents', 'slowGaitFirstStepHipStanceTrajCoefs');
        
        
        [coefs, hip_interp_swing] = generate_traj(swing_hip_perc, swing_hip_angle, 0, figSwing, 'Hip');
        print_c_code(swing_hip_perc, coefs, ...
            'First (half step), hip, swing leg.', ...
            'slowGaitFirstStepHipSwingPercents', 'slowGaitFirstStepHipSwingTrajCoefs');
        
        hip_interp = [hip_interp_stance; hip_interp_swing];
        
        % Knee.
        [coefs, knee_interp_stance] = generate_traj(stance_knee_perc, stance_knee_angle, 0, figStance, 'Knee');
        print_c_code(stance_knee_perc, coefs, ...
            'First (half step), knee, stance leg.', ...
            'slowGaitFirstStepKneeStancePercents', 'slowGaitFirstStepKneeStanceTrajCoefs');
        
        [coefs, knee_interp_swing] = generate_traj(swing_knee_perc, swing_knee_angle, 0, figSwing, 'Knee');
        print_c_code(swing_knee_perc, coefs, ...
            'First (half step), knee, swing leg.', ...
            'slowGaitFirstStepKneeSwingPercents', 'slowGaitFirstStepKneeSwingTrajCoefs');
        
        knee_interp = [knee_interp_stance; knee_interp_swing];
        
    case 'slow_gait_last_step'
        stance_knee_angle = [2 2 2 2 2 0]; % [deg].
        stance_knee_perc = [0 10 20 30 40 50]; % [%].
        
        stance_hip_angle = [15 12 8 6 3 0]; % [deg].
        stance_hip_perc = [0 10 20 30 40 50]; % [%].
        
        swing_knee_angle = [9.2 35 60 60 35 0]; % [deg].
        swing_knee_perc = [50 60 70 80 90 100]-50; % [%].
        
        swing_hip_angle = [-4  4 20 35 20 0]; % [deg].
        swing_hip_perc = [50 60 70 80 90 100]-50; % [%].     
        
        %
        close all;
        figStance = figure('Name', 'Slow gait last step stance trajectories');
        figSwing = figure('Name', 'Slow gait last step swing trajectories');
        figDisplay = figure('Name', 'Visualisation of the trajectories');
        
        % Hip.
        [coefs, hip_interp_stance] = generate_traj(stance_hip_perc, stance_hip_angle, 0, figStance, 'Hip');
        print_c_code(stance_hip_perc, coefs, ...
            'Last (half step), hip, stance leg.', ...
            'slowGaitLastStepHipStancePercents', 'slowGaitLastStepHipStanceTrajCoefs');
        
        
        [coefs, hip_interp_swing] = generate_traj(swing_hip_perc, swing_hip_angle, 0, figSwing, 'Hip');
        print_c_code(swing_hip_perc, coefs, ...
            'Last (half step), hip, swing leg.', ...
            'slowGaitLastStepHipSwingPercents', 'slowGaitLastStepHipSwingTrajCoefs');
        
        hip_interp = [hip_interp_stance; hip_interp_swing];
        
        
        % Knee.
        [coefs, knee_interp_stance] = generate_traj(stance_knee_perc, stance_knee_angle, 0, figStance, 'Knee');
        print_c_code(stance_knee_perc, coefs, ...
            'Last (half step), knee, stance leg.', ...
            'slowGaitLastStepKneeStancePercents', 'slowGaitLastStepKneeStanceTrajCoefs');
        
        [coefs, knee_interp_swing] = generate_traj(swing_knee_perc, swing_knee_angle, 0, figSwing, 'Knee');
        print_c_code(swing_knee_perc, coefs, ...
            'Last (half step), knee, swing leg.', ...
            'slowGaitLastStepKneeSwingPercents', 'slowGaitLastStepKneeSwingTrajCoefs');
        
        knee_interp = [knee_interp_stance; knee_interp_swing];
        
    case 'normal_gait'
        % Steps used during most of 2017.
        knee_angle = [2 2.1 7.5  8 26 50 65 53 35   9   2]; % [deg].
        knee_perc  = [0   3  48 50 58 65 75 82 87 94 100]; % [%].
     
        hip_angle  = [10 9.8 -7 -9.17 -9   4 15 24 26   21 16  10]; % [deg].
        hip_perc   = [ 0 3   40   50  52 60 66 73 80 87.5 93 100]; % [%].
        
        close all;
        hFig = figure('Name', 'Normal gait trajectories');
        figDisplay = figure('Name', 'Visualisation of the trajectories');
        
        % Hip.
        [coefs, hip_interp] = generate_traj(hip_perc, hip_angle, 1, hFig, 'Hip');
        print_c_code(hip_perc, coefs, 'Hip.', ...
            'normalGaitStridePercentsHip', 'normalGaitTrajCoefsHip');
        
        % Knee.
        [coefs, knee_interp] = generate_traj(knee_perc, knee_angle, 1, hFig, 'Knee');
        print_c_code(knee_perc, coefs, 'Knee.', ...
            'normalGaitStridePercentsKnee', 'normalGaitTrajCoefsKnee');
        
    case 'normal_gait_first_step'
        stance_knee_angle = [0  1   2 2.5  5  7.8  8]; % [deg].
        stance_knee_perc =  [0 10  20  30 40   48 50]; % [%].
        
        stance_hip_angle = [0 0 -0.8 -3.5 -9 -9.17]; % [deg].
        stance_hip_perc =  [0 8   30   40 48    50]; % [%].
        
        swing_knee_angle = [0  8 30 60 70 68 48      22 3.5   2]; % [deg].
        swing_knee_perc = [50 55 63 70 75 82 87.5 93.75  98 100]-50; % [%].
        
        swing_hip_angle = [0  3 20 30 38    24 11 10]; % [deg].
        swing_hip_perc = [50 55 65 70 83 93.75 98 100]-50; % [%].
        
        %
        close all;
        figStance = figure('Name', 'Normal gait first step stance trajectories');
        figSwing = figure('Name', 'Normal gait first step swing trajectories');
        figDisplay = figure('Name', 'Visualisation of the trajectories');
        
        % Hip.
        [coefs, hip_interp_stance] = generate_traj(stance_hip_perc, stance_hip_angle, 0, figStance, 'Hip');
        print_c_code(stance_hip_perc, coefs, ...
            'First (half step), hip, stance leg.', ...
            'normalGaitFirstStepHipStancePercents', 'normalGaitFirstStepHipStanceTrajCoefs');
        
        [coefs, hip_interp_swing] = generate_traj(swing_hip_perc, swing_hip_angle, 0, figSwing, 'Hip');
        print_c_code(swing_hip_perc, coefs, ...
            'First (half step), hip, swing leg.', ...
            'normalGaitFirstStepHipSwingPercents', 'normalGaitFirstStepHipSwingTrajCoefs');
        
        hip_interp = [hip_interp_stance; hip_interp_swing];
        
        % Knee.
        [coefs, knee_interp_stance] = generate_traj(stance_knee_perc, stance_knee_angle, 0, figStance, 'Knee');
        print_c_code(stance_knee_perc, coefs, ...
            'First (half step), knee, stance leg.', ...
            'normalGaitFirstStepKneeStancePercents', 'normalGaitFirstStepKneeStanceTrajCoefs');
        
        [coefs, knee_interp_swing] = generate_traj(swing_knee_perc, swing_knee_angle, 0, figSwing, 'Knee');
        print_c_code(swing_knee_perc, coefs, ...
            'First (half step), knee, swing leg.', ...
            'normalGaitFirstStepKneeSwingPercents', 'normalGaitFirstStepKneeSwingTrajCoefs');
        
        knee_interp = [knee_interp_stance; knee_interp_swing];
        
    case 'normal_gait_last_step'
        stance_knee_angle = [2  2 1.2 0.3  0]; % [deg].
        stance_knee_perc =  [0 30 41  45 50]; % [%].
        
        stance_hip_angle = [10 9.5 0.5  0]; % [deg].
        stance_hip_perc =  [ 0  10  45 50]; % [%].
        
        swing_knee_angle = [ 8 35 60 60  5   0]; % [deg].
        swing_knee_perc =  [50 60 70 80 95 100]-50; % [%].
        
        swing_hip_angle = [-9.17  1 20 35  5   0]; % [deg].
        swing_hip_perc =  [   50 60 70 80 95 100]-50; % [%].     
        
        %
        close all;
        figStance = figure('Name', 'Normal gait last step stance trajectories');
        figSwing = figure('Name', 'Normal gait last step swing trajectories');
        figDisplay = figure('Name', 'Visualisation of the trajectories');
        
        % Hip.
        [coefs, hip_interp_stance] = generate_traj(stance_hip_perc, stance_hip_angle, 0, figStance, 'Hip');
        print_c_code(stance_hip_perc, coefs, ...
            'Last (half step), hip, stance leg.', ...
            'normalGaitLastStepHipStancePercents', 'normalGaitLastStepHipStanceTrajCoefs');
        
        
        [coefs, hip_interp_swing] = generate_traj(swing_hip_perc, swing_hip_angle, 0, figSwing, 'Hip');
        print_c_code(swing_hip_perc, coefs, ...
            'Last (half step), hip, swing leg.', ...
            'normalGaitLastStepHipSwingPercents', 'normalGaitLastStepHipSwingTrajCoefs');
        
        hip_interp = [hip_interp_stance; hip_interp_swing];
        
        
        % Knee.
        [coefs, knee_interp_stance] = generate_traj(stance_knee_perc, stance_knee_angle, 0, figStance, 'Knee');
        print_c_code(stance_knee_perc, coefs, ...
            'Last (half step), knee, stance leg.', ...
            'normalGaitLastStepKneeStancePercents', 'normalGaitLastStepKneeStanceTrajCoefs');
        
        [coefs, knee_interp_swing] = generate_traj(swing_knee_perc, swing_knee_angle, 0, figSwing, 'Knee');
        print_c_code(swing_knee_perc, coefs, ...
            'Last (half step), knee, swing leg.', ...
            'normalGaitLastStepKneeSwingPercents', 'normalGaitLastStepKneeSwingTrajCoefs');
        
        knee_interp = [knee_interp_stance; knee_interp_swing];
        
    case 'fast_gait'
%         % Longer step (for speed record) - pre Cybathlon 2017 week.
%         knee_angle = [2 2.2 3.5 8.7 9.3 23 50 65 53 35 10 2]; % [deg].
%         knee_perc  = [0 5   20  45  50  58 65 75 82 87 94 100]; % [%].
%      
%         hip_angle  = [20 18 11 6   1 -5 -7  1 15 25 29  28    24   20]; % [deg].
%         hip_perc   = [0   6 15 22 30 40 50 60 66 73 80 87.5 93.75 100]; % [%].
        
        % Longer step (for speed record) - new.
        knee_angle = [2 2.2 3.5 8.7 9.3 23 50 65 53 35 10 2]; % [deg].
        knee_perc  = [0 5   20  45  50  58 65 75 82 87 94 100]; % [%].
     
        hip_angle  = [20 18 11 6   1 -5 -7 -2 15 29  28  23   20]; % [deg].
        hip_perc   = [0   6 15 22 30 40 50 60 70 82 89   94 100]; % [%].
                
%         % Cybathlon steps        
%         knee_angle = [2 3 3.3 3.5 4 5 9.2 26 50 65 53 35 10 2]; % [deg].
%         knee_perc  = [0 10 15 20 30 45 50 58 65 75 82 87 94 100]; % [%].
%      
%         hip_angle  = [15 13 10 6 1 -5 -4 4 15 25 29 25 20 15]; % [deg].
%         hip_perc   = [0 7 15 22 30 40 50 60 66 73 80 87.5 93.75 100]; % [%].
        
        close all;
        hFig = figure('Name', 'Fast gait trajectories');
        figDisplay = figure('Name', 'Visualisation of the trajectories');
        
        % Hip.
        [coefs, hip_interp] = generate_traj(hip_perc, hip_angle, 1, hFig, 'Hip');
        print_c_code(hip_perc, coefs, 'Hip.', ...
            'fastGaitStridePercentsHip', 'fastGaitTrajCoefsHip');
        
        % Knee.
        [coefs, knee_interp] = generate_traj(knee_perc, knee_angle, 1, hFig, 'Knee');
        print_c_code(knee_perc, coefs, 'Knee.', ...
            'fastGaitStridePercentsKnee', 'fastGaitTrajCoefsKnee');
        
    case 'fast_gait_first_step'
        % Longer step
        stance_knee_angle = [0 1 1.5 2 2.5 5 9.3]; % [deg].
        stance_knee_perc = [0 10 15 20 30 40 50]; % [%].
        
        stance_hip_angle = [0 0 -0.2 -0.8 -3 -5 -6 -7]; % [deg].
        stance_hip_perc = [0 8 15 20 30 36 40 50]; % [%].
        
        swing_knee_angle = [0 8 30 60 70 68 48 22 2]; % [deg].
        swing_knee_perc = [50 55 63 70 75 82 87.5 93.75 100]-50; % [%].
        
        swing_hip_angle = [0 6 18 30 37 38 33 24 20]; % [deg].
        swing_hip_perc = [50 55 60 65 70 80 87.5 93.75 100]-50; % [%].
        
      %% Cybathlon step
%         stance_knee_angle = [0 1 1.5 2 2.5 3 4]; % [deg].
%         stance_knee_perc = [0 10 15 20 30 40 50]; % [%].
%         
%         stance_hip_angle = [0 0 -0.2 -0.8 -2.3 -3 -3.5 -4]; % [deg].
%         stance_hip_perc = [0 8 15 20 30 36 40 50]; % [%].
%         
%         swing_knee_angle = [0 8 30 60 70 68 48 22 2]; % [deg].
%         swing_knee_perc = [50 55 63 70 75 82 87.5 93.75 100]-50; % [%].
%         
%         swing_hip_angle = [0 5 15 28 35 38 33 24 15]; % [deg].
%         swing_hip_perc = [50 55 60 65 70 80 87.5 93.75 100]-50; % [%].
        
        %
        close all;
        figStance = figure('Name', 'Fast gait first step stance trajectories');
        figSwing = figure('Name', 'Fast gait first step swing trajectories');
        figDisplay = figure('Name', 'Visualisation of the trajectories');
        
        % Hip.
        [coefs, hip_interp_stance] = generate_traj(stance_hip_perc, stance_hip_angle, 0, figStance, 'Hip');
        print_c_code(stance_hip_perc, coefs, ...
            'First (half step), hip, stance leg.', ...
            'fastGaitFirstStepHipStancePercents', 'fastGaitFirstStepHipStanceTrajCoefs');
        
        [coefs, hip_interp_swing] = generate_traj(swing_hip_perc, swing_hip_angle, 0, figSwing, 'Hip');
        print_c_code(swing_hip_perc, coefs, ...
            'First (half step), hip, swing leg.', ...
            'fastGaitFirstStepHipSwingPercents', 'fastGaitFirstStepHipSwingTrajCoefs');
        
        hip_interp = [hip_interp_stance; hip_interp_swing];
        
        % Knee.
        [coefs, knee_interp_stance] = generate_traj(stance_knee_perc, stance_knee_angle, 0, figStance, 'Knee');
        print_c_code(stance_knee_perc, coefs, ...
            'First (half step), knee, stance leg.', ...
            'fastGaitFirstStepKneeStancePercents', 'fastGaitFirstStepKneeStanceTrajCoefs');
        
        [coefs, knee_interp_swing] = generate_traj(swing_knee_perc, swing_knee_angle, 0, figSwing, 'Knee');
        print_c_code(swing_knee_perc, coefs, ...
            'First (half step), knee, swing leg.', ...
            'fastGaitFirstStepKneeSwingPercents', 'fastGaitFirstStepKneeSwingTrajCoefs');
        
        knee_interp = [knee_interp_stance; knee_interp_swing];
        
    case 'stride_over_gait'
        knee_angle = [2 6 2]; % [deg].
        knee_perc  = [0 50 100]; % [%].
     
        hip_angle  = [23 -10 23]; % [deg].
        hip_perc   = [0 50 100]; % [%].
        
        close all;
        hFig = figure('Name', 'Stride-over gait trajectories');
        figDisplay = figure('Name', 'Visualisation of the trajectories');
        
        % Hip.
        [coefs, hip_interp] = generate_traj(hip_perc, hip_angle, 1, hFig, 'Hip');
        print_c_code(hip_perc, coefs, 'Hip.', ...
            'strideOverGaitStridePercentsHip', 'strideOverGaitTrajCoefsHip');
        
        % Knee.
        [coefs, knee_interp] = generate_traj(knee_perc, knee_angle, 1, hFig, 'Knee');
        print_c_code(knee_perc, coefs, 'Knee.', ...
            'strideOverGaitStridePercentsKnee', 'strideOverGaitTrajCoefsKnee');       
        
    case 'stride_over_gait_first_step'
        stance_knee_angle = [0 1 6]; % [deg].
        stance_knee_perc = [0 25 50]; % [%].
        
        stance_hip_angle = [0 -2 -10]; % [deg].
        stance_hip_perc = [0 25 50]; % [%].
        
        swing_knee_angle = [0 10 40 85 95 80 55 20 2]; % [deg].
        swing_knee_perc = [50 55 63 70 77 84 87.5 93.75 100]-50; % [%].
        
        swing_hip_angle = [0 8 18 25 30 55 55 35 23]; % [deg].
        swing_hip_perc = [50 55 60 63 72 80 87.5 93.75 100]-50; % [%].
        
        %
        close all;
        figStance = figure('Name', 'Stride-over gait first step stance trajectories');
        figSwing = figure('Name', 'Stride-over gait first step swing trajectories');
        figDisplay = figure('Name', 'Visualisation of the trajectories');
        
        % Hip.
        [coefs, hip_interp_stance] = generate_traj(stance_hip_perc, stance_hip_angle, 0, figStance, 'Hip');
        print_c_code(stance_hip_perc, coefs, ...
            'First (half step), hip, stance leg.', ...
            'strideOverGaitFirstStepHipStancePercents', 'strideOverGaitFirstStepHipStanceTrajCoefs');
        
        [coefs, hip_interp_swing] = generate_traj(swing_hip_perc, swing_hip_angle, 0, figSwing, 'Hip');
        print_c_code(swing_hip_perc, coefs, ...
            'First (half step), hip, swing leg.', ...
            'strideOverGaitFirstStepHipSwingPercents', 'strideOverGaitFirstStepHipSwingTrajCoefs');
        
        hip_interp = [hip_interp_stance; hip_interp_swing];
        
        % Knee.
        [coefs, knee_interp_stance] = generate_traj(stance_knee_perc, stance_knee_angle, 0, figStance, 'Knee');
        print_c_code(stance_knee_perc, coefs, ...
            'First (half step), knee, stance leg.', ...
            'strideOverGaitFirstStepKneeStancePercents', 'strideOverGaitFirstStepKneeStanceTrajCoefs');
        
        [coefs, knee_interp_swing] = generate_traj(swing_knee_perc, swing_knee_angle, 0, figSwing, 'Knee');
        print_c_code(swing_knee_perc, coefs, ...
            'First (half step), knee, swing leg.', ...
            'strideOverGaitFirstStepKneeSwingPercents', 'strideOverGaitFirstStepKneeSwingTrajCoefs');
        
        knee_interp = [knee_interp_stance; knee_interp_swing];
        
case 'stride_over_gait_last_step'
        stance_knee_angle = [2 0]; % [deg].
        stance_knee_perc = [0 50]; % [%].
        
        stance_hip_angle = [23 0]; % [deg].
        stance_hip_perc = [0 50]; % [%].
        
        swing_knee_angle = [6 35 85 85 35 0]; % [deg].
        swing_knee_perc = [50 60 70 80 90 100]-50; % [%].
        
        swing_hip_angle = [-10 4 35 50 30 0]; % [deg].
        swing_hip_perc = [50 60 70 80 90 100]-50; % [%].   
        
        %
        close all;
        figStance = figure('Name', 'Fast gait last step stance trajectories');
        figSwing = figure('Name', 'Fast gait last step swing trajectories');
        figDisplay = figure('Name', 'Visualisation of the trajectories');
        
        % Hip.
        [coefs, hip_interp_stance] = generate_traj(stance_hip_perc, stance_hip_angle, 0, figStance, 'Hip');
        print_c_code(stance_hip_perc, coefs, ...
            'Last (half step), hip, stance leg.', ...
            'strideOverGaitLastStepHipStancePercents', 'strideOverGaitLastStepHipStanceTrajCoefs');
        
        
        [coefs, hip_interp_swing] = generate_traj(swing_hip_perc, swing_hip_angle, 0, figSwing, 'Hip');
        print_c_code(swing_hip_perc, coefs, ...
            'Last (half step), hip, swing leg.', ...
            'strideOverGaitLastStepHipSwingPercents', 'strideOverGaitLastStepHipSwingTrajCoefs');
        
        hip_interp = [hip_interp_stance; hip_interp_swing];
        
        
        % Knee.
        [coefs, knee_interp_stance] = generate_traj(stance_knee_perc, stance_knee_angle, 0, figStance, 'Knee');
        print_c_code(stance_knee_perc, coefs, ...
            'Last (half step), knee, stance leg.', ...
            'strideOverGaitLastStepKneeStancePercents', 'strideOverGaitLastStepKneeStanceTrajCoefs');
        
        [coefs, knee_interp_swing] = generate_traj(swing_knee_perc, swing_knee_angle, 0, figSwing, 'Knee');
        print_c_code(swing_knee_perc, coefs, ...
            'Last (half step), knee, swing leg.', ...
            'strideOverGaitLastStepKneeSwingPercents', 'strideOverGaitLastStepKneeSwingTrajCoefs');
        
        knee_interp = [knee_interp_stance; knee_interp_swing];
        
    case 'stairs_first_step'
        stance_knee_angle = [0 0.1 1.9  2]; % [deg].
        stance_knee_perc =  [0  10  40 50]; % [%].
        
        stance_hip_angle = [0 0.1 1.9  2]; % [deg].
        stance_hip_perc =  [0  10  40 50]; % [%].
%%  Old      
%         stance_knee_angle = [0 1 1.5 2 2 6 12]; % [deg].
%         stance_knee_perc = [0 5 10 20 28 40 50]; % [%].
% 
%         stance_hip_angle = [0 0 0 0 1 4 3]; % [deg].
%         stance_hip_perc = [0 8 13 20 30 40 50]; % [%].

        swing_knee_angle = [0 45 105 80 62]; % [deg].
        swing_knee_perc = [50 60  75 90 100]-50; % [%].

        swing_hip_angle = [0 26 70 73  62]; % [deg].
        swing_hip_perc = [50 60 78 90 100]-50; % [%].
        
%         % Escaliers Montbenon
%         swing_knee_angle = [0 37 65 95 80 60]; % [deg].
%         swing_knee_perc = [50 60 65 75 90 100]-50; % [%].
% 
%         swing_hip_angle = [0 20 40 65 78 60]; % [deg].
%         swing_hip_perc = [50 60 69 78 90 100]-50; % [%].

%% Old        
%         swing_knee_angle = [0 45 74 97 80 65]; % [deg].
%         swing_knee_perc = [50 60 65 80 90 100]-50; % [%].
%         
%         swing_hip_angle = [0 23 47 73 80 65]; % [deg].
%         swing_hip_perc = [50 60 69 78 90 100]-50; % [%].

        %%        Old Old
%         swing_knee_angle = [0 45 70 97 80 65]; % [deg].
%         swing_knee_perc = [50 60 65 80 90 100]-50; % [%].
%         
%         swing_hip_angle = [0 30 50 75 80 65]; % [deg].
%         swing_hip_perc = [50 60 67 78 90 100]-50; % [%].
        
        %
        close all;
        figStance = figure('Name', 'Stairs first step stance trajectories');
        figSwing = figure('Name', 'Stairs first step swing trajectories');
        figDisplay = figure('Name', 'Visualisation of the trajectories');
        
        % Hip.
        [coefs, hip_interp_stance] = generate_traj(stance_hip_perc, stance_hip_angle, 0, figStance, 'Hip');
        print_c_code(stance_hip_perc, coefs, ...
            'First (half) step, hip, stance leg.', ...
            'fastStairsFirstStepHipStancePercents', 'fastStairsFirstStepHipStanceTrajCoefs');
        
        [coefs, hip_interp_swing] = generate_traj(swing_hip_perc, swing_hip_angle, 0, figSwing, 'Hip');
        print_c_code(swing_hip_perc, coefs, ...
            'First (half) step, hip, swing leg.', ...
            'fastStairsFirstStepHipSwingPercents', 'fastStairsFirstStepHipSwingTrajCoefs');
        
        hip_interp = [hip_interp_stance; hip_interp_swing];
        
        % Knee.
        [coefs, knee_interp_stance] = generate_traj(stance_knee_perc, stance_knee_angle, 0, figStance, 'Knee');
        print_c_code(stance_knee_perc, coefs, ...
            'First (half) step, knee, stance leg.', ...
            'fastStairsFirstStepKneeStancePercents', 'fastStairsFirstStepKneeStanceTrajCoefs');
        
        [coefs, knee_interp_swing] = generate_traj(swing_knee_perc, swing_knee_angle, 0, figSwing, 'Knee');
        print_c_code(swing_knee_perc, coefs, ...
            'First (half) step, knee, swing leg.', ...
            'fastStairsFirstStepKneeSwingPercents', 'fastStairsFirstStepKneeSwingTrajCoefs');
        
        knee_interp = [knee_interp_stance; knee_interp_swing];
        
    case 'stairs_last_step'
        stance_knee_angle = [62 50 38 23 15 7 1 0]; % [deg].
        stance_knee_perc = [0 5 10 20 28 36 47 50]; % [%].
        
        stance_hip_angle = [62 46 28 16  8  2  0]; % [deg].
        stance_hip_perc =  [ 0  6 15 22 30 40 50]; % [%].
        
        swing_knee_angle = [2 60 90 80 40 0]; % [deg].
        swing_knee_perc = [50 60 70 77 90 100]-50; % [%].
        
        swing_hip_angle = [2 20 38 43 25 0]; % [deg].
        swing_hip_perc = [50 60 67 78 90 100]-50; % [%].
        
        %
        close all;
        figStance = figure('Name', 'Stairs last step stance trajectories');
        figSwing = figure('Name', 'Stairs last step swing trajectories');
        figDisplay = figure('Name', 'Visualisation of the trajectories');
        
        % Hip.
        [coefs, hip_interp_stance] = generate_traj(stance_hip_perc, stance_hip_angle, 0, figStance, 'Hip');
        print_c_code(stance_hip_perc, coefs, ...
            'Last (half) step, hip, stance leg.', ...
            'fastStairsLastStepHipStancePercents', 'fastStairsLastStepHipStanceTrajCoefs');
        
        [coefs, hip_interp_swing] = generate_traj(swing_hip_perc, swing_hip_angle, 0, figSwing, 'Hip');
        print_c_code(swing_hip_perc, coefs, ...
            'Last (half) step, hip, swing leg.', ...
            'fastStairsLastStepHipSwingPercents', 'fastStairsLastStepHipSwingTrajCoefs');
        
        hip_interp = [hip_interp_stance; hip_interp_swing];
        
        % Knee.
        [coefs, knee_interp_stance] = generate_traj(stance_knee_perc, stance_knee_angle, 0, figStance, 'Knee');
        print_c_code(stance_knee_perc, coefs, ...
            'Last (half) step, knee, stance leg.', ...
            'fastStairsLastStepKneeStancePercents', 'fastStairsLastStepKneeStanceTrajCoefs');
        
        [coefs, knee_interp_swing] = generate_traj(swing_knee_perc, swing_knee_angle, 0, figSwing, 'Knee');
        print_c_code(swing_knee_perc, coefs, ...
            'Last (half) step, knee, swing leg.', ...
            'fastStairsLastStepKneeSwingPercents', 'fastStairsLastStepKneeSwingTrajCoefs');
        knee_interp = [knee_interp_stance; knee_interp_swing];
        
    case 'fast_stairs'
        knee_angle = [62 30  10 2.05  2 40 78 100 80 63 62]; % [deg].        
        knee_perc =  [ 0 20  30   49 50 60 65  75 90 97 100];  % [%].
        
        hip_angle = [62 30 10 2.05  2  18 35 65 75 63  62]; % [deg]. 0 23 43 73 82 65
        hip_perc =  [ 0 20 30   49 50  60 67 78 90 97 100]; % [%].

%         knee_angle = [62 50 38 20 10  6 2.05  2 50 78 100 80 65 62]; %[0 45 76 105 80 65]; % [deg].        
%         knee_perc =  [ 0 4  10 20 28 36   49 50 60 65  75 90 97 100];  %[%].[50 60 65 75 90 100]
%         
%         hip_angle = [62 45 28 16 8   4  2  4 20 60 75 65  62]; % [deg]. 0 23 43 73 82 65
%         hip_perc =  [ 0  6 15 22 30 40 50 60 67 78 90 97 100]; % [%].
        
% %   Escaliers 23/01/2017.
%         knee_angle = [65 50 38 22 12 5 12 50 83 105 90 65]; %[0 45 76 105 80 65]; % [deg].        
%         knee_perc = [0 5 10 20 28 36 50 60 65 75 90 100];  %[%].[50 60 65 75 90 100]
%         
%         hip_angle = [65 46 28 16 8 2 4 18 40 73 82 65]; % [deg]. 0 23 43 73 82 65
%         hip_perc = [0 6 15 22 30 40 50 60 67 78 90 100]; % [%].
        
% %   Escaliers Montbenon
%         knee_angle = [60 50 38 22 12 5 12 50 83 100 83 60]; %[0 45 76 105 80 65]; % [deg].        
%         knee_perc = [0 5 10 20 28 36 50 60 65 75 90 100];  %[%].[50 60 65 75 90 100]
%         
%         hip_angle = [60 46 28 16 8 2 4 18 40 67 75 60]; % [deg]. 0 23 43 73 82 65
%         hip_perc = [0 6 15 22 30 40 50 60 67 78 90 100]; % [%].
        
        
        close all;
        hFig = figure('Name', 'fast stair trajectories');
        figDisplay = figure('Name', 'Visualisation of the trajectories');
        
        % Hip.
        [coefs, hip_interp] = generate_traj(hip_perc, hip_angle, 1, hFig, 'Hip');
        print_c_code(hip_perc, coefs, 'Hip.', ...
            'fastStairsStridePercentsHip', 'fastStairsTrajCoefsHip');
        
        % Knee.
        [coefs, knee_interp] = generate_traj(knee_perc, knee_angle, 1, hFig, 'Knee');
        print_c_code(knee_perc, coefs, 'Knee.', ...
            'fastStairsStridePercentsKnee', 'fastStairsTrajCoefsKnee');
        
    case 'stairs_small_first_step'
        stance_knee_angle = [0 0.1 1.9  2]; % [deg].
        stance_knee_perc =  [0  10  40 50]; % [%].
        
        stance_hip_angle = [0 0.1 1.9  2]; % [deg].
        stance_hip_perc =  [0  10  40 50]; % [%].

        swing_knee_angle = [0 45 105 80 50]; % [deg].
        swing_knee_perc = [50 60  75 90 100]-50; % [%].

        swing_hip_angle = [0 26 70 60  50]; % [deg].
        swing_hip_perc = [50 60 85 95 100]-50; % [%].
        
        %
        close all;
        figStance = figure('Name', 'Stairs small first step stance trajectories');
        figSwing = figure('Name', 'Stairs small first step swing trajectories');
        figDisplay = figure('Name', 'Visualisation of the trajectories');
        
        % Hip.
        [coefs, hip_interp_stance] = generate_traj(stance_hip_perc, stance_hip_angle, 0, figStance, 'Hip');
        print_c_code(stance_hip_perc, coefs, ...
            'First (half) step, hip, stance leg.', ...
            'fastStairsSFirstStepHipStancePercents', 'fastStairsSFirstStepHipStanceTrajCoefs');
        
        [coefs, hip_interp_swing] = generate_traj(swing_hip_perc, swing_hip_angle, 0, figSwing, 'Hip');
        print_c_code(swing_hip_perc, coefs, ...
            'First (half) step, hip, swing leg.', ...
            'fastStairsSFirstStepHipSwingPercents', 'fastStairsSFirstStepHipSwingTrajCoefs');
        
        hip_interp = [hip_interp_stance; hip_interp_swing];
        
        % Knee.
        [coefs, knee_interp_stance] = generate_traj(stance_knee_perc, stance_knee_angle, 0, figStance, 'Knee');
        print_c_code(stance_knee_perc, coefs, ...
            'First (half) step, knee, stance leg.', ...
            'fastStairsSFirstStepKneeStancePercents', 'fastStairsSFirstStepKneeStanceTrajCoefs');
        
        [coefs, knee_interp_swing] = generate_traj(swing_knee_perc, swing_knee_angle, 0, figSwing, 'Knee');
        print_c_code(swing_knee_perc, coefs, ...
            'First (half) step, knee, swing leg.', ...
            'fastStairsSFirstStepKneeSwingPercents', 'fastStairsSFirstStepKneeSwingTrajCoefs');
        
        knee_interp = [knee_interp_stance; knee_interp_swing];
        
    case 'stairs_small_last_step'
        stance_knee_angle = [50 48  1 0]; % [deg].
        stance_knee_perc =  [ 0  5  47 50]; % [%].
        
        stance_hip_angle = [50 48  1  0]; % [deg].
        stance_hip_perc =  [ 0  5 47 50]; % [%].
        
        swing_knee_angle = [2 54 90 80 40 0]; % [deg].
        swing_knee_perc = [50 60 70 77 90 100]-50; % [%].
        
        swing_hip_angle = [2 26 38 43 25 0]; % [deg].
        swing_hip_perc = [50 60 67 78 90 100]-50; % [%].
        
        %
        close all;
        figStance = figure('Name', 'Stairs small last step stance trajectories');
        figSwing = figure('Name', 'Stairs small last step swing trajectories');
        figDisplay = figure('Name', 'Visualisation of the trajectories');
        
        % Hip.
        [coefs, hip_interp_stance] = generate_traj(stance_hip_perc, stance_hip_angle, 0, figStance, 'Hip');
        print_c_code(stance_hip_perc, coefs, ...
            'Last (half) step, hip, stance leg.', ...
            'fastStairsSLastStepHipStancePercents', 'fastStairsSLastStepHipStanceTrajCoefs');
        
        [coefs, hip_interp_swing] = generate_traj(swing_hip_perc, swing_hip_angle, 0, figSwing, 'Hip');
        print_c_code(swing_hip_perc, coefs, ...
            'Last (half) step, hip, swing leg.', ...
            'fastStairsSLastStepHipSwingPercents', 'fastStairsSLastStepHipSwingTrajCoefs');
        
        hip_interp = [hip_interp_stance; hip_interp_swing];
        
        % Knee.
        [coefs, knee_interp_stance] = generate_traj(stance_knee_perc, stance_knee_angle, 0, figStance, 'Knee');
        print_c_code(stance_knee_perc, coefs, ...
            'Last (half) step, knee, stance leg.', ...
            'fastStairsSLastStepKneeStancePercents', 'fastStairsSLastStepKneeStanceTrajCoefs');
        
        [coefs, knee_interp_swing] = generate_traj(swing_knee_perc, swing_knee_angle, 0, figSwing, 'Knee');
        print_c_code(swing_knee_perc, coefs, ...
            'Last (half) step, knee, swing leg.', ...
            'fastStairsSLastStepKneeSwingPercents', 'fastStairsSLastStepKneeSwingTrajCoefs');
        knee_interp = [knee_interp_stance; knee_interp_swing];
        
    case 'fast_stairs_small'
        knee_angle = [50 18  10 2.05  2 40 78 100 68 51 50]; % [deg].        
        knee_perc =  [ 0 20  30   49 50 60 65  75 90 97 100];  % [%].
        
        hip_angle = [50 18 10 2.05  2  18 35 65 63 51  50]; % [deg]. 0 23 43 73 82 65
        hip_perc =  [ 0 20 30   49 50  60 67 78 90 97 100]; % [%].

        close all;
        hFig = figure('Name', 'fast stairs small trajectories');
        figDisplay = figure('Name', 'Visualisation of the trajectories');
        
        % Hip.
        [coefs, hip_interp] = generate_traj(hip_perc, hip_angle, 1, hFig, 'Hip');
        print_c_code(hip_perc, coefs, 'Hip.', ...
            'fastStairsSStridePercentsHip', 'fastStairsSTrajCoefsHip');
        
        % Knee.
        [coefs, knee_interp] = generate_traj(knee_perc, knee_angle, 1, hFig, 'Knee');
        print_c_code(knee_perc, coefs, 'Knee.', ...
            'fastStairsSStridePercentsKnee', 'fastStairsSTrajCoefsKnee');
        
        
    case 'stairs_large_first_step'
        stance_knee_angle = [0 1 1.5 2 2 6 12]; % [deg].
        stance_knee_perc = [0 5 10 20 28 40 50]; % [%].
        
        stance_hip_angle = [0 0 0 0 1 2 3]; % [deg].
        stance_hip_perc = [0 8 13 20 30 40 50]; % [%].

        swing_knee_angle = [0 45 76 105 80 65]; % [deg].
        swing_knee_perc = [50 60 65 75 90 100]-50; % [%].

        swing_hip_angle = [0 23 43 73 82 65]; % [deg].
        swing_hip_perc = [50 60 69 78 90 100]-50; % [%].
        
        %
        close all;
        figStance = figure('Name', 'Stairs large first step stance trajectories');
        figSwing = figure('Name', 'Stairs large first step swing trajectories');
        figDisplay = figure('Name', 'Visualisation of the trajectories');
        
        % Hip.
        [coefs, hip_interp_stance] = generate_traj(stance_hip_perc, stance_hip_angle, 0, figStance, 'Hip');
        print_c_code(stance_hip_perc, coefs, ...
            'First (half) step, hip, stance leg.', ...
            'stairsLFirstStepHipStancePercents', 'stairsLFirstStepHipStanceTrajCoefs');
        
        [coefs, hip_interp_swing] = generate_traj(swing_hip_perc, swing_hip_angle, 0, figSwing, 'Hip');
        print_c_code(swing_hip_perc, coefs, ...
            'First (half) step, hip, swing leg.', ...
            'stairsLFirstStepHipSwingPercents', 'stairsLFirstStepHipSwingTrajCoefs');
        
        hip_interp = [hip_interp_stance; hip_interp_swing];
        
        % Knee.
        [coefs, knee_interp_stance] = generate_traj(stance_knee_perc, stance_knee_angle, 0, figStance, 'Knee');
        print_c_code(stance_knee_perc, coefs, ...
            'First (half) step, knee, stance leg.', ...
            'stairsLFirstStepKneeStancePercents', 'stairsLFirstStepKneeStanceTrajCoefs');
        
        [coefs, knee_interp_swing] = generate_traj(swing_knee_perc, swing_knee_angle, 0, figSwing, 'Knee');
        print_c_code(swing_knee_perc, coefs, ...
            'First (half) step, knee, swing leg.', ...
            'stairsLFirstStepKneeSwingPercents', 'stairsLFirstStepKneeSwingTrajCoefs');
        
        knee_interp = [knee_interp_stance; knee_interp_swing];
        
    case 'stairs_large_second_step'
        stance_knee_angle = [65 50 38 23 15 7 1 0]; % [deg].
        stance_knee_perc = [0 5 10 20 28 36 47 50]; % [%].
        
        stance_hip_angle = [65 46 28 16 8 1 0]; % [deg].
        stance_hip_perc = [0 6 15 22 30 40 50]; % [%].
        
        swing_knee_angle = [12 60 90 80 40 0]; % [deg].
        swing_knee_perc = [50 60 70 77 90 100]-50; % [%].
        
        swing_hip_angle = [3 20 38 43 25 0]; % [deg].
        swing_hip_perc = [50 60 67 78 90 100]-50; % [%].
        
        %
        close all;
        figStance = figure('Name', 'Stairs large second step stance trajectories');
        figSwing = figure('Name', 'Stairs large second step swing trajectories');
        figDisplay = figure('Name', 'Visualisation of the trajectories');
        
        % Hip.
        [coefs, hip_interp_stance] = generate_traj(stance_hip_perc, stance_hip_angle, 0, figStance, 'Hip');
        print_c_code(stance_hip_perc, coefs, ...
            'Second (half) step, hip, stance leg.', ...
            'stairsLSecondStepHipStancePercents', 'stairsLSecondStepHipStanceTrajCoefs');
        
        [coefs, hip_interp_swing] = generate_traj(swing_hip_perc, swing_hip_angle, 0, figSwing, 'Hip');
        print_c_code(swing_hip_perc, coefs, ...
            'Second (half) step, hip, swing leg.', ...
            'stairsLSecondStepHipSwingPercents', 'stairsLSecondStepHipSwingTrajCoefs');
        
        hip_interp = [hip_interp_stance; hip_interp_swing];
        
        % Knee.
        [coefs, knee_interp_stance] = generate_traj(stance_knee_perc, stance_knee_angle, 0, figStance, 'Knee');
        print_c_code(stance_knee_perc, coefs, ...
            'Second (half) step, knee, stance leg.', ...
            'stairsLSecondStepKneeStancePercents', 'stairsLSecondStepKneeStanceTrajCoefs');
        
        [coefs, knee_interp_swing] = generate_traj(swing_knee_perc, swing_knee_angle, 0, figSwing, 'Knee');
        print_c_code(swing_knee_perc, coefs, ...
            'Second (half) step, knee, swing leg.', ...
            'stairsLSecondStepKneeSwingPercents', 'stairsLSecondStepKneeSwingTrajCoefs');
        knee_interp = [knee_interp_stance; knee_interp_swing];
        
    case 'fast_stairs_large'
        knee_angle = [65 55 38 20 10 5 12 50 78 100 85 68 65]; %[0 45 76 105 80 65]; % [deg].        
        knee_perc = [0 4 10 20 28 36 50 60 65 75 90 97 100];  %[%].[50 60 65 75 90 100]
        
        hip_angle = [65 50 28 16 8 2 4 18 40 73 82 67 65]; % [deg]. 0 23 43 73 82 65
        hip_perc = [0 6 15 22 30 40 50 60 67 78 90 97 100]; % [%].

        close all;
        hFig = figure('Name', 'fast stairs large trajectories');
        figDisplay = figure('Name', 'Visualisation of the trajectories');
        
        % Hip.
        [coefs, hip_interp] = generate_traj(hip_perc, hip_angle, 1, hFig, 'Hip');
        print_c_code(hip_perc, coefs, 'Hip.', ...
            'fastStairsLStridePercentsHip', 'fastStairsLTrajCoefsHip');
        
        % Knee.
        [coefs, knee_interp] = generate_traj(knee_perc, knee_angle, 1, hFig, 'Knee');
        print_c_code(knee_perc, coefs, 'Knee.', ...
            'fastStairsLStridePercentsKnee', 'fastStairsLTrajCoefsKnee');
        
    case 'stones_first_step'
        stance_knee_angle = [2 2 2 2 2 2 2]; % [deg].
        stance_knee_perc = [0 10 15 20 30 40 50]; % [%].
                
        stance_hip_angle = [0 -1 -2 -4 -6 -8 -10]; % [deg].
        stance_hip_perc = [0 8 13 20 30 40 50]; % [%].
        
        swing_knee_angle = [0 25 70 60 45 40]; % [deg].
        swing_knee_perc = [50 65 75 85 95 100]-50; % [%].
        
        swing_hip_angle = [0 15 45 55 45 40]; % [deg].
        swing_hip_perc = [50 65 75 85 95 100]-50; % [%].

        %%       Old        
%         stance_knee_angle = [2 2 2 2 2 2 2]; % [deg].
%         stance_knee_perc = [0 10 15 20 30 40 50]; % [%].
%                 
%         stance_hip_angle = [0 -1 -2 -4 -6 -8 -10]; % [deg].
%         stance_hip_perc = [0 8 13 20 30 40 50]; % [%].
%         
%         swing_knee_angle = [0 30 60 50 35 25]; % [deg].
%         swing_knee_perc = [50 60 70 80 90 100]-50; % [%].
%         
%         swing_hip_angle = [0 15 35 40 35 25]; % [deg].
%         swing_hip_perc = [50 60 70 80 90 100]-50; % [%].

        %
        close all;
        figStance = figure('Name', 'Stones first step stance trajectories');
        figSwing = figure('Name', 'Stones first step swing trajectories');
        figDisplay = figure('Name', 'Visualisation of the trajectories');
        
        % Hip.
        [coefs, hip_interp_stance] = generate_traj(stance_hip_perc, stance_hip_angle, 0, figStance, 'Hip');
        print_c_code(stance_hip_perc, coefs, ...
            'First (half) step, hip, stance leg.', ...
            'stairsFirstStepHipStancePercents', 'stairsFirstStepHipStanceTrajCoefs');
        
        [coefs, hip_interp_swing] = generate_traj(swing_hip_perc, swing_hip_angle, 0, figSwing, 'Hip');
        print_c_code(swing_hip_perc, coefs, ...
            'First (half) step, hip, swing leg.', ...
            'stairsFirstStepHipSwingPercents', 'stairsFirstStepHipSwingTrajCoefs');
        
        hip_interp = [hip_interp_stance; hip_interp_swing];
        
        % Knee.
        [coefs, knee_interp_stance] = generate_traj(stance_knee_perc, stance_knee_angle, 0, figStance, 'Knee');
        print_c_code(stance_knee_perc, coefs, ...
            'First (half) step, knee, stance leg.', ...
            'stairsFirstStepKneeStancePercents', 'stairsFirstStepKneeStanceTrajCoefs');
        
        [coefs, knee_interp_swing] = generate_traj(swing_knee_perc, swing_knee_angle, 0, figSwing, 'Knee');
        print_c_code(swing_knee_perc, coefs, ...
            'First (half) step, knee, swing leg.', ...
            'stairsFirstStepKneeSwingPercents', 'stairsFirstStepKneeSwingTrajCoefs');
        
        
        knee_interp = [knee_interp_stance; knee_interp_swing];
        
    case 'slope'
            knee_angle = [15 11  4  2  8 65 80 53 25 15]; % [deg].
            knee_perc  = [ 0 10 30 50 55 65 75 87 92 100]; % [%].
            
            hip_angle = [30 10  0  0  6 15 30 48 48 33 30]; % [deg].
            hip_perc  = [ 0 20 45 50 60 64 68 74 86 93 100]; % [%].   
             
           % close all;
            hFig = figure('Name', 'Slope trajectories');
            figDisplay = figure('Name', 'Visualisation of the trajectories');

            % Hip.
            [coefs, hip_interp] = generate_traj(hip_perc, hip_angle, 1, hFig, 'Hip');
            print_c_code(hip_perc, coefs, 'Hip.', ...
                'slopeStridePercentsHip', 'slopeTrajCoefsHip');

            % Knee.
            [coefs, knee_interp] = generate_traj(knee_perc, knee_angle, 1, hFig, 'Knee');
            print_c_code(knee_perc, coefs, 'Knee.', ...
                'slopeStridePercentsKnee', 'slopeTrajCoefsKnee');
            
   case 'slope_first_step'
        stance_knee_angle = [0 2]; % [deg].
        stance_knee_perc = [0 50]; % [%].

        stance_hip_angle = [0 0]; % [deg].
        stance_hip_perc = [0 50]; % [%].
        
        swing_knee_angle = [0 25 50 65 55 42 35 15]; % [deg].
        swing_knee_perc = [50 60 70 80 87 92 95 100]-50; % [%].
        
        swing_hip_angle = [0 8 20 30 45 45 38 35 30]; % [deg].
        swing_hip_perc = [50 55 60 65 75 85 92 95 100]-50; % [%].
% OLD
%         stance_knee_angle = [0 3.65 3.45 4.17 6.87 10.00 5.1]; % [deg].
%         stance_knee_perc = [0 10 15 20 30 40 50]; % [%].
% 
%         stance_hip_angle = [0 -1 -2 -3 -4 -4.1 -4.38 -3.25]; % [deg].
%         stance_hip_perc = [0 10 15 20 30 36 40 50]; % [%].
%         
%         swing_knee_angle = [0 25 50 65 55 35 27 23]; % [deg].
%         swing_knee_perc = [50 60 70 80 87 92 95 100]-50; % [%].
%         
%         swing_hip_angle = [0 6 15 30 45 45 40 35 30]; % [deg].
%         swing_hip_perc = [50 55 60 65 75 85 92 95 100]-50; % [%].
        %
        close all;
        figStance = figure('Name', 'Slope gait first step stance trajectories');
        figSwing = figure('Name', 'Slope gait first step swing trajectories');
        figDisplay = figure('Name', 'Visualisation of the trajectories');

        % Hip.
        [coefs, hip_interp_stance] = generate_traj(stance_hip_perc, stance_hip_angle, 0, figStance, 'Hip');
        print_c_code(stance_hip_perc, coefs, ...
                     'First (half step), hip, stance leg.', ...
                     'slopeGaitFirstStepHipStancePercents', 'slopeGaitFirstStepHipStanceTrajCoefs');
        
        
        [coefs, hip_interp_swing] = generate_traj(swing_hip_perc, swing_hip_angle, 0, figSwing, 'Hip');
        print_c_code(swing_hip_perc, coefs, ...
                     'First (half step), hip, swing leg.', ...
                     'slopeGaitFirstStepHipSwingPercents', 'slopeGaitFirstStepHipSwingTrajCoefs');
        
        hip_interp = [hip_interp_stance; hip_interp_swing];

        % Knee.
       [coefs, knee_interp_stance] = generate_traj(stance_knee_perc, stance_knee_angle, 0, figStance, 'Knee');
        print_c_code(stance_knee_perc, coefs, ...
                     'First (half step), knee, stance leg.', ...
                     'slopeGaitFirstStepKneeStancePercents', 'slopeGaitFirstStepKneeStanceTrajCoefs');
                 
        [coefs, knee_interp_swing] = generate_traj(swing_knee_perc, swing_knee_angle, 0, figSwing, 'Knee');
        print_c_code(swing_knee_perc, coefs, ...
                     'First (half step), knee, swing leg.', ...
                     'slopeGaitFirstStepKneeSwingPercents', 'slopeGaitFirstStepKneeSwingTrajCoefs');
                 
        knee_interp = [knee_interp_stance; knee_interp_swing];
        
        
    case 'platform_first_step_from_fast_gait'
        stance_knee_angle = [2 2]; % [deg].
        stance_knee_perc = [0 50]; % [%].
                
        stance_hip_angle = [10 -7]; % [deg].
        stance_hip_perc = [0 50]; % [%].
        
        swing_knee_angle = [8 50 70 70 18 10]; % [deg].
        swing_knee_perc =  [0 15 20 30 42 50]; % [%].
        
        swing_hip_angle = [-9.17 4 17 30 35 25 25]; % [deg].
        swing_hip_perc =  [   0 10 18 25 35 45 50]; % [%].
        
        %
        close all;
        figStance = figure('Name', 'Plaform first step FFG stance trajectories');
        figSwing = figure('Name', 'Plaform first step FFG swing trajectories');
        figDisplay = figure('Name', 'Visualisation of the trajectories');
        
        % Hip.
        [coefs, hip_interp_stance] = generate_traj(stance_hip_perc, stance_hip_angle, 0, figStance, 'Hip');
        print_c_code(stance_hip_perc, coefs, ...
            'First (half step), hip, stance leg.', ...
            'platformFirstStepFFGHipStancePercents', 'platformFirstStepFFGHipStanceTrajCoefs');
        
        [coefs, hip_interp_swing] = generate_traj(swing_hip_perc, swing_hip_angle, 0, figSwing, 'Hip');
        print_c_code(swing_hip_perc, coefs, ...
            'First (half step), hip, swing leg.', ...
            'platformFirstStepFFGHipSwingPercents', 'platformFirstStepFFGHipSwingTrajCoefs');
        
        hip_interp = [hip_interp_stance; hip_interp_swing];
        
        % Knee.
        [coefs, knee_interp_stance] = generate_traj(stance_knee_perc, stance_knee_angle, 0, figStance, 'Knee');
        print_c_code(stance_knee_perc, coefs, ...
            'First (half step), knee, stance leg.', ...
            'platformFirstStepFFGKneeStancePercents', 'platformFirstStepFFGKneeStanceTrajCoefs');
        
        [coefs, knee_interp_swing] = generate_traj(swing_knee_perc, swing_knee_angle, 0, figSwing, 'Knee');
        print_c_code(swing_knee_perc, coefs, ...
            'First (half step), knee, swing leg.', ...
            'platformFirstStepFFGKneeSwingPercents', 'platformFirstStepFFGKneeSwingTrajCoefs');
        
        knee_interp = [knee_interp_stance; knee_interp_swing];
       

    case 'platform_first_step'
        stance_knee_angle = [0 2]; % [deg].
        stance_knee_perc = [0 50]; % [%].
                
        stance_hip_angle = [0 -2 -6.5 -7]; % [deg].
        stance_hip_perc = [0 25 45 50]; % [%].
        
        swing_knee_angle = [0 25 70 70 35 10]; % [deg].
        swing_knee_perc = [ 0 15 25 35 45 50]; % [%].
        
        swing_hip_angle = [0 15 50 55 40 25]; % [deg].
        swing_hip_perc = [ 0 15 25 35 45 50]; % [%].
        
        %
        close all;
        figStance = figure('Name', 'Plaform first step stance trajectories');
        figSwing = figure('Name', 'Plaform first step swing trajectories');
        figDisplay = figure('Name', 'Visualisation of the trajectories');
        
        % Hip.
        [coefs, hip_interp_stance] = generate_traj(stance_hip_perc, stance_hip_angle, 0, figStance, 'Hip');
        print_c_code(stance_hip_perc, coefs, ...
            'First (half step), hip, stance leg.', ...
            'platformFirstStepHipStancePercents', 'platformFirstStepHipStanceTrajCoefs');
        
        [coefs, hip_interp_swing] = generate_traj(swing_hip_perc, swing_hip_angle, 0, figSwing, 'Hip');
        print_c_code(swing_hip_perc, coefs, ...
            'First (half step), hip, swing leg.', ...
            'platformFirstStepHipSwingPercents', 'platformFirstStepHipSwingTrajCoefs');
        
        hip_interp = [hip_interp_stance; hip_interp_swing];
        
        % Knee.
        [coefs, knee_interp_stance] = generate_traj(stance_knee_perc, stance_knee_angle, 0, figStance, 'Knee');
        print_c_code(stance_knee_perc, coefs, ...
            'First (half step), knee, stance leg.', ...
            'platformFirstStepKneeStancePercents', 'platformFirstStepKneeStanceTrajCoefs');
        
        [coefs, knee_interp_swing] = generate_traj(swing_knee_perc, swing_knee_angle, 0, figSwing, 'Knee');
        print_c_code(swing_knee_perc, coefs, ...
            'First (half step), knee, swing leg.', ...
            'platformFirstStepKneeSwingPercents', 'platformFirstStepKneeSwingTrajCoefs');
        
        knee_interp = [knee_interp_stance; knee_interp_swing];
        
    case 'platform_second_step'
        stance_knee_angle = [10 8]; % [deg].
        stance_knee_perc = [0 50]; % [%].
                
        stance_hip_angle = [25 -9.17]; % [deg].
        stance_hip_perc = [0 50]; % [%].
        
        swing_knee_angle = [2 50 92 15 2]; % [deg].
        swing_knee_perc = [50 60 77 95 100]-50; % [%].
        
        swing_hip_angle = [-7 -5 40 20 10]; % [deg].
        swing_hip_perc =  [50 52 80 95 100]-50; % [%].
        
        %
        close all;
        figStance = figure('Name', 'Plaform second step stance trajectories');
        figSwing = figure('Name', 'Plaform second step swing trajectories');
        figDisplay = figure('Name', 'Visualisation of the trajectories');
        
        % Hip.
        [coefs, hip_interp_stance] = generate_traj(stance_hip_perc, stance_hip_angle, 0, figStance, 'Hip');
        print_c_code(stance_hip_perc, coefs, ...
            'Second (half step), hip, stance leg.', ...
            'platformSecondStepHipStancePercents', 'platformSecondStepHipStanceTrajCoefs');
        
        [coefs, hip_interp_swing] = generate_traj(swing_hip_perc, swing_hip_angle, 0, figSwing, 'Hip');
        print_c_code(swing_hip_perc, coefs, ...
            'Second (half step), hip, swing leg.', ...
            'platformSecondStepHipSwingPercents', 'platformSecondStepHipSwingTrajCoefs');
        
        hip_interp = [hip_interp_stance; hip_interp_swing];
        
        % Knee.
        [coefs, knee_interp_stance] = generate_traj(stance_knee_perc, stance_knee_angle, 0, figStance, 'Knee');
        print_c_code(stance_knee_perc, coefs, ...
            'Second (half step), knee, stance leg.', ...
            'platformSecondStepKneeStancePercents', 'platformSecondStepKneeStanceTrajCoefs');
        
        [coefs, knee_interp_swing] = generate_traj(swing_knee_perc, swing_knee_angle, 0, figSwing, 'Knee');
        print_c_code(swing_knee_perc, coefs, ...
            'Second (half step), knee, swing leg.', ...
            'platformSecondStepKneeSwingPercents', 'platformSecondStepKneeSwingTrajCoefs');
        
        knee_interp = [knee_interp_stance; knee_interp_swing];
end


display_traj(hip_interp, knee_interp, mode,figDisplay, 'Stick Figure')


percent = linspace(0, 100, 1000);
hip = hip_interp;
knee = knee_interp;

end


function [coefs, interp] = generate_traj(stride_perc, joint_angle, cyclic, ...
    hFig, name)

%% Fit.
[xData, yData] = prepareCurveData(stride_perc, joint_angle);

if cyclic
    xData = [xData(1:end-1)-100
        xData(1:end-1)
        xData(1:end-1)+100];
    yData = repmat(yData(1:end-1), 3, 1);
else
    xData = [xData(1:end-1)-xData(end)
        xData(1:end-1)
        xData(1:end)+xData(end)];
    yData = [ones(length(stride_perc)-1,1)*yData(1)
        yData(1:end-1)
        ones(length(stride_perc),1)*yData(end)];
end

[fitresult, ~] = fit(xData, yData, 'splineinterp');


%% Plot.
figure(hFig);
perc_interp = linspace(0, 100, 1000);
plot(perc_interp, fitresult(perc_interp)); % Interpolated function.
hold('on');
plot(stride_perc, joint_angle, '*'); % Original points.

% le = legend;
% legend([name ' (interpolated) [deg]'], ...
%        [name ' (original points) [deg]']);

xlabel('Gait stride [%]');
ylabel('Joint angle [deg]');

%% Extract the relevant coefficients.
len = length(stride_perc);
coefs = fitresult.p.coefs(len:len+len-2, :);

interp = fitresult(perc_interp);

newName = strcat('mode ', name);
S.(newName) = interp;
save(name, '-struct', 'S')

% %% Test interpolation and plot the result.
% stride_perc_interp = linspace(0, 100, 1000);
% hip_angle_interp = zeros(size(stride_perc_interp));
%
% for i=1:length(stride_perc_interp)
%     [j,~] = get_bounding_indices(stride_perc, stride_perc_interp(i));
%
%     hip_angle_interp(i) = polyval(coefs(j,:), stride_perc_interp(i)-stride_perc(j));
% end
%
% prevFig = gcf;
% figure('Name', 'Interpolation formula test');
% plot(stride_perc, joint_angle, '*', stride_perc_interp, hip_angle_interp);
% figure(prevFig);

end

function print_c_code(stride_perc, coefs, var_comment, stride_var_name, ...
    coefs_var_name)

fprintf(['// ' var_comment '\nconst std::vector<std::vector<float>> ' ...
    coefs_var_name ' = {\n']);

for i=1:size(coefs, 1)
    fprintf('\t{ ');
    for j=1:size(coefs,2)-1
        fprintf('%ff, ', coefs(i,j));
    end
    
    if i < size(coefs, 1)
        fprintf('%ff },\n', coefs(i,end));
    else
        fprintf('%ff }\n', coefs(i,end));
    end
end

fprintf('};\n\n');

fprintf(['const std::vector<float> ' stride_var_name ' = { ']);
for i=1:length(stride_perc)
    if i < length(stride_perc)
        fprintf('%ff, ', stride_perc(i));
    else
        fprintf('%ff };\n', stride_perc(i));
    end
end
fprintf('\n\n');

end

function [prevIndex, nextIndex] = get_bounding_indices(array, value)

prevIndex = 0;
nextIndex = 0;

for i=1:length(array)-1
    if array(i) <= value && array(i+1) >= value
        prevIndex = i;
        nextIndex = i+1;
        break;
    end
end

if prevIndex == 0 && nextIndex == 0
    error(['Error, ' num2str(value) '\n' mat2str(array)]);
end
end


function display_traj(hip_interp, knee_interp,mode,hfig, name)

dsmp_hip_interp = downsample(hip_interp,10);
dsmp_knee_interp = downsample(knee_interp,10);

%% Calculate foot locus

L_thigh = 360; % [mm] from hip to knee
L_shank = 490; % [mm] from knee to ground
L_trunk = 550; % [mm] from hip to head
H_sole = 20; % [mm] heigth of the wooden sole
L_foot = 240;
hor_offset = 0;

if strcmp(mode, 'stairs_last_step') || strcmp(mode, 'stairs_first_step') || strcmp(mode, 'fast_stairs') || ...
   strcmp(mode, 'stairs_small_last_step') || strcmp(mode, 'stairs_small_first_step') || strcmp(mode, 'fast_stairs_small')
    vert_offset = 170; % [mm]
    L_stair = 280; % [mm]
    hor_offset = 240;
elseif strcmp(mode, 'slope_first_step')
    vert_offset = tand(20)* hor_offset;
elseif strcmp(mode,'slope')
    hor_offset = 700;
    vert_offset = tand(20)* hor_offset;    
elseif strcmp(mode, 'first_step_backward_slope')
    vert_offset = tand(20)* hor_offset; % [mm]
else
    vert_offset = 0;
end

%% Stance leg

heel_pos_stance.X = -cosd(dsmp_knee_interp-dsmp_hip_interp) * L_foot;
heel_pos_stance.Y = sind(dsmp_knee_interp-dsmp_hip_interp) * L_foot;

knee_pos_stance.X =  heel_pos_stance.X + cosd(90+dsmp_hip_interp-dsmp_knee_interp) * L_shank;
knee_pos_stance.Y = heel_pos_stance.Y + sind(90+dsmp_hip_interp-dsmp_knee_interp) * L_shank;

hip_pos_stance.X = knee_pos_stance.X - cosd(90-dsmp_hip_interp) * L_thigh;
hip_pos_stance.Y = knee_pos_stance.Y + sind(90-dsmp_hip_interp) * L_thigh;

%% Swing Leg
knee_pos_swing.X = sind(dsmp_hip_interp) * L_thigh;
knee_pos_swing.Y = -cosd(dsmp_hip_interp) * L_thigh;

heel_pos_swing.X = knee_pos_swing.X - sind(dsmp_knee_interp-dsmp_hip_interp) * L_shank;
heel_pos_swing.Y = knee_pos_swing.Y - cosd(dsmp_knee_interp-dsmp_hip_interp) * L_shank;

foottip_pos_swing.X = heel_pos_swing.X + cosd(dsmp_hip_interp-dsmp_knee_interp) * L_foot;
foottip_pos_swing.Y = heel_pos_swing.Y + sind(dsmp_hip_interp-dsmp_knee_interp) * L_foot;


% Display animated foot locus
figure(hfig)

pause

for i = 1:length(dsmp_hip_interp)/2-1
    
  
    legX_stance = [0 + hor_offset;
                   heel_pos_stance.X(i) + hor_offset;
                   knee_pos_stance.X(i) + hor_offset;
                   hip_pos_stance.X(i)  + hor_offset;
                   hip_pos_stance.X(i)  + hor_offset + sind(15)*L_trunk];
    
    legY_stance = [vert_offset + H_sole;        %170 is the height a stair [mm]
                   heel_pos_stance.Y(i) + vert_offset + H_sole;
                   knee_pos_stance.Y(i)+vert_offset + H_sole;
                   hip_pos_stance.Y(i)+vert_offset+ H_sole;
                   hip_pos_stance.Y(i)+vert_offset+ H_sole + L_trunk];
    
    
    legX_swing = [hip_pos_stance.X(i)   + hor_offset;
                  hip_pos_stance.X(i)   + knee_pos_swing.X(length(dsmp_hip_interp)/2+1+i) + hor_offset;
                  hip_pos_stance.X(i)   + heel_pos_swing.X(length(dsmp_hip_interp)/2+1+i) + hor_offset;
                  hip_pos_stance.X(i)   + foottip_pos_swing.X(length(dsmp_hip_interp)/2+1+i)  + hor_offset];
    
    legY_swing = [hip_pos_stance.Y(i)   + vert_offset + H_sole;             %170 is the height a stair [mm]
                  hip_pos_stance.Y(i)   + knee_pos_swing.Y(length(dsmp_hip_interp)/2+1+i)      + vert_offset + H_sole;
                  hip_pos_stance.Y(i)   + heel_pos_swing.Y(length(dsmp_hip_interp)/2+1+i)      + vert_offset + H_sole;
                  hip_pos_stance.Y(i)   + foottip_pos_swing.Y(length(dsmp_hip_interp)/2+1+i)   + vert_offset + H_sole];
    
    
    subplot(211);
%     subplot(133);
    hold off
    plot(legX_stance, legY_stance,'linewidth',4);
    hold on
    plot(legX_swing, legY_swing,'linewidth',4);
    
    plot(foottip_pos_swing.X(length(dsmp_hip_interp)/2+1:length(dsmp_hip_interp)/2+i)+hip_pos_stance.X(1:i) + hor_offset,...
        hip_pos_stance.Y(1:i) + foottip_pos_swing.Y(length(dsmp_hip_interp)/2+1:length(dsmp_hip_interp)/2+i)+vert_offset,'linewidth',2)
    plot([-600 700], [-30 -30], 'linewidth', 1.5)
    
    plot(heel_pos_swing.X(length(dsmp_hip_interp)/2+1:length(dsmp_hip_interp)/2+i)+hip_pos_stance.X(1:i) + hor_offset,...
        hip_pos_stance.Y(1:i) + heel_pos_swing.Y(length(dsmp_hip_interp)/2+1:length(dsmp_hip_interp)/2+i)+vert_offset,'linewidth',2)

    
    if strcmp(mode,'stairs_last_step') || strcmp(mode, 'stairs_first_step') || strcmp(mode, 'fast_stairs') || ...
       strcmp(mode, 'stairs_small_last_step') || strcmp(mode, 'stairs_small_first_step') || strcmp(mode, 'fast_stairs_small')
        plot([0 0],[-30 vert_offset],'k',[0 L_stair],[vert_offset vert_offset],'k',...
            [L_stair L_stair],[vert_offset 2*vert_offset],'k', [L_stair 2*L_stair],[2*vert_offset 2*vert_offset],'k','linewidth',2);
    elseif strcmp(mode,'slope') || strcmp(mode, 'first_step_backward_slope') || strcmp(mode, 'slope_first_step')
        plot([0 1000],[0 tand(20)*1000],'k','linewidth',2)
        plot([1000 1500], [tand(20)*1000 tand(20)*1000],'k','linewidth',2)
    end
    
%     ylim([-100 100]);
    set(gca,'xtick',[],'ytick',[])

    axis equal
    
   subplot(223);
%  subplot(131);
    hold off
    plot(1:i,dsmp_hip_interp(1:i),'linewidth',2);
    hold on
    plot(1:i,dsmp_knee_interp(1:i),'linewidth',2);
    xlim([1 50]);
    legend('Hip[deg]', 'Knee[deg]');
    title('STANCE')
    ylabel('Joint Angle[deg]')
    xlabel('Cycle [%]')
    
    subplot(224);
%   subplot(132);
    hold off
    plot(1:i,dsmp_hip_interp(length(dsmp_hip_interp)/2+1:length(dsmp_hip_interp)/2+i),'linewidth',2);
    hold on
    plot(1:i,dsmp_knee_interp(length(dsmp_hip_interp)/2+1:length(dsmp_hip_interp)/2+i),'linewidth',2);
    xlim([1 50]);
    title('SWING')
    xlabel('Cycle [%]')
    
    pause(0.02)
    
end

end

