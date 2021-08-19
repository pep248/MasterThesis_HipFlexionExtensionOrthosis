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

% one cycle wave reshaping (pick olny one cycle)
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

%% CREATE A POLYNOMIAL
[poly, gof] = poly_fit(one_cycle_time_r, one_cycle_torque_r);
p4 = poly.p1;
p3 = poly.p2;
p2 = poly.p3;
p1 = poly.p4;
p0 = poly.p5;


for i = 1:1:length(one_cycle_time_r)
   t = one_cycle_time_r(i);
   polynomial_y(i) = p4*t^4 + p3*t^3 + p2*t^2 + p1*t + p0;
end
%% PLOT
%plot each equation
figure();
hold on
plot(gait_r,one_cycle_torque_r) % right leg torque
plot(gait_r,polynomial_y) % right leg polynomial aproximation
legend('Healthy Gait Moment','Polynomial Fit')
xlabel('Gait %','fontsize',20)
ylabel('Torque(N·m)','fontsize',20)
set(gca,'FontSize',20)
hold off

figure();
hold on
plot(gait_r,one_cycle_femmur_r_pos*10) % original position
plot(gait_r,one_cycle_femmur_r_vel) % original position
legend('Right Hip Lateral Angle (x10)','Right Hip Lateral angular velocity')
hold off

%% TRIAL FOR TIME DILATATION
original_period = one_cycle_time_r(length(one_cycle_time_r));
new_period = original_period * 2;

fake_time(:) = one_cycle_time_r(:)*2;

for i = 1:1:length(gait_r)
   t = (original_period/new_period) * fake_time(i);
   fake_polynomial_y(i) = p4*t^4 + p3*t^3 + p2*t^2 + p1*t + p0;
end

figure();
hold on
plot(one_cycle_time_r,polynomial_y) % original position
plot(fake_time,fake_polynomial_y) % original position
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