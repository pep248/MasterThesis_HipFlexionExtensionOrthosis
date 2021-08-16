function spriint_calibrate_encoder(wkv)

%% Instructions.
% In WalkiBBB, set HD_INDEX_ANGLE and HIP_ANGLE_SENSOR_OFFSET to 0.0f, and
% HIP_ANGLE_SENSOR_GAIN to 1.0f.
% Do several turns with the SPRIINT actuator with no load, horizontally.
% Download the logfile, load it, and use wkv_gsubset() to keep only the
% relevant region. The first cycle should be discarded because the index
% line was not found yet.
% Run this script.
% 

%% Mechanical constants.
R = 0.0225; % [m].
h = 0.045; % [m].

hip_angle_f = @(gain, horz_offset, vert_offset, x) gain * atand(-R*sind(x+horz_offset)./(R*cosd(x+horz_offset)+h)) + vert_offset;

%% Extract the relevant data from the log.
x = wkv_get(wkv, 'controller/harmonic_drive_angle'); % [deg].
y = wkv_get(wkv, 'controller/hip_angle'); % [deg].
%y = smooth(y, 100);

%% Fit.
[xData, yData] = prepareCurveData( x, y );

% Set up fittype and options.
opts = fitoptions('Method', 'NonlinearLeastSquares');
opts.Display = 'Off';
opts.StartPoint = [1 0 0];

% Fit model to data.
[fitresult, gof] = fit(xData, yData, hip_angle_f, opts);

% Plot fit with data.
figure('Name', 'untitled fit 1');
h = plot( fitresult, xData, yData );
legend(h, 'y vs. x', 'untitled fit 1', 'Location', 'NorthEast', 'Interpreter', 'none');
% Label axes
xlabel('x', 'Interpreter', 'none');
ylabel('y', 'Interpreter', 'none');
grid on;

%%
plot(x, y, '.', x, fitresult(x));
fprintf('HIP_ANGLE_SENSOR_GAIN = %f\nHD_INDEX_ANGLE = %f°\nHIP_ANGLE_SENSOR_OFFSET = %f°\n', ...
        fitresult.gain, -fitresult.horz_offset, fitresult.vert_offset);
fprintf('Average RMS error: %f°.\n', gof.rmse);