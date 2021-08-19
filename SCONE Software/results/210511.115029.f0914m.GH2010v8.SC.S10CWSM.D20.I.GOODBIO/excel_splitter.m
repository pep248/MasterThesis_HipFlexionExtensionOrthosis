%clear previous values
clear;
close all;

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
   ground_force_l(i-(fix(l/10))+1)= array(i,9);
   period_right(i-(fix(l/10))+1)= array(i,10);
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

%determine the iterations where we have a heel strike
heel_strike_l_length=1; %simple counter

for i = 2:1:(l)
    if ( (ground_force_l(i-1) == 0) && (ground_force_l(i) ~= 0) )
        heel_strike_l(heel_strike_l_length) = i;
        heel_strike_l_length = heel_strike_l_length + 1;
    end
end
heel_strike_l_length=heel_strike_l_length-1;


%% FILTER 10%
% interval of a hole cycle
starting_r = heel_strike_r(1); %first iteration of the first cycle
ending_r = heel_strike_r(2)-1; %last iteration of the first cycle
pre_last_r = heel_strike_r(heel_strike_r_length-1)-1;
last_r = heel_strike_r(heel_strike_r_length)-1; %last iteration of the last complete cycle

% interval of a hole cycle
starting_l = heel_strike_l(1); %first iteration of the first cycle
ending_l = heel_strike_l(2)-1; %last iteration of the first cycle
pre_last_l = heel_strike_l(heel_strike_l_length-1)-1;
last_l = heel_strike_l(heel_strike_l_length)-1; %last iteration of the last complete cycle

% hole wave reshaping (to eliminate the initial and last values)
filtered_time_r = time(starting_r:last_r);
filtered_time_r = filtered_time_r - filtered_time_r(1);
filtered_time_l = time(starting_l:last_l);
filtered_time_l = filtered_time_l - filtered_time_l(1);

filtered_hamstring_r = hamstring_r(starting_r:last_r);
filtered_glutmax_r = glutmax_r(starting_r:last_r);
filtered_iliopsoas_r = iliopsoas_r(starting_r:last_r);
   
filtered_hamstring_l = hamstring_l(starting_l:last_l);
filtered_glutmax_l = glutmax_l(starting_l:last_l);
filtered_iliopsoas_l = iliopsoas_l(starting_l:last_l);

filtered_actuator_right = actuator_right(starting_r:last_r);

% one cycle wave reshaping (to eliminate the initial values)
one_cycle_time_r = time(starting_r:ending_r);
one_cycle_time_r = one_cycle_time_r - one_cycle_time_r(1);
one_cycle_time_l = time(starting_l:ending_l);
one_cycle_time_l = one_cycle_time_l - one_cycle_time_l(1);
last_cycle_time_r = time(pre_last_r:last_r);
last_cycle_time_l = time(pre_last_l:last_l);


%% ADD
one_cycle_ham_r = hamstring_r(starting_r:ending_r);
one_cycle_ham_l = hamstring_l(starting_l:ending_l);
last_cycle_ham_r = hamstring_r(pre_last_r:last_r);
last_cycle_ham_l = hamstring_l(pre_last_l:last_l);

one_cycle_glut_r = glutmax_r(starting_r:ending_r);
one_cycle_glut_l = glutmax_l(starting_l:ending_l);
last_cycle_glut_r = glutmax_r(pre_last_r:last_r);
last_cycle_glut_l = glutmax_l(pre_last_l:last_l);

one_cycle_ili_r = iliopsoas_r(starting_r:ending_r);
one_cycle_ili_l = iliopsoas_l(starting_l:ending_l);
last_cycle_ili_r = iliopsoas_r(pre_last_r:last_r);
last_cycle_ili_l = iliopsoas_l(pre_last_l:last_l);


% gait axis
gait_r = one_cycle_time_r(:)/one_cycle_time_r(length(one_cycle_time_r));
gait_l = one_cycle_time_l(:)/one_cycle_time_l(length(one_cycle_time_l));


%% CREATE AN ARRAY OF MAX AND MIN FOR EACH VARIABLE

