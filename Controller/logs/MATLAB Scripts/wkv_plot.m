function wkv_plot(wkv, variablesToPlot, varargin)
%WKV_PLOT Plots several variables over time, from a WKV structure.
% wkv_plot(wkv, varsIndices) plots the variables designated by the indices
% varsIndices.
% wkv_plot(wkv, varsNames) plots the variables designated by their name.
% The names should be in a cell array if there is more than one variable to
% plot.
% wkv_plot(___, 'LineStyle', lineStyle) sets the line style (see the plot()
% documentation for more information).
% wkv_plot(___, 'LineWidth', lineWidth) sets the line width (see the plot()
% documentation for more information).
% wkv_plot(___, 'MessagesFile', messagesFile) adds tags on the figure (a
% vertical line and a label) for each message starting by "#" in the given
% file.

%% Interpret the optional arguments.
argParser = inputParser;
addOptional(argParser, 'LineStyle', '-');
addOptional(argParser, 'LineWidth', 0.5);
addOptional(argParser, 'MessagesFile', '');
parse(argParser, varargin{:});

lineStyle = argParser.Results.LineStyle;
lineWidth = argParser.Results.LineWidth;
messagesFile = argParser.Results.MessagesFile;

%% If a string cell array is given (variables names), convert to indices.
if isnumeric(variablesToPlot)
    varsIndices = variablesToPlot;
else
    if iscell(variablesToPlot)
        numericVarIndices = [];
        
        for str = variablesToPlot
            [~, idx] = wkv_get(wkv, str{:});
            numericVarIndices = [numericVarIndices idx]; %#ok<AGROW>
        end
        
        varsIndices = numericVarIndices;
    else
        [~, varsIndices] = wkv_get(wkv, variablesToPlot);
    end
end

%% Plot the variables values.

% Find the timestamp index.
timeIndex = find(strcmp({wkv.name}, 'timestamp'), 1);

if isempty(timeIndex)
    error('The timestamp could not be found.');
end

% Discard all the data before a change of time (dt > 1 min), otherwise a
% lot of manual zooming will be required to see the relevant data.
setTimeInd = find(abs(diff(wkv(timeIndex).values)) > duration(0, 1, 0), ...
                  1, 'last');

if ~isempty(setTimeInd)
    for i=1:length(wkv)
        wkv(i).values(1:setTimeInd) = [];
    end
end

% Plot.
% The colorblind-friendly colormap is from
% https://ch.mathworks.com/matlabcentral/fileexchange/46802-color-blind-friendly-colormap,
% except the third color ("yellow") which is darker, otherwise it is too
% difficult to see on some screens.
varsMat = cell2mat({wkv(varsIndices).values}');

plot_colormap = [0,0,1;
                 1,0,0;
                 0.9,0.9,0;
                 0.660156250000000,0.660156250000000,0.660156250000000;
                 0,0,0;1,0.644531250000000,0;
                 1,0,1;0,0.500000000000000,0.500000000000000;
                 0,0,0.542968750000000;
                 0,0.390625000000000,0;
                 0,1,1;
                 0.597656250000000,0.195312500000000,0.796875000000000];

plot(wkv(timeIndex).values, varsMat, ...
     'LineStyle', lineStyle, 'LineWidth', lineWidth);
set(gca, 'ColorOrder', plot_colormap);
xtickformat('hh:mm:ss.SSS');

%% Generate the legend.
legendLabels = cell(1, length(varsIndices));
for i=1:length(varsIndices)
    varIndex = varsIndices(i);
    legendLabels{i} = [wkv(varIndex).name ' [' wkv(varIndex).unit ']'];
end

hLegend = legend(legendLabels);
hLegend.Interpreter = 'none';

%% Show the user tags.
if exist(messagesFile, 'file')
    tags = extract_messages(wkv, messagesFile, 1);
    
    wkv_plot_show_messages(tags);
end

end
