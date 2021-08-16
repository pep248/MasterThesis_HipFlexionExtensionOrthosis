function cycles = wkv_split_cycles_continuous(wkv, splittingVar, ...
                                              phaseOffset, smoothing)
%WKV_SPLIT_CYCLES_CONTINUOUS Splits into sub-wkvs with a single cycle.
%   Splits the given wkv into individual cycles. The method is to strongly
%   filter the values corresponding to the splittingVar index, such that
%   they become a centered sine-like curve. The cutting point are where
%   this curve is crossing zero from a negative to a positive value.
%   The found cutting points can be shift by the phaseOffset [%].

%%
if ~exist('smoothing', 'var')
    smoothing = 500;
end

if ischar(splittingVar)
    traj = wkv_get(wkv, splittingVar);
else
    traj = wkv(splittingVar).values;
end

%% Detect the cycles.

% Apply strong filtering to obtain a centered sine wave.
trajSmooth = smooth(traj, smoothing); % Low-pass filter.
trajSmooth = trajSmooth - smooth(traj, smoothing*2); % High-pass filter.

%
zc = diff(trajSmooth > 0) > 0;
indices = find(zc);
indices = indices(2:end-1); % Remove the first and last cycles, that are likely incomplete.

N = length(indices)-1;

cycles = cell(N,1);

for i=1:length(cycles)
    cycleIndexDuration = indices(i+1) - indices(i);
    phaseIndexOffset = floor(phaseOffset/100 * cycleIndexDuration);
    cycles{i} = wkv_subset(wkv, (indices(i):indices(i+1)) + phaseIndexOffset);
end

end
