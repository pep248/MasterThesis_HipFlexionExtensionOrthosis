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

%% CREATE A wave
% f(x) = a1*sin(b1*x+c1) + a2*sin(b2*x+c2) + a3*sin(b3*x+c3) + 
%        a4*sin(b4*x+c4) + a5*sin(b5*x+c5) + a6*sin(b6*x+c6) + 
%        a7*sin(b7*x+c7) + a8*sin(b8*x+c8)
[siny, gof] = sine_fit(filtered_time_r, filtered_torque_r);
a1 = siny.a1; b1 = siny.b1; c1 = siny.c1;
a2 = siny.a2; b2 = siny.b2; c2 = siny.c2;
a3 = siny.a3; b3 = siny.b3; c3 = siny.c3;
a4 = siny.a4; b4 = siny.b4; c4 = siny.c4;
a5 = siny.a5; b5 = siny.b5; c5 = siny.c5;
a6 = siny.a6; b6 = siny.b6; c6 = siny.c6;
a7 = siny.a7; b7 = siny.b7; c7 = siny.c7;
a8 = siny.a8; b8 = siny.b8; c8 = siny.c8;


for i = 1:1:length(one_cycle_time_r)
   t = one_cycle_time_r(i);
   sine_y(i) = a1*sin(b1*t+c1) + a2*sin(b2*t+c2) + a3*sin(b3*t+c3) + a4*sin(b4*t+c4) + a5*sin(b5*t+c5) + a6*sin(b6*t+c6) + a7*sin(b7*t+c7) + a8*sin(b8*t+c8);
end

%% CREATE A wave 2
% f(x) = a1*sin(b1*x+c1) + a2*sin(b2*x+c2) + a3*sin(b3*x+c3) + 
%        a4*sin(b4*x+c4) + a5*sin(b5*x+c5) + a6*sin(b6*x+c6) + 
%        a7*sin(b7*x+c7) + a8*sin(b8*x+c8)
[siny, gof] = sine_fit(filtered_time_r, filtered_torque_r);
a01 = siny.a1*0.8+2; b1 = siny.b1; c01 = siny.c1+0.1;
a02 = siny.a2*0.8+0.5; b2 = siny.b2; c02 = siny.c2;
a03 = siny.a3*0.8+1; b3 = siny.b3; c03 = siny.c3+0.1;
a04 = siny.a4*0.8-0.1; b4 = siny.b4; c04 = siny.c4;
a05 = siny.a5*0.8-0.1; b5 = siny.b5; c05 = siny.c5+0.1;
a06 = siny.a6*0.8-0.1; b6 = siny.b6; c06 = siny.c6;
a07 = siny.a7*0.8-0.05; b7 = siny.b7; c07 = siny.c7+0.1;
a08 = siny.a8*0.8-0.05; b8 = siny.b8; c08 = siny.c8;


for i = 1:1:length(one_cycle_time_r)
   t = one_cycle_time_r(i);
   sine0_y(i) = a01*sin(b1*t+c01) + a02*sin(b2*t+c02) + a03*sin(b3*t+c03) + a04*sin(b4*t+c04) + a05*sin(b5*t+c05) + a06*sin(b6*t+c06) + a07*sin(b7*t+c07) + a08*sin(b8*t+c08);
end

%% PLOT
%plot each equation
figure();
hold on
plot(gait_r,sine_y) % right leg polynomial aproximation
plot(gait_r,sine0_y) % right leg polynomial aproximation
legend('Biological Gait','Biological Gait after optimization')
hold off




%% TRIAL FOR SINE TIME DILATATION

first_period = 1;
second_period = 3;

original_period = 1/(b1/(2*pi));

for i = 1:1:100
    t = ((i-1)/100); %0 - 0.99
    time_1(i) = t;
    frequency_ratio = (original_period/first_period);
    fake_sine_y(i) = a1*sin(b1*t*frequency_ratio+c1) + a2*sin(b2*t*frequency_ratio+c2) + a3*sin(b3*t*frequency_ratio+c3) + a4*sin(b4*t*frequency_ratio+c4) + a5*sin(b5*t*frequency_ratio+c5) + a6*sin(b6*t*frequency_ratio+c6) + a7*sin(b7*t*frequency_ratio+c7) + a8*sin(b8*t*frequency_ratio+c8);
end

for i = 101:1:400
    t = ((i-1)/100); %1 - 3.99
    time_1(i) = t;
end

for i = 101:1:400
    t = ((i-101)/100); %1 - 3.99
    frequency_ratio = (original_period/second_period);
    fake_sine_y(i) = a1*sin(b1*t*frequency_ratio+c1) + a2*sin(b2*t*frequency_ratio+c2) + a3*sin(b3*t*frequency_ratio+c3) + a4*sin(b4*t*frequency_ratio+c4) + a5*sin(b5*t*frequency_ratio+c5) + a6*sin(b6*t*frequency_ratio+c6) + a7*sin(b7*t*frequency_ratio+c7) + a8*sin(b8*t*frequency_ratio+c8);
end


function [fitresult, gof] = sine_fit(filtered_time_r, filtered_torque_r)
%CREATEFIT3(FILTERED_TIME_R,FILTERED_TORQUE_R)
%  Create a fit.
%
%  Data for 'untitled fit 1' fit:
%      X Input : filtered_time_r
%      Y Output: filtered_torque_r
%  Output:
%      fitresult : a fit object representing the fit.
%      gof : structure with goodness-of fit info.
%
%  See also FIT, CFIT, SFIT.

%  Auto-generated by MATLAB on 18-Apr-2021 11:37:01


%% Fit: 'untitled fit 1'.
[xData, yData] = prepareCurveData( filtered_time_r, filtered_torque_r );

% Set up fittype and options.
ft = fittype( 'sin8' );
opts = fitoptions( 'Method', 'NonlinearLeastSquares' );
opts.Display = 'Off';
opts.Lower = [-Inf 0 -Inf -Inf 0 -Inf -Inf 0 -Inf -Inf 0 -Inf -Inf 0 -Inf -Inf 0 -Inf -Inf 0 -Inf -Inf 0 -Inf];
opts.StartPoint = [29.7887279344231 4.36164039835066 -2.22346467821143 14.1577567741483 8.72328079670132 2.22404982731221 6.8066107003031 0.0605783388659814 3.13963796118068 7.70650707332085 13.084921195052 1.05804922522836 6.68642426481538 17.4465615934026 1.06019481475258 6.44563210354092 34.8931231868053 0.852354886777799 4.87028699118765 30.5314827884546 1.79140652897109 3.90852317106897 39.2547635851559 0.0994008956954762];

% Fit model to data.
[fitresult, gof] = fit( xData, yData, ft, opts );

% Plot fit with data.
%{
figure( 'Name', 'untitled fit 1' );
h = plot( fitresult, xData, yData );
legend( h, 'filtered_torque_r vs. filtered_time_r', 'untitled fit 1', 'Location', 'NorthEast', 'Interpreter', 'none' );
% Label axes
xlabel( 'filtered_time_r', 'Interpreter', 'none' );
ylabel( 'filtered_torque_r', 'Interpreter', 'none' );
grid on
%}
end
