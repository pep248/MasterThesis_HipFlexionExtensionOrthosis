table = readtable('hip_flexion_gait2.xlsx');
array = table2array(table);
l = length(array);
d = length((fix(l/10)):1:l);
f = 0.13993;

time(d) = 0;
hip_flexion_r(d) = 0;
hip_flexion_l(d) = 0;

% ignore the first 10% of the simulation
for i = (fix(l/10)):1:l
   time(i-(fix(l/10))+1)= array(i,1);
   hip_flexion_r(i-(fix(l/10))+1)= array(i,2);
   hip_flexion_l(i-(fix(l/10))+1)= array(i,3);
end

l = length(time);
ln = (fix((l-1)/3));

% look for the "beginning" of a cycle
i=1;
while hip_flexion_r(i) > 0.2
i = i+1;
end

while hip_flexion_r(i) < 0.2
i = i+1;
end

% start of the cycle
starting = i;

% 1 cycle
while hip_flexion_r(i) > 0.2
i = i+1;
end

while hip_flexion_r(i) < 0.2
i = i+1;
end

ending = i;

% number of cycles
n = 1;
while i <= l
    while hip_flexion_r(i) > 0.2
        i = i+1;
        if (i >= l)
            break;
        end
    end

    while hip_flexion_r(i) < 0.2
        i = i+1;
        if (i >= l)
            break;
        end
    end
    
    if (i >= l)
        break;
    else
        last = i;
        n = n+1;
    end
    
end

total_time = time(last) - time(starting);
period = total_time / n;
f = 1/period



for i = starting:1:ending 
   y(i-starting+1) = hip_flexion_r(i);
   x(i-starting+1) = time(i); 
end

t = time(starting):0.1:time(ending);
a = sin(2*pi*f*t);
hold on
plot(t,a)

plot(x,y)
hold off
    
cycle_length = ending-starting;
ln = (fix((cycle_length-1)/3));


syms A0 
syms A [1 ln] 
syms B [1 ln] 
syms fase [1 ln]
syms t

eqn=A0/2;

for j = 1:1:ln
    eqn= eqn + A(j)*sin(2 * i * pi * f * t + fase(j) );
    eqn= eqn + B(j)*cos(2 * i * pi * f * t + fase(j) );
end

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