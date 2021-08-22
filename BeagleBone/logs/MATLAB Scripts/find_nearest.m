function index = find_nearest(array, value)
%FIND_NEAREST Finds the index of the closest element of the ordered array.

startIndex = 1;
stopIndex = length(array);

while stopIndex - startIndex >= 2
    midIndex = floor((startIndex + stopIndex) / 2);
    
    if array(midIndex) == value
        index = midIndex;
        return;
    elseif array(midIndex) < value
        startIndex = midIndex;
    else
        stopIndex = midIndex;
    end
end

if abs(array(startIndex)-value) < abs(array(stopIndex)-value)
    index = startIndex;
else
    index = stopIndex;
end

end