%RIGHT
for p = 1 : 1 : length(heel_strike_r) - 1 % p = amount of heelstrikes
    %resize the shape of the curves
    temp_hamstring_r = hamstring_r(heel_strike_r(p) : heel_strike_r(p+1)-1);
    new_hamstring_r(p,:) = imresize(temp_hamstring_r, [1,length(last_cycle_time_r)]);
    
    temp_glutmax_r = glutmax_r(heel_strike_r(p) : heel_strike_r(p+1)-1);
    new_glutmax_r(p,:) = imresize(temp_glutmax_r, [1,length(last_cycle_time_r)]);
    
    temp_iliopsoas_r = iliopsoas_r(heel_strike_r(p) : heel_strike_r(p+1)-1);
    new_iliopsoas_r(p,:) = imresize(temp_iliopsoas_r, [1,length(last_cycle_time_r)]);
    
    for q = 1 : 1 : length(last_cycle_time_r) %q = array of values for one step
        
        if (p == 1)
            max_hamstring_r(q) = new_hamstring_r(p,q);
            min_hamstring_r(q) = new_hamstring_r(p,q);
            
            max_glutmax_r(q) = new_glutmax_r(p,q);
            min_glutmax_r(q) = new_glutmax_r(p,q);
            
            max_iliopsoas_r(q) = new_iliopsoas_r(p,q);
            min_iliopsoas_r(q) = new_iliopsoas_r(p,q);
        else
            %if ((r >= length(max_hamstring_r)) || (q >= length(hamstring_r)))
                %break 
            %end            
            %hamstrings
            if (new_hamstring_r(p,q) > max_hamstring_r(q))
                max_hamstring_r(q) = new_hamstring_r(p,q);
            end
            if (new_hamstring_r(p,q) < min_hamstring_r(q))
                min_hamstring_r(q) = new_hamstring_r(p,q);
            end
            %gluteus
            if (new_glutmax_r(p,q) > max_glutmax_r(q))
                max_glutmax_r(q) = new_glutmax_r(p,q);
            end
            if (new_glutmax_r(p,q) < min_glutmax_r(q))
                min_glutmax_r(q) = new_glutmax_r(p,q);
            end
            %iliopsoas
            if (new_iliopsoas_r(p,q) > max_iliopsoas_r(q))
                max_iliopsoas_r(q) = new_iliopsoas_r(p,q);
            end
            if (new_iliopsoas_r(p,q) < min_iliopsoas_r(q))
                min_iliopsoas_r(q) = new_iliopsoas_r(p,q);
            end

        end
    end
end

%LEFT
for p = 1 : 1 : length(heel_strike_l) - 1 % p = amount of heelstrikes
    %resize the shape of the curves
    temp_hamstring_l = hamstring_l(heel_strike_l(p) : heel_strike_l(p+1)-1);
    new_hamstring_l(p,:) = imresize(temp_hamstring_l, [1,length(last_cycle_time_l)]);
    
    temp_glutmax_l = glutmax_l(heel_strike_l(p) : heel_strike_l(p+1)-1);
    new_glutmax_l(p,:) = imresize(temp_glutmax_l, [1,length(last_cycle_time_l)]);
    
    temp_iliopsoas_l = iliopsoas_l(heel_strike_l(p) : heel_strike_l(p+1)-1);
    new_iliopsoas_l(p,:) = imresize(temp_iliopsoas_l, [1,length(last_cycle_time_l)]);
    
    for q = 1 : 1 : length(last_cycle_time_l) %q = array of values for one step
        
        if (p == 1)
            max_hamstring_l(q) = new_hamstring_l(p,q);
            min_hamstring_l(q) = new_hamstring_l(p,q);
            
            max_glutmax_l(q) = new_glutmax_l(p,q);
            min_glutmax_l(q) = new_glutmax_l(p,q);
            
            max_iliopsoas_l(q) = new_iliopsoas_l(p,q);
            min_iliopsoas_l(q) = new_iliopsoas_l(p,q);
        else
            %if ((r >= length(max_hamstring_r)) || (q >= length(hamstring_r)))
                %break 
            %end            
            %hamstrings
            if (new_hamstring_l(p,q) > max_hamstring_l(q))
                max_hamstring_l(q) = new_hamstring_l(p,q);
            end
            if (new_hamstring_l(p,q) < min_hamstring_l(q))
                min_hamstring_l(q) = new_hamstring_l(p,q);
            end
            %gluteus
            if (new_glutmax_l(p,q) > max_glutmax_l(q))
                max_glutmax_l(q) = new_glutmax_l(p,q);
            end
            if (new_glutmax_l(p,q) < min_glutmax_l(q))
                min_glutmax_l(q) = new_glutmax_l(p,q);
            end
            %iliopsoas
            if (new_iliopsoas_l(p,q) > max_iliopsoas_l(q))
                max_iliopsoas_l(q) = new_iliopsoas_l(p,q);
            end
            if (new_iliopsoas_l(p,q) < min_iliopsoas_l(q))
                min_iliopsoas_l(q) = new_iliopsoas_l(p,q);
            end

        end
    end
