%% WKMOT_CURRLOGREAD
function wkmot_currlogread(name)

%% Constants.
LOGS_DIR = '../MotorboardTester/build/logs/';
SAMPLING_PERIOD = 0.0001; % [s].

%% Open the logfile.
if exist('name', 'var')
    filename = name;
else
    files = dir([LOGS_DIR 'curlog_*.wkmotlog']);
    [~, index] = max([files.datenum]);
    filename = [LOGS_DIR files(index).name];
end

A = load(filename);

%% Extract the data.
current = A(:,1);
target = A(:,2);
command = A(:,3);

times = linspace(0, length(current)*SAMPLING_PERIOD, length(current));

%% TEMP
% current = smooth(current, 10);
% command = smooth(command, 10);

%% Remove outliers.
%current(abs(current) > 1e3) = nan;
%target(abs(target) > 1e6) = nan;

%% Filter.
%current = smooth(current, 1);

%% Test.
% err = target - current;
% 
% errD = [0 diff(err)'];
% 
% errFilt = zeros(1, length(current));
% presentWeight = 0.1;
% mean = 0.0;
% for i=2:length(err)
%     mean = err(i) * presentWeight + mean * (1.0 - presentWeight);
%     errFilt(i) = mean;
% end
% 
% errFiltD = [0 diff(errFilt)];

%% Plot.
figure('name', filename);
ax1 = subplot(2, 1, 1);
plot(times, [current'; target'], '.-');
xlabel('Time [s]');
ylabel('Current [A]');
legend('Current current [A]', 'Target current [A]');

ax2 = subplot(2, 1, 2);
plot(times, command, '.-');
ylabel('Command voltage [V]');

linkaxes([ax1 ax2], 'x');

%plot(times, current, times, target, times, command);

end