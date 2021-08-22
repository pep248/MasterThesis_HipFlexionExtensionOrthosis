%% WKMOT_CURRLOGREAD
function wkmot_commutlogread(name)

close all;

%% Constants.
LOGS_DIR = '../MotorboardTester/build/logs/';
INCR_TABLE = [ 2,  2,  2,  2,  2,  2,  2, 2;
               2,  0,  2, +1,  2, -1,  2, 2;
               2,  2,  0, -1,  2,  2, +1, 2;
               2, -1, +1,  0,  2,  2,  2, 2;
               2,  2,  2,  2,  0, +1, -1, 2;
               2, +1,  2,  2, -1,  0,  2, 2;
               2,  2, -1,  2, +1,  2,  0, 2;
               2,  2,  2,  2,  2,  2,  2, 2];
c = constants();

%% Open the logfile.
if exist('name', 'var')
    filename = name;
else
    files = dir([LOGS_DIR 'commutlog_*.wkmotlog']);
    [~, index] = max([files.datenum]);
    filename = [LOGS_DIR files(index).name];
end

A = load(filename);

blockCommutationMode = 0;

%% Extract the data.
u = A(:,1);
v = A(:,2);
w = A(:,3);
boardAbsCurrent = A(:,4);
pwmVoltage = A(:,5);
hallsState = A(:,6);

times = linspace(0, length(u)*c.SAMPLING_PERIOD, length(u));

% %% Plot.
% plot(times, u, '.-', times, v, '.-', times, w, '.-', ...
%      times, pwmVoltage);
% legend('U [A]', 'V [A]', 'W [A]', 'Command voltage [V]');
% xlabel('Time [s]');

%% Display the three phases currents, and the overall motor current.
DISPLAY_COMMUT_GRID = 0;

if blockCommutationMode
    % Compute the spinning direction from the Hall states.
    spinDirections = zeros(length(u), 1);

    for i=2:length(spinDirections)

        if hallsState(i) > 7
            hallsState(i) = 0;
        end

        if hallsState(i-1) ~= hallsState(i)
            spinDirections(i) = (INCR_TABLE(hallsState(i-1)+1, hallsState(i)+1) == 1);
        else
            spinDirections(i) = spinDirections(i-1);
        end
    end

    % Compute the overall current from the phases currents.
    desktopAbsCurrents = zeros(length(u), 1);

    for i=1:length(desktopAbsCurrents)
        desktopAbsCurrents(i) = uvwToAbsCurrent1(u(i), v(i), w(i), hallsState(i), ...
                                          spinDirections(i));
    end
else
    magneticAngles = hallsState / 255 * 360;
    
    % Compute the motor speed.
    speed = [0; diff(magneticAngles)];
    jv = abs(speed) > 0.5 * 360; % Jumping values.
    speed(jv) = speed(jv) - sign(speed(jv)) * 360;
    speed = speed / c.SAMPLING_PERIOD / c.N_POLES_PAIRS / 360 * 60; % [RPM].
    
    for i=1:length(speed)
        if abs(speed(i)) > 20000
            speed(i) = speed(i-1);
        end
    end
    speed = smooth(speed, 100);
    
%     plot(times, speed);
    
    % Compute the overall current from the phases currents.
    desktopAbsCurrents = zeros(length(u), 1);
    
    for i=1:length(desktopAbsCurrents)
        desktopAbsCurrents(i) = uvwToAbsCurrentSine(u(i), v(i), w(i), magneticAngles(i), speed(i));
    end
end

% Compute the unsigned current for reference.
absUnsignedCurrent = zeros(length(u), 1);

for i=1:length(absUnsignedCurrent)
    absUnsignedCurrent(i) = uvwToAbsCurrentUnsigned(u(i), v(i), w(i));
end

% % Plot with optionnal commutation grid.
% figure('name', filename);
% if DISPLAY_COMMUT_GRID == 0
%     plot(times, u, '.-', times, v, '.-', times, w, '.-', ...
%          times, absCurrents, '*-');
%     legend('U [A]', 'V [A]', 'W [A]', 'Absolute current [A]');
% else
%     % Generate a grid, to separate the commutation states.
%     commutDelimiters = zeros(length(u), 1);
%     commutDelimiters(1) = 10;
% 
%     for i = 2:length(commutDelimiters)
%         if hallsState(i) ~= hallsState(i-1)
%             commutDelimiters(i) = -commutDelimiters(i-1);
%         else
%             commutDelimiters(i) = commutDelimiters(i-1);
%         end
%     end
%     plot(times, commutDelimiters, ...
%          times, u, '.-', times, v, '.-', times, w, '.-', ...
%          times, absCurrents, '*');
%     legend('', 'U [A]', 'V [A]', 'W [A]', 'Absolute current [A]');
% end

