function wkvars = wkmain_logread(name, plotAll)
%WKMAIN_LOGREAD Load and optionally plot a Walki mainboard logile.

%% Constants.
LOGS_DIR = '../../RemoteControl/build/logs/';

%% Open the logfile.
if ~exist('plotAll', 'var')
    plotAll = 1;
end

if exist('name', 'var')
    filename = name;
else
    files = dir([LOGS_DIR 'log_*.wkl']);
    [~, index] = max([files.datenum]);
    filename = [LOGS_DIR files(index).name];
end

A = importdata(filename, ';', 1);
data = A.data;
headers = A.textdata;

% Add empty columns at the right, skipped by importdata.
data = [data nan(size(data,1), length(headers)-size(data,2))];

% Extract the variable names by removing the unit.
varNames = cell(1, length(headers));
for i=1:length(headers)
    varName = headers{i};
    varNames{i} = varName(1:find(varName == '[')-2);
end

%
wkvars.names = varNames;
wkvars.data = data;

%% Extract specific data and plot [reference only].
% times = getVar('timestamp', data, varNames); % [s].
% times = times - times(1);
% 
% loadCellA = getVar('sensors/left_foot/load_cells/a', data, varNames);
% loadCellB = getVar('sensors/left_foot/load_cells/b', data, varNames);
% 
% plot(times, loadCellA, times, loadCellB);

%% Plot all.
if plotAll
    times = getVar('timestamp', data, varNames); % [s].
    times = times - times(1);

    nVars = length(headers);
    nTimesteps = length(times);

    varsValues = [];
    varsLegends = [];

    for i=2:nVars
        if nnz(isnan(data(:,i))) < nTimesteps
            varsValues = [varsValues; data(:,i)']; %#ok<AGROW>
            varsLegends = [varsLegends headers(i)]; %#ok<AGROW>
        end
    end

    plot(times, varsValues);
    legend(varsLegends, 'Interpreter', 'none');
end

end

function a = getVar(varName, data, varNames)

    % Does the requested variable exist?
    varIndex = find(ismember(varNames, varName));
    
    if varIndex == 0
        error('loadWalkiLog:getVar', 'The requested var does not exist.');
    end
    
    % Extract the right column.
    a = data(:, varIndex);
end