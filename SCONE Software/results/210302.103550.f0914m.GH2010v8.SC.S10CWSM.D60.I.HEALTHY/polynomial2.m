%clear previous values
clear;

%read the excel file
table = readtable('Torque_right.xlsx');
array = table2array(table);
l = length(array);
d = length((fix(l/10)):1:l); %ignore the first 10% of values

%we declare the arrays where to store the read values
time(d) = 0;
hip_flexion_r(d) = 0;
hip_flexion_l(d) = 0;

% ignore the first 10% of the simulation
for i = (fix(l/10)):1:l
   time(i-(fix(l/10))+1)= array(i,1);
   hip_flexion_r(i-(fix(l/10))+1)= array(i,2);
   %hip_flexion_l(i-(fix(l/10))+1)= array(i,3);
end

l = length(time);
ln = (fix((l-1)/3));

% look for the "beginning" of a cycle
value = -20; %value that we know that it will only be crossed twice per cycle
i=1;
while hip_flexion_r(i) > value
i = i+1;
end

while hip_flexion_r(i) < value
i = i+1;
end

% start of the cycle (we suppose it starts at value)
starting = i; %first iteration of the first cycle

% 1 cycle
while hip_flexion_r(i) > value
i = i+1;
end

while hip_flexion_r(i) < value
i = i+1;
end

ending = i; %last iteration of the first cycle

% we follow all the wave, counting wave cycles
n = 1; %number of cycles
while i <= l
    while hip_flexion_r(i) > -20
        i = i+1;
        if (i >= l)
            break;
        end
    end

    while hip_flexion_r(i) < -20
        i = i+1;
        if (i >= l)
            break;
        end
    end
    
    if (i >= l)
        break;
    else
        last = i; %last iteration of a complete cycle
        n = n+1;
    end
    
end

% properties of the wave
total_time = time(last) - time(starting); %duration of the wave in secons
period = total_time / n; %period of the wave
f = 1/period; %frequency of the wave

% hole wave reshaping (to eliminate the initial values)
filtered_time = time(starting:last);
filtered_time = filtered_time - filtered_time(1);
filtered_hip_flexion_r = hip_flexion_r(starting:last);

%we assign the values to be plotted to x and y
for i = starting:1:ending 
   y(i-starting+1) = hip_flexion_r(i);
   x(i-starting+1) = time(i); 
end

%we make the first point of y to start at 0
new_y = y';
new_x = x(:) - x(1); %by substrating the initial time, to all values

%interesting points
d = length(new_x);
interesting(1) = 1; % each of the points inside a loop that are interesting for us starting with the first one
l_inter = length(interesting);

%criteria for an interesting point (peak or a valley)
for i = 2:1:d-1
    if(((new_y(i-1) > new_y(i)) && (new_y(i) < new_y(i+1))) || ((new_y(i-1) < new_y(i)) && (new_y(i) > new_y(i+1))) )
    % if(((new_y(i-1) > new_y(i)) && (new_y(i) < new_y(i+1))) || ((new_y(i-1) < new_y(i)) && (new_y(i) > new_y(i+1))) || (i - interesting(l_inter)>11))
        l_inter = length(interesting);
        interesting(l_inter+1)=i;
        l_inter = length(interesting);
    end
end

l_inter = length(interesting);
interesting(l_inter+1)=d; %we add as an interesting point, the last point of the cycle
l_inter = length(interesting);
c_interesting = num2str(interesting(:));

%list = {c_interesting(:)};
%[indx,tf] = listdlg('ListString',list);

% We create an array of Y values for each interesting point
Y(interesting) = 0;
for k = 1:1:l_inter
    Y(k,1) = hip_flexion_r(starting+interesting(k));
end

% We create a square X matrix for each (x·X size)
X(l_inter,l_inter)= 0;
for j = 1:1:l_inter
    for i = 1:1:l_inter
        X(j,i) = new_x(interesting(j))^(i-1);
    end
end

% Z contains the coeficients of the polinomic equation (A + B·x + C·x^2...)
Z=inv(X)*Y;

% we rebuilt the polynomial equation
syms t
eq = 0;
for k = 1:1:length(Z)
   eq = eq + Z(k)*t^(k-1);
end

% we compute the y value with the new polynomial
for  m = 1:1:length(new_x)
    new_new_y(m) = double(subs (eq,t,new_x(m)));
end

%polynomial according to 
p = polyfit(new_x,new_y,30);
f1 = polyval(p,new_x);

%plot each equation
hold on
plot(new_x(interesting),new_y(interesting)) % original one
plot(new_x,new_y) % interesting points
%plot(new_x,new_new_y) % found using the polinomyal
plot(new_x,f1,'r--')
hold off


%{
for k = 1:1:ln*3+1
    eqan(k) = eqn;
    eqan(k) = subs( eqan(k),t,time(k+starting) );
    % eqn_r(k) = eqan(k) == hip_flexion_r(k+starting);
    % eqn_l(k) = eqan(k) == hip_flexion_l(k+starting);
    eqn_r(k) = hip_flexion_r(k+starting) - eqan(k);
    eqn_l(k) = hip_flexion_l(k+starting) - eqan(k);
end
assume(A0<1);
assumeAlso(A0>-1);


%sol = vpasolve(eqn_r, [A0, A, B, fase]);
options = optimoptions('fsolve','Display','iter');
my_fun = @(A0,A,B,fase)eqn_r
[x,fval] = fsolve(my_fun,0,options)
%}