%%
figure;
desktopAbsCurrents = smooth(desktopAbsCurrents, 100);
absUnsignedCurrent = smooth(absUnsignedCurrent, 100);
boardAbsCurrent = smooth(boardAbsCurrent, 100);

[dCurrent, qCurrent, oCurrent] = parkTransform(u, v, w, magneticAngles);
dCurrent = smooth(dCurrent, 100);
qCurrent = smooth(qCurrent, 100);
oCurrent = smooth(oCurrent, 100);

ax1 = subplot(4, 1, 1);
plot(times, abs(desktopAbsCurrents), times, absUnsignedCurrent);
legend('MATLAB-computed signed current, rectified', 'MATLAB-computed unsigned current (ground truth)');

ax2 = subplot(4, 1, 2);
plot(times, desktopAbsCurrents, times, boardAbsCurrent);
legend('MATLAB-computed signed current',  'Board current / target current');

ax3 = subplot(4, 1, 3);
yyaxis('left');
plot(times, speed);
yyaxis('right');
plot(times, pwmVoltage);
legend('Speed [RPM]', 'Voltage [V]');

ax4 = subplot(4, 1, 4);
plot(times, dCurrent, times, qCurrent, times, oCurrent, ...
     times, absUnsignedCurrent, '--', times, desktopAbsCurrents, '--');
legend('d', 'q', 'o', 'old MATLAB unsigned', 'old MATLAB signed');

linkaxes([ax1 ax2 ax3 ax4], 'x');



figure;
plot(times, absUnsignedCurrent, times, sqrt(dCurrent.^2 + qCurrent.^2));
xlabel('Time [s]');
ylabel('Current [A]');
legend('MATLAB unsigned', 'd^2 + q^2');

   
% %%
% figure;
% ax1 = subplot(4, 1, 1:2);
% plot(times, u, '.-', times, v, '.-', times, w, '.-', ...
%      times, desktopAbsCurrents, '*', times, smooth(desktopAbsCurrents, 1000));
% legend('U', 'V', 'W', 'MATLAB-computed signed current', 'Smoothed MATLAB-computed signed current');
% xlabel('Time [s]');
% ylabel('Current [A]');
% 
% ax2 = subplot(4, 1, 3);
% plot(times, magneticAngles);
% 
% ax3 = subplot(4, 1, 4);
% plot(times, pwmVoltage);
% legend('Command voltage [V]');
% xlabel('Time [s]');
% 
% linkaxes([ax1 ax2 ax3], 'x');

%%
figure;

expectedSinAmp = 2;
expectedSinU = sind(magneticAngles + c.SINE_COMMUT_PHASE_U) * expectedSinAmp;
expectedSinV = sind(magneticAngles + c.SINE_COMMUT_PHASE_V) * expectedSinAmp;
expectedSinW = sind(magneticAngles + c.SINE_COMMUT_PHASE_W) * expectedSinAmp;

plot(times, expectedSinU, 'b.', times, expectedSinV, 'r.', times, expectedSinW, 'g.', ...
     times, u, 'b', times, v, 'r', times, w, 'g');
legend('Expected U', 'Expected V', 'Expected W', 'U', 'V', 'W');



% %% Compute the motor current.
% close;
% plot(times(1:10:end), current(1:10:end));
% 
% % Select the area of interest.
% [t,~] = ginput(2);
% 
% iSelection = current(times>t(1) & times<t(2));
% tSelection = times(times>t(1) & times<t(2));
% plot(tSelection, iSelection, '.-');
% 
% % Compute the motor current.
% fprintf('Motor current: %f A.\n', rms(iSelection));

%%
%save('currents_stm32.mat', 'times', 'u', 'v', 'w');

end

% function commutState = hallsState2CommutState(hallsState)
%     
% end

function absCurrent = uvwToAbsCurrent1(u, v, w, hallsState, spinCw)

switch hallsState
    case 1
        if spinCw
            absCurrent = u;
        else
            absCurrent = -w;
        end
    case 2
        if spinCw
            absCurrent = v;
        else
            absCurrent = -u;
        end
    case 3
        if spinCw
            absCurrent = -w;
        else
            absCurrent = v;
        end
    case 4
        if spinCw
            absCurrent = w;
        else
            absCurrent = -v;
        end
    case 5
        if spinCw
            absCurrent = -v;
        else
            absCurrent = u;
        end
    case 6
        if spinCw
            absCurrent = -u;
        else
            absCurrent = w;
        end
    otherwise % Error.
        absCurrent = NaN;
end

end

