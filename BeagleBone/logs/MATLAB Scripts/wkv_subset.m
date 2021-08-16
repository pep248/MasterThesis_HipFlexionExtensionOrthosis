function subwkv = wkv_subset(wkv, range)
%WKV_SUBSET Extracts a subset from the given WKV dataset.

subwkv = wkv;

for i=1:length(wkv)
    subwkv(i).values = subwkv(i).values(range);
end

end

