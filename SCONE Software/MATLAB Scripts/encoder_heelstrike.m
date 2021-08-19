%clear previous values
clear;

%% READ EXCEL FILE
table = readtable('leg_graphs.xlsx');
array = table2array(table);
l = length(array);
d = length((fix(l/10)):1:l); %ignore the first 10% of values

%we declare the arrays where to store the read values
time(d) = 0;
femmur_r_pos(d) = 0;
femmur_l_pos(d) = 0;
femmur_r_vel(d) = 0;
femmur_l_vel(d) = 0;
torque_r(d) = 0;
torque_l(d) = 0;
ground_force_r(d) = 0;
ground_force_l(d) = 0;

% ignore the first 10% of the simulation
for i = (fix(l/10)):1:l
   time(i-(fix(l/10))+1)= array(i,1);
   femmur_r_pos(i-(fix(l/10))+1)= array(i,2);
   femmur_l_pos(i-(fix(l/10))+1)= array(i,3);
   femmur_r_vel(i-(fix(l/10))+1)= array(i,4);
   femmur_l_vel(i-(fix(l/10))+1)= array(i,5);
   torque_r(i-(fix(l/10))+1)= array(i,6);
   torque_l(i-(fix(l/10))+1)= array(i,7);
   ground_force_r(i-(fix(l/10))+1)= array(i,8);
   ground_force_l(i-(fix(l/10))+1)= array(i,9);
end
l = length(time);

%% DETERMINE WHERE DO WE HAVE A HEEL STRIKE
%determine the iterations where we have a heel strike
heel_strike_r_length=1; %simple counter
heel_strike_l_length=1; %simple counter
for i = 2:1:(l)
    if ( (ground_force_r(i-1) == 0) && (ground_force_r(i) ~= 0) )
        heel_strike_r(heel_strike_r_length) = i;
        heel_strike_r_length = heel_strike_r_length + 1;
    end
    if ( (ground_force_l(i) == 0) && (ground_force_l(i+1) ~= 0) )
        heel_strike_l(heel_strike_l_length) = i;
        heel_strike_l_length = heel_strike_l_length + 1;
    end
end
heel_strike_r_length=heel_strike_r_length-1;
heel_strike_l_length=heel_strike_l_length-1;

%% GET ONE CYCLE
% interval of a hole cycle
starting_r = heel_strike_r(1); %first iteration of the first cycle
starting_l = heel_strike_l(1);
ending_r = heel_strike_r(2)-1; %last iteration of the first cycle
ending_l = heel_strike_l(2)-1;
last_r = heel_strike_r(heel_strike_r_length)-1; %last iteration of the last complete cycle
last_l = heel_strike_l(heel_strike_l_length)-1; 

% properties of the wave
total_time = time(last_r) - time(starting_r); %duration of the studied wave in secons (from the first complete cycle to the last complete one)
period = total_time / heel_strike_r_length; %period of the wave
f = 1/period; %frequency of the wave

% hole wave reshaping (to eliminate the initial and last values)
filtered_time_r = time(starting_r:last_r);
filtered_time_r = filtered_time_r - filtered_time_r(1);
filtered_time_l = time(starting_l:last_l);
filtered_time_l = filtered_time_l - filtered_time_l(1);
filtered_femmur_r_pos = femmur_r_pos(starting_r:last_r);
filtered_femmur_l_pos = femmur_l_pos(starting_l:last_l);
filtered_femmur_r_vel = femmur_r_vel(starting_r:last_r);
filtered_femmur_l_vel = femmur_l_vel(starting_l:last_l);
filtered_torque_r = torque_r(starting_r:last_r);
filtered_torque_l = torque_l(starting_l:last_l);

% one cycle wave reshaping (to eliminate the initial values)
one_cycle_time_r = time(starting_r:ending_r);
one_cycle_time_r = one_cycle_time_r - one_cycle_time_r(1);
one_cycle_time_l = time(starting_l:ending_l);
one_cycle_time_l = one_cycle_time_l - one_cycle_time_l(1);
one_cycle_femmur_r_pos = femmur_r_pos(starting_r:ending_r);
one_cycle_femmur_l_pos = femmur_l_pos(starting_l:ending_l);
one_cycle_femmur_r_vel = femmur_r_vel(starting_r:ending_r);
one_cycle_femmur_l_vel = femmur_l_vel(starting_l:ending_l);
one_cycle_torque_r = torque_r(starting_r:ending_r);
one_cycle_torque_l = torque_l(starting_l:ending_l);

