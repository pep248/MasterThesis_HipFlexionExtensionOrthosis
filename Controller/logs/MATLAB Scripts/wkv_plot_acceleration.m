function wkv_plot_acceleration(stackedVectors, times, plotAllCurves, meanPlotColor)
%WKV_PLOT_MEAN_STD Plots mean and standard deviation of a set of vectors.
%   stackedVectors is a matrix made of horizontal vectors, stacked
%   vertically.

%% Constants.
FILL_COLOR = meanPlotColor; % Light gray.

%% Check the function arguments.
if ~isvector(times)
    error('times should be a vector.');
end

if size(stackedVectors, 2) ~= length(times)
    error(['times should have the same length as the number of columns' ...
           'of stackedVectors.']);
end

if ~exist('plotAllCurves', 'var')
    plotAllCurves = 0;
end

if ~exist('meanPlotColor', 'var')
    meanPlotColor = 'black';
end

nSeries = size(stackedVectors, 1);

%% Compute the mean and std curves.
m = mean(stackedVectors, 1);


for i = 1 : 1 : length(m)-1
    
    if(i == 1)
        accel_m(i) = (m(i+1)-m(i)) / (times(i+1)-times(i));
    elseif (i == length(m)-1)
        accel_m(i) = (m(i+1)-m(i)) / (times(i+1)-times(i));
    else
        accel_m(i) = (m(i+1)-m(i-1)) / (times(i+1)-times(i-1));
    end
    
end

accel_m(i+1) = (m(i+1)-m(i-1)) / (times(i+1)-times(i-1));

accel_m = moving_average(accel_m,20);

standardDeviation = std(stackedVectors, 1);
accel_mstdp = accel_m + standardDeviation/4;
accel_mstdm = accel_m - standardDeviation/4;

%accel_mstdp = moving_average(accel_mstdp,20);
%accel_mstdm = moving_average(accel_mstdm,20);
%accel_m = moving_average(accel_m,5);
    

%% Plot.

% Mean and standard deviation envelope.
yyaxis right
plot_fill = fill([times fliplr(times)], [accel_mstdm fliplr(accel_mstdp)], ...
     FILL_COLOR, 'EdgeColor','None');
set(plot_fill,'facealpha',.1)
set( get( get( plot_fill, 'Annotation'), 'LegendInformation' ), 'IconDisplayStyle', 'off' );
hold on;

plot(times, accel_m, 'color',meanPlotColor, 'LineWidth', 2, 'LineStyle', '-');

% All the curves.
if plotAllCurves
    plot(times, stackedVectors);
end

end

