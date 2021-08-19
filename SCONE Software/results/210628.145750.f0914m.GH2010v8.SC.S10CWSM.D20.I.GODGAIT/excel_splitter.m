%clear previous values
clear;
%close all;

%% READ EXCEL FILE
table = readtable('data.xlsx');
array = table2array(table);
l = length(array);
d = length((fix(l/10)):1:l); %ignore the first 10% of values

%we declare the arrays where to store the read values
time(d) = 0;
actuator_right = 0;
ground_force_r(d) = 0;


% ignore the first 10% of the simulation
for i = (fix(l/10)):1:l
   time(i-(fix(l/10))+1)= array(i,1);
   
   hamstring_r(i-(fix(l/10))+1)= array(i,2);
   glutmax_r(i-(fix(l/10))+1)= array(i,3);
   iliopsoas_r(i-(fix(l/10))+1)= array(i,4);
   
   hamstring_l(i-(fix(l/10))+1)= array(i,5);
   glutmax_l(i-(fix(l/10))+1)= array(i,6);
   iliopsoas_l(i-(fix(l/10))+1)= array(i,7);
   
   ground_force_r(i-(fix(l/10))+1)= array(i,8);
   period_right(i-(fix(l/10))+1)= array(i,9);
   actuator_right(i-(fix(l/10))+1)= array(i,11);
end
l = length(time);

%% DETERMINE WHERE DO WE HAVE A HEEL STRIKE
%determine the iterations where we have a heel strike
heel_strike_r_length=1; %simple counter

for i = 2:1:(l)
    if ( (ground_force_r(i-1) == 0) && (ground_force_r(i) ~= 0) )
        heel_strike_r(heel_strike_r_length) = i;
        heel_strike_r_length = heel_strike_r_length + 1;
    end
end
heel_strike_r_length=heel_strike_r_length-1;

%% GET ONE CYCLE
% interval of a hole cycle
starting_r = heel_strike_r(1); %first iteration of the first cycle
ending_r = heel_strike_r(2)-1; %last iteration of the first cycle
last_r = heel_strike_r(heel_strike_r_length)-1; %last iteration of the last complete cycle

% properties of the wave
total_time = time(last_r) - time(starting_r); %duration of the studied wave in secons (from the first complete cycle to the last complete one)
period = total_time / heel_strike_r_length; %period of the wave
f = 1/period; %frequency of the wave

% hole wave reshaping (to eliminate the initial and last values)
filtered_time_r = time(starting_r:last_r);
%filtered_time_r = filtered_time_r - filtered_time_r(1);
filtered_actuator_right = actuator_right(starting_r:last_r);



%% PLOT
%plot each equation
%figure();
hold on

last_period = time(heel_strike_r(13):heel_strike_r(14));
last_period(:) = last_period(:)-last_period(1);
last_period(:) = last_period(:)/last_period(length(last_period))*100;
last_torque = actuator_right(heel_strike_r(13):heel_strike_r(14));
%plot(filtered_time_r,filtered_actuator_right*2000) % right leg torque
%plot(time(heel_strike_r(13)):time(heel_strike_r(14)), period_right(heel_strike_r(13)):period_right(heel_strike_r(14)) )
str = '#009900'; %green
color_line = sscanf(str(2:end),'%2x%2x%2x',[1 3])/255;
plot(last_period,20*last_torque,'color',color_line,'LineWidth',2);
%plot(gait_r,polynomial_y) % right leg polynomial aproximation
%plot(gait_r,sine_y) % right leg polynomial aproximation
%legend('Healthy Gait Moment','Sinusoidal Fit')
xlabel('% gait cycle','fontsize',20)
ylabel('Actuator Torque(N�m)','fontsize',20)
set(gca,'FontSize',20)
legend('Experimental provided torque','Theoretical torque shape')
hold off

