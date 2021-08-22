%% PLOT HIP VELOCITY AND POSITION TOLERANCE

%legend('Experimental assisted hip position','Experimental unassisted hip position','Ideal hip position')
legend('Experimental assisted hip position','Experimental unassisted hip position')

%figure propperties
    %title('Hip Position and Velocity Tolerance')

xlabel('% gait cycle','fontsize',20) 

yyaxis left
%fill(shit ,inBetween_position, color_position, 'LineStyle','none');
ylabel('Torque (N·m)','fontsize',20)


yyaxis right
%fill(shit ,inBetween_velocity, color_velocity,'LineStyle','none');
ylabel('Acceleration (º/s^2)  ,  Position (º)','fontsize',20)

ax = gca;
set(gca,'FontSize',20)

str = '#FF0000';
color_right = sscanf(str(2:end),'%2x%2x%2x',[1 3])/255;
str = '#0099FF';
color_left = sscanf(str(2:end),'%2x%2x%2x',[1 3])/255;

ax.YAxis(1).Color = color_right;
ax.YAxis(2).Color = color_left;


legend('Torque applied to the motors','Hip Position','Hip Acceleration')

hold off