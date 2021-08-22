function [values, varIndex] = wkv_get(wkv, varName)
%WKV_GET Gets the values of a WKV variable, from its name.

nameMatches = strcmp({wkv.name}, varName);

if nnz(nameMatches) == 0 % Variable name not found.
    error('The given variable name was not found in the list.');
else
    varIndex = find(nameMatches, 1);
    values = wkv(varIndex).values;
end

end
