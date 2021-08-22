function [stackedCycles, times] = wkv_stack_cycles(arg1, arg2, nPoints)
%WKV_STACK_CYCLES Makes a matrix with the cycles stacked and synchronized.
%
%   This function can be used with WKVs, or vectors:
%   wkv_stack_cycles(wkvs, varIndex, nPoints)
%   wkv_stack_cycles(times, values, nPoints)
%
%   From cycles data, interpolates such that all the cycles have the same
%   duration and time base, and returns a matrix made of horizontal values
%   vectors.
%   The output of this function can be given to wkv_plot_mean_std(), or
%   used with "mean(stackedCycles, 1)" to get the average over the cycles.

%% Interpret the function arguments.

% Select the right mode of the function.
wkvMode = isscalar(arg2);

if wkvMode
    %
    wkvs = arg1;
    varIndex = arg2;
    
    %
    times = cellfun(@(wkv) wkv(end).values, wkvs, 'UniformOutput', 0);
    values = cellfun(@(wkv) wkv(varIndex).values, wkvs, 'UniformOutput', 0);
else
    %
    times = arg1;
    values = arg2;
    
    % Check the inputs.
    if ~isvector(times) || ~isvector(values)
        error('Inputs should be vectors of cells, containing arrays.');
    end

    if length(times) ~= length(values)
        error('Times and values lengths do not match.');
    end    
end

nCycles = length(times);

% If the points count is not given, compute it from the cycles mean.
if ~exist('nPoints', 'var')
    nPoints = 0;
    
    for i=1:nCycles
        cycleTimes = times{i};
        nPoints = nPoints + length(cycleTimes) / nCycles;
    end
    
    nPoints = floor(nPoints);
end

%% Compute the mean cycle duration.
meanCyclePeriod = 0;

for i=1:nCycles
    cycleTimes = wkvs{i};

    cycleDuration = (cycleTimes(end).values(end) - cycleTimes(end).values(1));
    meanCyclePeriod = meanCyclePeriod + cycleDuration / nCycles; 
end

%% Make the array.
timesInterp = linspace(0, 1, nPoints);
stackedCycles = zeros(nCycles, nPoints);

for i=1:nCycles
    localTimes = times{i};
    localTimes = localTimes - localTimes(1);
    localTimes = localTimes / localTimes(end);
    stackedCycles(i,:) = interp1(localTimes, values{i}, timesInterp);
end

times = timesInterp * meanCyclePeriod;

end