% function absCurrent = uvwToAbsCurrent2(u, v, w, hallsState)
% 
% switch hallsState
%     case 1
%         absCurrent = (u - w) / 2;
%     case 2
%         absCurrent = (v - u) / 2;
%     case 3
%         absCurrent = (v - w) / 2;
%     case 4
%         absCurrent = (w - v) / 2;
%     case 5
%         absCurrent = (u - v) / 2;
%     case 6
%         absCurrent = (w - u) / 2;
%     otherwise % Error.
%         absCurrent = NaN;
% end
% 
% end

function absCurrent = uvwToAbsCurrentSine(u, v, w, magneticAngle, speed)

c = constants();

% 
SIN_MIN = 0.6;

divider = 0;
absCurrent = 0;

%speedShift = speed * 0.003;
%speedShift = speed /1000 * 90;
%speedShift = speed /1900 * 90;
%speedShift = speed /3560 * 50;
speedShift = 0;

% speedShift = sign(speed) * interp1([0 4000 4500], ...
%                                    [0 90 45], abs(speed));

% if abs(speed) < 4000
%     speedShift = speed /3560 * 50;
% elseif abs(speed) < 4500
%     speedShift = 0;
% else
%     speedShift = -45 * sign(speed);
% end

sinU = sind(magneticAngle + c.SINE_COMMUT_PHASE_U - speedShift);
sinV = sind(magneticAngle + c.SINE_COMMUT_PHASE_V - speedShift);
sinW = sind(magneticAngle + c.SINE_COMMUT_PHASE_W - speedShift);

if abs(sinU) > SIN_MIN
    absCurrent = absCurrent + u / sinU;
    divider = divider + 1;
end

if abs(sinV) > SIN_MIN
    absCurrent = absCurrent + v / sinV;
    divider = divider + 1;
end

if abs(sinW) > SIN_MIN
    absCurrent = absCurrent + w / sinW;
    divider = divider + 1;
end

if divider >= 1
    absCurrent = absCurrent / divider;
else
    absCurrent = 0;
end

end

function absCurrent = uvwToAbsCurrentUnsigned(u, v, w)

    uvw = [u v w];
    %absCurrent = sum(uvw(uvw > 0));
    absCurrent = sum(abs(uvw)) / 2;
end

function c = constants()

c.SAMPLING_PERIOD = 0.0001; % [s].

% c.N_POLES_PAIRS = 7;                % Autonomyo's EC-i motor.
% c.SINE_COMMUT_PHASE_U = (30 + 0);   %
% c.SINE_COMMUT_PHASE_V = (30 - 120); %
% c.SINE_COMMUT_PHASE_W = (30 - 240); %

% c.N_POLES_PAIRS = 7;                  % Other EC-i motor.
% mc.SINE_COMMUT_PHASE_U = (127 + 0);   %
% mc.SINE_COMMUT_PHASE_V = (127 - 120); %
% mc.SINE_COMMUT_PHASE_W = (127 - 240); %

% c.N_POLES_PAIRS = 2;                 % Walki's EC-4pole motor.
% c.SINE_COMMUT_PHASE_U = (250 + 0);   %
% c.SINE_COMMUT_PHASE_V = (250 - 120); %
% c.SINE_COMMUT_PHASE_W = (250 - 240); %

% c.N_POLES_PAIRS = 5;           % TWIICE's SBZ-5612 motor.
% c.SINE_COMMUT_PHASE_U = (240); %
% c.SINE_COMMUT_PHASE_V = (120); %
% c.SINE_COMMUT_PHASE_W = (0);   %

c.N_POLES_PAIRS = 10;           % TWIICE 2 MMT motor.
c.SINE_COMMUT_PHASE_U = (244+0); %
c.SINE_COMMUT_PHASE_V = (244+120); %
c.SINE_COMMUT_PHASE_W = (244+240);   %

end

function [d, q, o] = parkTransform(u, v, w, theta)

d = zeros(size(theta));
q = zeros(size(theta));
o = zeros(size(theta));

for i=1:length(theta)
    [ds, qs, qo] = parkTransformMono(u(i), v(i), w(i), theta(i));
    d(i) = ds;
    q(i) = qs;
    o(i) = qo;
end

end

function [d, q, o] = parkTransformMono(u, v, w, theta)

c = constants();

thetaU = theta + c.SINE_COMMUT_PHASE_U;
thetaV = theta + c.SINE_COMMUT_PHASE_V;
thetaW = theta + c.SINE_COMMUT_PHASE_W;

dq = 2/3 * [cosd(thetaU) cosd(thetaV) cosd(thetaW);
            sind(thetaU) sind(thetaV) sind(thetaW);
            0.5          0.5          0.5          ] ...
         * [u; v; w];
     
d = dq(1);
q = dq(2);
o = dq(3);

end