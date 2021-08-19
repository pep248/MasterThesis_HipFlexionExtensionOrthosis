%clear previous values
%clear;

%% READ EXCEL FILE
table = xlsread('Winter_Gait.xls','Joint Moments');

%% PLOT
figure();
gait = table(1:51, 1);

very_slow_minus = table(1:51, 2);
very_slow = table(1:51, 3);
very_slow_plus = table(1:51, 4);

slow_minus = table(1:51, 5);
slow = table(1:51, 6);
slow_plus = table(1:51, 7);

free_minus = table(1:51, 8);
free = table(1:51, 9);
free_plus = table(1:51, 10);

fast_minus = table(1:51, 11);
fast = table(1:51, 12);
fast_plus = table(1:51, 13);

very_fast_minus = table(1:51, 14);
very_fast = table(1:51, 15);
very_fast_plus = table(1:51, 16);

%% PLOT HIP VELOCITY AND POSITION TOLERANCE

hold on
%choose colors
str = '#FF0000';
color_very_slow = sscanf(str(2:end),'%2x%2x%2x',[1 3])/255;
str = '#FF8000';
color_slow = sscanf(str(2:end),'%2x%2x%2x',[1 3])/255;
str = '#009900';
color_free = sscanf(str(2:end),'%2x%2x%2x',[1 3])/255;
str = '#FFFF00';
color_fast = sscanf(str(2:end),'%2x%2x%2x',[1 3])/255;
str = '#0000FF';
color_very_fast = sscanf(str(2:end),'%2x%2x%2x',[1 3])/255;

%time vector
shit = [gait'*100, fliplr(gait'*100)];
%{
%very_slow
plot(gait*100,very_slow,'LineWidth',2,'color',color_very_slow) % original position
inBetween_very_slow = [very_slow_plus', fliplr(very_slow_minus')];
plot_very_slow = fill(shit ,inBetween_very_slow, color_very_slow,'LineStyle','none','HandleVisibility','off');
set(plot_very_slow,'facealpha',.2)

%slow
plot(gait*100,slow,'LineWidth',2,'color',color_slow) % original position
inBetween_slow = [slow_plus', fliplr(slow_minus')];
plot_slow = fill(shit ,inBetween_slow, color_slow,'LineStyle','none','HandleVisibility','off');
set(plot_slow,'facealpha',.2)
%}
%free
plot(gait*100,free,'LineWidth',2,'color',color_free) % original position
inBetween_free = [free_plus', fliplr(free_minus')];
plot_free = fill(shit ,inBetween_free, color_free,'LineStyle','none','HandleVisibility','off');
set(plot_free,'facealpha',.1)
%{
%fast
plot(gait*100,fast,'LineWidth',2,'color',color_fast) % original position
inBetween_fast = [fast_plus', fliplr(fast_minus')];
plot_fast = fill(shit ,inBetween_fast, color_fast,'LineStyle','none','HandleVisibility','off');
set(plot_fast,'facealpha',.2)

%very_fast
plot(gait*100,very_fast,'LineWidth',2,'color',color_very_fast) % original position
inBetween_very_fast = [very_fast_plus', fliplr(very_fast_minus')];
plot_very_fast = fill(shit ,inBetween_very_fast, color_very_fast,'LineStyle','none','HandleVisibility','off');
set(plot_very_fast,'facealpha',.2)
%}
%axis
xlabel('% gait cycle','fontsize',20) 
ylabel('Position (º)','fontsize',20)
ax = gca;
set(gca,'FontSize',20)

%legend('Experimental assisted hip position','Experimental unassisted hip position','Ideal hip position')
legend('Experimental assisted hip position','Experimental unassisted hip position')

hold off