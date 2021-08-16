function x = periodic2continuous(x, minVal, maxVal)
%PERIODIC2CONTINUOUS Inverse of the constrainPeriodic() function.
% For a signal constrained between two bounds (e.g. -180° to 180°), this
% function returns an array that is continuous, and can potentially overcome
% these bounds.

offset = 0;
rangeAmplitude = maxVal - minVal;

for i=2:length(x)
    x(i) = x(i) + offset;
    
    if x(i) - x(i-1) < -rangeAmplitude/2
        x(i) = x(i) + rangeAmplitude;
        offset = offset + rangeAmplitude;
    elseif x(i) - x(i-1) > rangeAmplitude/2
        x(i) = x(i) - rangeAmplitude;
        offset = offset - rangeAmplitude;
    end
end

end

