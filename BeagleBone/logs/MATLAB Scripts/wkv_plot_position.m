function wkv_plot_mean_std(stackedVectors, times, plotAllCurves, meanPlotColor)
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
standardDeviation = std(stackedVectors, 1);
mstdp = m + standardDeviation;
mstdm = m - standardDeviation;

%% Plot.
yyaxis right
% Mean and standard deviation envelope.
plot_fill = fill([times fliplr(times)], [mstdm fliplr(mstdp)], ...
     FILL_COLOR, 'EdgeColor','None');
set(plot_fill,'facealpha',.1)
set( get( get( plot_fill, 'Annotation'), 'LegendInformation' ), 'IconDisplayStyle', 'off' );
hold on;

plot(times, m, 'color',meanPlotColor, 'LineWidth', 2, 'LineStyle', '-');

% All the curves.
if plotAllCurves
    plot(times, stackedVectors);
end

end