end

%time vector
gait_r = 1:1:length(last_cycle_time_r);
gait_r = gait_r(:)/gait_r(length(gait_r));
shit_r = [gait_r'*100, fliplr(gait_r'*100)];

gait_l = 1:1:length(last_cycle_time_l);
gait_l = gait_l(:)/gait_l(length(gait_l));
shit_l = [gait_l'*100, fliplr(gait_l'*100)];



%% PLOT

%choose colors
str = '#009900';
color_right = sscanf(str(2:end),'%2x%2x%2x',[1 3])/255;
str = '#0000FF';
color_left = sscanf(str(2:end),'%2x%2x%2x',[1 3])/255;

%hamstrings
figure();
hold on

plot(gait_r*100,last_cycle_ham_r,'LineWidth',2,'color',color_right) % original position
inBetween_right = [max_hamstring_r, fliplr(min_hamstring_r)];
plot_right = fill(shit_r ,inBetween_right, color_right,'LineStyle','none','HandleVisibility','off');
set(plot_right,'facealpha',.1)

plot(gait_l*100,last_cycle_ham_l,'LineWidth',2,'color',color_left) % original position
inBetween_left = [max_hamstring_l, fliplr(min_hamstring_l)];
plot_left = fill(shit_l ,inBetween_left, color_left,'LineStyle','none','HandleVisibility','off');
set(plot_left,'facealpha',.1)

xlabel('% gait cycle','fontsize',20) 
ylabel('Muscle Activation','fontsize',20)
ax = gca;
set(gca,'FontSize',20)

legend('Hamstrings activation of the motorized leg','Hamstrings activation of the non-motorized leg')

hold off

%gluteus
figure();
hold on

plot(gait_r*100,last_cycle_glut_r,'LineWidth',2,'color',color_right) % original position
inBetween_right = [max_glutmax_r, fliplr(min_glutmax_r)];
plot_right = fill(shit_r ,inBetween_right, color_right,'LineStyle','none','HandleVisibility','off');
set(plot_right,'facealpha',.1)

plot(gait_l*100,last_cycle_glut_l,'LineWidth',2,'color',color_left) % original position
inBetween_left = [max_glutmax_l, fliplr(min_glutmax_l)];
plot_left = fill(shit_l ,inBetween_left, color_left,'LineStyle','none','HandleVisibility','off');
set(plot_left,'facealpha',.1)

xlabel('% gait cycle','fontsize',20) 
ylabel('Muscle Activation','fontsize',20)
ax = gca;
set(gca,'FontSize',20)

legend('Gluteus Maximus activation of the motorized leg','Gluteus Maximus activation of the non-motorized leg')

hold off

%iliopsoas
figure();
hold on

plot(gait_r*100,last_cycle_ili_r,'LineWidth',2,'color',color_right) % original position
inBetween_right = [max_iliopsoas_r, fliplr(min_iliopsoas_r)];
plot_right = fill(shit_r ,inBetween_right, color_right,'LineStyle','none','HandleVisibility','off');
set(plot_right,'facealpha',.1)

plot(gait_l*100,last_cycle_ili_l,'LineWidth',2,'color',color_left) % original position
inBetween_left = [max_iliopsoas_l, fliplr(min_iliopsoas_l)];
plot_left = fill(shit_l ,inBetween_left, color_left,'LineStyle','none','HandleVisibility','off');
set(plot_left,'facealpha',.1)

xlabel('% gait cycle','fontsize',20) 
ylabel('Muscle Activation','fontsize',20)
ax = gca;
set(gca,'FontSize',20)

legend('Iliopsoas activation for of motorized leg','Iliopsoas activation of the non-motorized leg')

hold off