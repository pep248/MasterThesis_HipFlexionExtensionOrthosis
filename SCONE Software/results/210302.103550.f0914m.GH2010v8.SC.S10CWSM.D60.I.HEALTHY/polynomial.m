clear;
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

% start of the cycle (we suppose it starts at 0.2
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


new_y = y;
new_x = x(:) - x(1);



    
cycle_length = ending-starting + 1 ;



%interesting points
d = length(new_x);
interesting(1) =1;
l_inter = length(interesting);

for i = 2:1:d-1
    if(((new_y(i-1) > new_y(i)) && (new_y(i) < new_y(i+1))) || ((new_y(i-1) < new_y(i)) && (new_y(i) > new_y(i+1))) || (i - interesting(l_inter)>11))
        l_inter = length(interesting);
        interesting(l_inter+1)=i;
        l_inter = length(interesting);
    end
end

l_inter = length(interesting);
interesting(l_inter+1)=d;



l_inter = length(interesting);

for k = 1:1:l_inter
    Y(k,1) = hip_flexion_r(starting+interesting(k));
end

X(l_inter,l_inter)= 0;

for j = 1:1:l_inter
    for i = 1:1:l_inter
        X(j,i) = new_x(interesting(j))^(i-1);
    end
end

Z=inv(X)*Y;

syms x

eq = 0;

for k = 1:1:length(Z)
   eq = eq + Z(k)*x^(k-1);
end



for  m = 1:1:length(new_x)
    new_new_y(m) = double(subs (eq,x,new_x(m)));
end

hold on
plot(new_x(interesting),new_y(interesting))
plot(new_x,new_y)
%plot(new_x,new_new_y)
hold off

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