% gait axis
gait_r = one_cycle_time_r(:)/one_cycle_time_r(length(one_cycle_time_r));

%% CREATE AN ARRAY OF POSITIONS AND VELOCITIES

for p = 1 : 1 : length(heel_strike_r) - 1 % p = amount of heelstrikes
    for q = heel_strike_r(p) : 1 : heel_strike_r(p+1)-1 %q = array of values for one step
        r = q-heel_strike_r(p)+1;

        if (p == 1)
            max_position(r) = femmur_r_pos(q);
            min_position(r) = femmur_r_pos(q);
            max_velocity(r) = femmur_r_vel(q);
            min_velocity(r) = femmur_r_vel(q);
        else
            if (r >= length(max_position))
                break 
            end

            if (femmur_r_pos(q) > max_position(r))
                max_position(r) = femmur_r_pos(q);
            end
            if (femmur_r_pos(q) < min_position(r))
                min_position(r) = femmur_r_pos(q);
            end
            if (femmur_r_vel(q) > max_velocity(r))
                max_velocity(r) = femmur_r_vel(q);
            end
            if (femmur_r_vel(q) < min_velocity(r))
                min_velocity(r) = femmur_r_vel(q);
            end
        end
    end
end





for h = 1:1:length(gait_r)
    position_tolerance_max(h) = max_position(1);
    position_tolerance_min(h) = min_position(1);
    velocity_tolerance_max(h) = max_velocity(1);
    velocity_tolerance_min(h) = min_velocity(1);
end
%% PLOT HIP VELOCITY AND POSITION TOLERANCE
figure();
hold on
str = '#C1F3B7';
color_position = sscanf(str(2:end),'%2x%2x%2x',[1 3])/255;
str = '#a3cbf2';
color_velocity = sscanf(str(2:end),'%2x%2x%2x',[1 3])/255;
str = '#fa888a';
color_tolerance = sscanf(str(2:end),'%2x%2x%2x',[1 3])/255;


shit = [gait_r'*100, fliplr(gait_r'*100)];
inBetween_position = [max_position, fliplr(min_position)];
inBetween_velocity = [max_velocity, fliplr(min_velocity)];
inBetween_tolerance1 = [position_tolerance_max, fliplr(position_tolerance_min)];
inBetween_tolerance2 = [velocity_tolerance_max, fliplr(velocity_tolerance_min)];

%figure propperties
    %title('Hip Position and Velocity Tolerance')

xlabel('Gait %','fontsize',20) 

yyaxis left
fill(shit ,inBetween_position, color_position, 'LineStyle','none');
ylabel('Position (º)','fontsize',20)


yyaxis right
fill(shit ,inBetween_velocity, color_velocity,'LineStyle','none');
ylabel('Velocity(º/s)','fontsize',20)

ax = gca;
set(gca,'FontSize',20)

str = '#2aa13e';
color_right = sscanf(str(2:end),'%2x%2x%2x',[1 3])/255;
str = '#4865c7';
color_left = sscanf(str(2:end),'%2x%2x%2x',[1 3])/255;

ax.YAxis(1).Color = color_right;
ax.YAxis(2).Color = color_left;

yyaxis left
tol1 = fill(shit ,inBetween_tolerance1, color_tolerance,'LineStyle','none','HandleVisibility','off');
set(tol1,'facealpha',.5)
yyaxis right
tol2 = fill(shit ,inBetween_tolerance2, color_tolerance,'LineStyle','none','HandleVisibility','off');
set(tol2,'facealpha',.5)


legend('Healthy Gait Position Interval','Healthy Gait Velocity Interval')

hold off



function [fitresult, gof] = poly_fit(gait_r, one_cycle_torque_r)
%% Fit: 'untitled fit 1'.
[xData, yData] = prepareCurveData( gait_r, one_cycle_torque_r );

% Set up fittype and options.
ft = fittype( 'poly4' );

% Fit model to data.
[fitresult, gof] = fit( xData, yData, ft );
%{
% Plot fit with data.
figure( 'Name', 'untitled fit 1' );
h = plot( fitresult, xData, yData );
legend( h, 'one_cycle_torque_r vs. gait_r', 'untitled fit 1', 'Location', 'NorthEast', 'Interpreter', 'none' );
% Label axes
xlabel( 'gait_r', 'Interpreter', 'none' );
ylabel( 'one_cycle_torque_r', 'Interpreter', 'none' );
grid on
%}
end