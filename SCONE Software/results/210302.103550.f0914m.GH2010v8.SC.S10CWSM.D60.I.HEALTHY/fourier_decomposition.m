% cftool

%{
a1 = LAR2.fittedmodel.a1;
b1 = LAR2.fittedmodel.b1;
c1 = LAR2.fittedmodel.c1;

a2 = LAR2.fittedmodel.a2;
b2 = LAR2.fittedmodel.b2;
c2 = LAR2.fittedmodel.c2;

a3 = LAR2.fittedmodel.a3;
b3 = LAR2.fittedmodel.b3;
c3 = LAR2.fittedmodel.c3;

a4 = LAR2.fittedmodel.a4;
b4 = LAR2.fittedmodel.b4;
c4 = LAR2.fittedmodel.c4;

a5 = LAR2.fittedmodel.a5;
b5 = LAR2.fittedmodel.b5;
c5 = LAR2.fittedmodel.c5;

a6 = LAR2.fittedmodel.a6;
b6 = LAR2.fittedmodel.b6;
c6 = LAR2.fittedmodel.c6;

a7 = LAR2.fittedmodel.a7;
b7 = LAR2.fittedmodel.b7;
c7 = LAR2.fittedmodel.c7;

a8 = LAR2.fittedmodel.a8;
b8 = LAR2.fittedmodel.b8;
c8 = LAR2.fittedmodel.c8;

equation = a1*sin(b1*t+c1) + a2*sin(b2*t+c2) + a3*sin(b3*t+c3) + a4*sin(b4*t+c4) + a5*sin(b5*t+c5) + a6*sin(b6*t+c6) + a7*sin(b7*t+c7) + a8*sin(b8*t+c8);
for  m = 1:1:length(new_x)
    equation_values(m) = double(subs (equation,t,filtered_time(m)));
end

hold on
plot(filtered_time,equation_values,'r');
plot(filtered_time,filtered_hip_flexion_r,'b');
hold off

%}

[fitresult, gof]=createFit2(filtered_time, filtered_hip_flexion_r);

a1 = fitresult.a1;
b1 = fitresult.b1;
c1 = fitresult.c1;

a2 = fitresult.a2;
b2 = fitresult.b2;
c2 = fitresult.c2;

a3 = fitresult.a3;
b3 = fitresult.b3;
c3 = fitresult.c3;

function_new = a1*sin(b1*t+c1) + a2*sin(b2*t+c2) + a3*sin(b3*t+c3);
yvalues = double(subs(function_new, t,filtered_time));

figure()
hold on
plot(filtered_time,yvalues)
plot(filtered_time,filtered_hip_flexion_r)
hold off

function [fitresult, gof] = poly_fit(gait_r, one_cycle_torque_r)
%% Fit: 'untitled fit 1'.
[xData, yData] = prepareCurveData( gait_r, one_cycle_torque_r );

% Set up fittype and options.
ft = fittype( 'poly4' );

% Fit model to data.
[fitresult, gof] = fit( xData, yData, ft );

% Plot fit with data.
figure( 'Name', 'untitled fit 1' );
h = plot( fitresult, xData, yData );
legend( h, 'one_cycle_torque_r vs. gait_r', 'untitled fit 1', 'Location', 'NorthEast', 'Interpreter', 'none' );
% Label axes
xlabel( 'gait_r', 'Interpreter', 'none' );
ylabel( 'one_cycle_torque_r', 'Interpreter', 'none' );
grid on
end