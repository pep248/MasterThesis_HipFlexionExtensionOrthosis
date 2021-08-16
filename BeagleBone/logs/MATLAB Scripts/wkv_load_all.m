function logs = wkv_load_all(directory)
%WKV_LOAD_ALL loads all the WKV logfiles in the selected directory.

%% Clean the given directory path.
directory(directory == '"') = [];

if directory(end) ~= '/' || directory(end) ~= '\'
    directory = [directory '/'];
end

%% Create a temporary working directory.
tempDirName = [directory 'wkv_load_all_temp/'];
mkdir(tempDirName);

%% List all the logfiles to load.
filenames = dir([directory '*.wkv']);
filenames = {filenames.name};

%% Load all the logfiles.
logs = repmat(struct('wkv', [], 'txt', []), length(filenames), 1);

for i=1:length(filenames)
    [~, baseName, ~] = fileparts(filenames{i});
    
    baseNameTxt = strrep(baseName, '_vars', '_info');
    
    % Load the logfile.
    disp(['Loading ' filenames{i} '...']);
    log = wkv_load([directory filenames{i}]);
    logs(i).wkv = log;
    logs(i).txt = [baseNameTxt '.txt'];
    
    % Temporarily save as a MAT file, in case the process is interrupted.
    save([tempDirName baseName '.mat'], 'log', '-v7.3');
end

%% Save as a single MAT file, and delete the working directory.
save([directory 'logs.mat'], 'logs', '-v7.3');
rmdir(tempDirName, 's');

end