function xls = wkv_export2xls(wkv, xlsxFilename, indices, headerLabels)
%WKV_EXPORT2XLS Exports the data from a WKV to an Excel spreadsheet.
% xslxFilename: filename of the output Excel file.
% indices: array of variable indices to be included in the Excel file. If
%   not present, all variables will be included.
% headersName: labels to use for the headers. If not present, the variables
%   name will be used instead.

if exist('indices', 'var')
    wkv = wkv(indices);
end

if exist('headerLabels', 'var')
    if length(headerLabels) ~= length(indices) - 1
        error('headerLabels length should be the same as indices length.');
    end
else
    if isdatetime(wkv(1).values(1))
        headerLabels = {wkv(2:end).name};
    else
        headerLabels = {wkv.name};
    end
end

name = inputname(1);

if isdatetime(wkv(1).values(1))
    timesStrColumns = arrayfun(@(x) {datestr(x, 'yyyy/mm/dd HH:MM:SS:FFF')}, wkv(1).values)';
    valuesColumns = cellfun(@(x)transpose(x), {wkv(2:end).values}, 'UniformOutput', 0);
    valuesMatrix = [timesStrColumns num2cell([valuesColumns{:}])];    
    xls = [['Time' strcat(headerLabels, ' [', {wkv(2:end).unit}, ']')];
           valuesMatrix];
else
    valuesColumns = cellfun(@(x)transpose(x), {wkv.values}, 'UniformOutput', 0);
    valuesMatrix = num2cell([valuesColumns{:}]);
    xls = [strcat(headerLabels, ' [', {wkv.unit}, ']');
           valuesMatrix];
end

warning('off', 'MATLAB:xlswrite:AddSheet');
xlswrite(xlsxFilename, xls, name);

end
