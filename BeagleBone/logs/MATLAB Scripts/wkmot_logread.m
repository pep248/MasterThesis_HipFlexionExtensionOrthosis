%% WKMOT_LOGREAD
function wkmot_logread(name, jointMode)

%% Constants.
LOGS_DIR = '../MotorboardTester/build/logs/';
%LOGS_DIR = '../MotorboardTesterMatlab/logs/';

%% Open the logfile.
if exist('name', 'var')
    filename = name;
else
    files = dir([LOGS_DIR 'log_*.wkmotlog']);
    [~, index] = max([files.datenum]);
    filename = [LOGS_DIR files(index).name];
end

if ~exist('jointMode', 'var')
    jointMode = 1;
end

A = importdata(filename, ' ', 1);
A = A.data;

%% Extract the data.
times = (A(:,1) - A(1,1)) / 1000000; % [us] -> [s].

boardVoltage = A(:,2);
boardCurrent = A(:,3);

currentMotorPosA = A(:,4);
currentJointPosA = A(:,5);
targetPosA = A(:,6);
commandA = A(:,7); % [A].
currentA = A(:,8); % [A].

currentMotorPosB = A(:,9);
currentJointPosB = A(:,10);
targetPosB = A(:,11);
commandB = A(:,12); % [A].
currentB = A(:,13); % [A].

if jointMode
    currentPosA = currentJointPosA;
    currentPosB = currentJointPosB;
else
    currentPosA = currentMotorPosA;
    currentPosB = currentMotorPosB;
end

%% Remove outliers.
times(abs(times) > 1e6) = nan;

currentPosA(abs(currentPosA) > 1e6) = nan;
targetPosA(abs(targetPosA) > 1e6) = nan;
commandA(abs(commandA) > 1e6) = nan;
currentA(abs(currentA) > 1e6) = nan;

currentPosB(abs(currentPosB) > 1e6) = nan;
targetPosB(abs(targetPosB) > 1e6) = nan;
commandB(abs(commandB) > 1e6) = nan;
currentB(abs(currentB) > 1e6) = nan;

%% Plot.
plotMotor(times, currentPosA, targetPosA, commandA, currentA, ...
          'A', 0, 1);
plotMotor(times, currentPosB, targetPosB, commandB, currentB, ...
          'B', 0, 1);
   
figure('name', 'Board status');
plot(times, boardVoltage, '.-', times, boardCurrent, '.-');
xlabel('Time [s]');
legend('Board input voltage [V]', 'Board input current [A]');

end

function plotMotor(times, currentPos, targetPos, command, actualTorque, ...
                   motorDesignator, plotSpeedAndAcceleration, plotCurrent)

figure('name', ['Motor ' motorDesignator ' regulation']);

nGraphs = 1 + plotCurrent + 2 * plotSpeedAndAcceleration;
currentGraphIndex = 1;
axes = zeros(1, nGraphs);

% Position.
axes(currentGraphIndex) = subplot(nGraphs, 1, currentGraphIndex);
plot(times, currentPos, '.-', times, targetPos, '.-');
xlabel('Time [s]');
ylabel('Angle [deg]');
legend('Current angle', 'Target angle');
currentGraphIndex = currentGraphIndex + 1;

% Current.
if plotCurrent
    axes(currentGraphIndex) = subplot(nGraphs, 1, currentGraphIndex);
    plot(times, command, '.-', times, actualTorque, '.-');
    grid;
    xlabel('Time [s]');
    ylabel('Current [A]');
    legend('Target current', 'Actual current');
    currentGraphIndex = currentGraphIndex + 1;
end
   
% Speed and acceleration.
if plotSpeedAndAcceleration
    % Speed.
    axes(currentGraphIndex) = subplot(nGraphs, 1, currentGraphIndex);
    timeSpeed = times(1:end-1);
    speed = diff(smooth(currentPos, 10)) ./ diff(times);
    plot(timeSpeed, speed, '.-');
    xlabel('Time [s]');
    legend('Current speed [deg/s]');
    currentGraphIndex = currentGraphIndex + 1;

    % Acceleration.
    axes(currentGraphIndex) = subplot(nGraphs, 1, currentGraphIndex);
    timeAccel = timeSpeed(1:end-1);
    accel = diff(smooth(speed, 10)) ./ diff(timeSpeed);
    axes(currentGraphIndex) = subplot(3, 1, 3);
    plot(timeAccel, accel, '.-');
    xlabel('Time [s]');
    legend('Current acceleration [deg/s^2]');
    currentGraphIndex = currentGraphIndex + 1; %#ok<NASGU>
end

linkaxes(axes, 'x');

end