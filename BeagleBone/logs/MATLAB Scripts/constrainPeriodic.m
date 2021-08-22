function x = constrainPeriodic(x, minVal, maxVal)
%CONSTRAINPERIODIC Constrain a signal into a periodic range.
% Constrains a signal between two bounds (e.g. -180° to 180°). This is typically
% useful for angles.

    x = arrayfun(@constrainPeriodicSingle, x, minVal*ones(size(x)), maxVal*ones(size(x)));
end

function x = constrainPeriodicSingle(x, minVal, maxVal)
    while x < minVal
        x = x + (maxVal - minVal);
    end
    
    while x > maxVal
        x = x - (maxVal - minVal);
    end
end