function [wkv, index] = wkv_add_var(wkv, name, unit, values, times, ...
                                    interpolation)
%WKV_ADD_VAR Adds a variable to a WKV variables list.
%   wkv_add_var(wkv, name, unit, values) adds a variable to the wkv
%   structure. The length of values should match the length of the existing
%   ones.
%   wkv_add_var(wkv, name, unit, values, times, interpolation) adds a
%   variable to the wkv structure, with a distinct time vector. The given
%   values will be interpolated to match the existing time vector of the
%   wkv.

% Check that the size of the given values array is correct.
if ~isvector(values)
    error('The values argument should be a vector.');
end

if ~isempty(wkv) && ~exist('times', 'var') && ...
   (length(values) ~= length(wkv(1).values))
    error(['The length of the values argument should match the length ' ...
           'of the existing variables in the WKV.']);
end

% Check if the variable already exists.
try
    [~, index] = wkv_get(wkv, name);
catch
    % Add the variable to the WKV, filling all the fields of the structure.
    index = length(wkv);
    wkv(end+1) = wkv(end);
    wkv(index).name = name;
    wkv(index).unit = unit;
    wkv(index).type = class(values);
end

if ~exist('times', 'var')
    wkv(index).values = values;
else
    if ~exist('interpolation', 'var')
        interpolation = 'nearest';
    end

    wkv(index).values = interp1(times, values, wkv(1).values, ...
                                interpolation, nan);
end

end
