function vars = wkv_load(filename)
%WKV_LOAD Loads a .wkv mainboard logfile to a structures array.

%#ok<*AGROW>

%% Constants.
indexToType = {'uint8', 'uint8', 'int8', 'uint16', 'int16', ...
               'uint32', 'int32', 'uint64', 'int64', 'single', 'double'};

%% Open the file.
filename(filename == '"') = [];
fid = fopen(filename, 'r');

%% Read the header.
nVars = fread(fid, 1, 'uint16');

vars = [];
logLineSize = 0;

for i=1:nVars
    vars(i).name = readString(fid);
    vars(i).unit = readString(fid);
    vars(i).type = indexToType{fread(fid, 1, 'uint8') + 1};
    vars(i).length = fread(fid, 1, 'uint32');
    
    logLineSize = logLineSize + vars(i).length;
end

%% Determine the number of log "lines" using the file size.
fileinfos = dir(filename);
filesSize = fileinfos.bytes;

headerSize = ftell(fid);

nLines = floor((filesSize - headerSize) / logLineSize);

%% Read the variables values over time.
% Remark: using plot() with mixed datetime and single values is bogus
% (horizontal precision issues), so all values are converted to double.
fprintf('\nLoading...  0%%');

fileOffset = headerSize;

for i=1:nVars
    fseek(fid, fileOffset, 'bof');
    vars(i).values = fread(fid, nLines, vars(i).type, ...
                           logLineSize-vars(i).length)';
    fprintf('\b\b\b%2i%%', floor(i / (nVars+1) * 100));
    
    fileOffset = fileOffset + vars(i).length;
end

fprintf('\b\b\bDone.\n');

%% Close the file.
fclose(fid);

%% Convert the timestamps to MATLAB dates.
if ~isempty(vars)
    timeIndex = find(strcmp({vars.name}, 'timestamp'), 1);

    if ~isempty(timeIndex) && strcmp(vars(timeIndex).unit, 'us')
        % Make a copy in case the double values (non-datetime) are
        % required.
        vars(nVars+1) = vars(1);
        vars(end).name = 'timestamp_num';
        vars(end).unit = 's';
		vars(end).type = 'double';
        vars(end).values = vars(end).values / 1e6; % [us to s].

        % Convert the double values to datetime objects.
        vars(1).values = datetime(vars(end).values, ...
                                  'ConvertFrom', 'posixtime');
    end
end

end

function str = readString(fid)

str = '';
c = fread(fid, 1, 'char');
while c ~= 0
    str = [str c];
    c = fread(fid, 1, 'char');
end

end