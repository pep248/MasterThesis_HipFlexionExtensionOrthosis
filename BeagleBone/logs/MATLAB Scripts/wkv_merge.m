function mergedWkv = wkv_merge(wkvA, wkvB)
%WKV_MERGE Merges two wkv files together.

mergedWkv = wkvA;

for i=1:length(mergedWkv)
    mergedWkv(i).values = [wkvA(i).values wkvB(i).values];
end

end

