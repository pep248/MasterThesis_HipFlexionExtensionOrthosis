function varargout = wkv_split_cycles_leftFootLoad(wkv, footLoadThreshold)
%WKV_SPLIT_CYCLES_CONTINUOUS Splits into sub-wkvs with a single cycle.
%   gaitCycles = wkv_split_cycles_leftFootLoad(wkv, footLoadThreshold = 5)
%   [gaitCycles, cycleTimes] = ...
%   
%   Splits the given wkv into individual cycles. The method is to strongly
%   filter the values corresponding to the splittingVar index, such that
%   they become a centered sine-like curve. The cutting point are where
%   this curve is crossing zero from a negative to a positive value.
%   The found cutting points can be shift by the phaseOffset [%].

% This function has been modified to also show a plot with the detected
% gait cycle starting points at the end, and optionally also return the
% cycle times in a vector -AM.

%%
if nargin < 2
    footLoadThreshold = 5;
end

nOutputs = nargout;
varargout = cell(1,nOutputs);

detectionSignal = wkv_get(wkv, 'controller/left_foot_load');

%% Detect the heel strike event: crossing the threshold only in the positive direction

%
aboveThresholdIndices = find(detectionSignal(2:end) >= footLoadThreshold)+ 1;
positiveCrossingIndices = aboveThresholdIndices( detectionSignal(aboveThresholdIndices-1) < footLoadThreshold );
indices = positiveCrossingIndices(2:end-1); % Remove the first and last cycles, that are likely incomplete.

N = length(indices)-1;

cycles = cell(N,1);

figure, plot(detectionSignal), title('wkv split cycles output plot'), hold on, grid on;

if nOutputs > 1
    cycleTimes = zeros(1,N);
end

for i=1:N
    cycles{i} = wkv_subset(wkv, (indices(i):indices(i+1)));
    
    if nOutputs > 1
        timestamps = wkv_get(cycles{i}, 'timestamp_num');
        cycleTimes(i) = timestamps(end) - timestamps(1);
    end
    xline(indices(i),'--r');
end

varargout{1} = cycles;

if nOutputs > 1
   varargout{2} = cycleTimes;
end

end
