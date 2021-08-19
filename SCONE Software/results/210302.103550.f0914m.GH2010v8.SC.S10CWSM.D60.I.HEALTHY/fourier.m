table = readtable('hip_flexion_gait2.xlsx');
array = table2array(table);
l = length(array);

f = 0.13993;

time(l) = 0;
hip_flexion_r(l) = 0;
hip_flexion_l(l) = 0;

for i = (fix(l/10)):1:l
   time(i)= array(i,1);
   hip_flexion_r(i)= array(i,2);
   hip_flexion_l(i)= array(i,3);
end

l = length(time);
ln = (fix((l-1)/3));

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
    eqan(k) = subs( eqan(k),t,time(k) );
    eqn_r(k) = eqan(k) - hip_flexion_r(k);
    eqn_l(k) = eqan(k) - hip_flexion_l(k);
end

%sol = solve(eqn_r, [A0, A, B, fase]);
options = optimoptions('fsolve','Display','iter');
[x,fval] = fsolve(eqn_r,0,options)