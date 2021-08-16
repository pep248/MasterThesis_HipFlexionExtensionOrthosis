function walki_spline_traj_interpolation_sitting()
%SPLINE_TRAJ_INTERPOLATION Interpolate points and generate C-array.
%   Interpolates the given points using piecewise polynomials, then generates
%   the C-array to be copy-pasted in the WalkiBBB code.
clc
clear
close all

mode = 'standing_sofa'; % <<< Choose desired trajectory here.
 

switch mode
    
    case 'standing_sofa'
        initial_knee_angle = 95;
        
        knee_perc  = [0 10 20 30 40 50 60 70 80 90 100]; % [%].
        slope = -initial_knee_angle/max(knee_perc);
        knee_angle = slope .* knee_perc + initial_knee_angle; % [deg].
        
      
        hip_angle = [105 100 95 90 85 80 70 55 40 22 5]; % [deg].
        hip_perc  = [0 10 20 30 40 50 60 70 80 90 100]; % [%].

        % close all;
        hFig = figure('Name', 'Standing stool trajectories');
        figDisplay = figure('Name', 'Visualisation of the trajectories');

        % Hip.
        [coefs, hip_interp] = generate_traj(hip_perc, hip_angle, 0, hFig, 'Hip');
        print_c_code(hip_perc, coefs, 'Hip.', ...
            'standingSofaPercentsHip', 'standingSofaTrajCoefsHip');
        
        % Knee.
        [coefs, knee_interp] = generate_traj(knee_perc, knee_angle, 0, hFig, 'Knee');
        print_c_code(knee_perc, coefs, 'Knee.', ...
            'standingSofaPercentsKnee', 'standingSofaTrajCoefsKnee');
        
    case 'standing_stool'
        initial_knee_angle = 85; 
        
        knee_perc  = [0 10 20 30 40 50 60 70 80 90 100]; % [%].
        slope = -initial_knee_angle/max(knee_perc);
        knee_angle = slope .* knee_perc + initial_knee_angle; % [deg].
        
        hip_angle = [95 90 85 80 75 65 55 45 35 25 15]; % [deg].
        hip_perc  = [0 10 20 30 40 50 60 70 80 90 100]; % [%].

        % close all;
        hFig = figure('Name', 'Standing stool trajectories');
        figDisplay = figure('Name', 'Visualisation of the trajectories');
        
        % Hip.
        [coefs, hip_interp] = generate_traj(hip_perc, hip_angle, 0, hFig, 'Hip');
        print_c_code(hip_perc, coefs, 'Hip.', ...
            'standingStoolPercentsHip', 'standingStoolTrajCoefsHip');
        
        % Knee.
        [coefs, knee_interp] = generate_traj(knee_perc, knee_angle, 0, hFig, 'Knee');
        print_c_code(knee_perc, coefs, 'Knee.', ...
            'standingStoolPercentsKnee', 'standingStoolTrajCoefsKnee');
        
    case 'standing_squatting'
        initial_knee_angle = 120;
        
        knee_perc  = [0 10 20 30 40 50 60 70 80 90 100]; % [%].
        slope = -initial_knee_angle/max(knee_perc);
        knee_angle = slope .* knee_perc + initial_knee_angle; % [deg].
        
   
        hip_angle = [110 100 90 80 75 65 55 45 35 25 15]; % [deg].
        hip_perc  = [0 10 20 30 40 50 60 70 80 90 100]; % [%].
        
        % close all;
        hFig = figure('Name', 'Standing stool trajectories');
        figDisplay = figure('Name', 'Visualisation of the trajectories');
        
        % Hip.
        [coefs, hip_interp] = generate_traj(hip_perc, hip_angle, 0, hFig, 'Hip');
        print_c_code(hip_perc, coefs, 'Hip.', ...
            'standingStoolPercentsHip', 'standingStoolTrajCoefsHip');
        
        % Knee.
        [coefs, knee_interp] = generate_traj(knee_perc, knee_angle, 0, hFig, 'Knee');
        print_c_code(knee_perc, coefs, 'Knee.', ...
            'standingStoolPercentsKnee', 'standingStoolTrajCoefsKnee');



end


display_traj(hip_interp, knee_interp, mode,figDisplay, 'Stick Figure')


end


function [coefs, interp] = generate_traj(stride_perc, joint_angle, cyclic, ...
    hFig, name)

%% Fit.
[xData, yData] = prepareCurveData(stride_perc, joint_angle);

if cyclic
    xData = [xData(1:end-1)-100
        xData(1:end-1)
        xData(1:end-1)+100];
    yData = repmat(yData(1:end-1), 3, 1);
else
    xData = [xData(1:end-1)-xData(end)
        xData(1:end-1)
        xData(1:end)+xData(end)];
    yData = [ones(length(stride_perc)-1,1)*yData(1)
        yData(1:end-1)
        ones(length(stride_perc),1)*yData(end)];
end

[fitresult, ~] = fit(xData, yData, 'splineinterp');


%% Plot.
figure(hFig);
perc_interp = linspace(0, 100, 1000);
plot(perc_interp, fitresult(perc_interp)); % Interpolated function.
hold('on');
plot(stride_perc, joint_angle, '*'); % Original points.

le = legend;
% legend([le.String {[name ' (interpolated) [deg]'] ...
%     [name ' (original points) [deg]']}]);

xlabel('Gait stride [%]');
ylabel('Joint angle [deg]');

%% Extract the relevant coefficients.
len = length(stride_perc);
coefs = fitresult.p.coefs(len:len+len-2, :);

interp = fitresult(perc_interp);

% %% Test interpolation and plot the result.
% stride_perc_interp = linspace(0, 100, 1000);
% hip_angle_interp = zeros(size(stride_perc_interp));
%
% for i=1:length(stride_perc_interp)
%     [j,~] = get_bounding_indices(stride_perc, stride_perc_interp(i));
%
%     hip_angle_interp(i) = polyval(coefs(j,:), stride_perc_interp(i)-stride_perc(j));
% end
%
% prevFig = gcf;
% figure('Name', 'Interpolation formula test');
% plot(stride_perc, joint_angle, '*', stride_perc_interp, hip_angle_interp);
% figure(prevFig);

end

function print_c_code(stride_perc, coefs, var_comment, stride_var_name, ...
    coefs_var_name)

fprintf(['// ' var_comment '\nconst std::vector<std::vector<float>> ' ...
    coefs_var_name ' = {\n']);

for i=1:size(coefs, 1)
    fprintf('\t{ ');
    for j=1:size(coefs,2)-1
        fprintf('%ff, ', coefs(i,j));
    end
    
    if i < size(coefs, 1)
        fprintf('%ff },\n', coefs(i,end));
    else
        fprintf('%ff }\n', coefs(i,end));
    end
end

fprintf('};\n\n');

fprintf(['const std::vector<float> ' stride_var_name ' = { ']);
for i=1:length(stride_perc)
    if i < length(stride_perc)
        fprintf('%ff, ', stride_perc(i));
    else
        fprintf('%ff };\n', stride_perc(i));
    end
end
fprintf('\n\n');

end

function [prevIndex, nextIndex] = get_bounding_indices(array, value)

prevIndex = 0;
nextIndex = 0;

for i=1:length(array)-1
    if array(i) <= value && array(i+1) >= value
        prevIndex = i;
        nextIndex = i+1;
        break;
    end
end

if prevIndex == 0 && nextIndex == 0
    error(['Error, ' num2str(value) '\n' mat2str(array)]);
end
end



function display_traj(hip_interp, knee_interp,mode,hfig, name)

dsmp_hip_interp = downsample(hip_interp,20);
dsmp_knee_interp = downsample(knee_interp,20);


%% Calculate foot locus
L_thigh = 360; % [mm] from hip to knee
L_shank = 490; % [mm] from knee to ground
L_trunk = 550; % [mm] from hip to head

L_foot = 240;
hor_offset = 0;
offset =0:20/(length(dsmp_hip_interp)-1):20;
offset=fliplr(offset)';

knee_pos_stance.X = L_shank*ones(length(dsmp_hip_interp),1).*sind(offset);
knee_pos_stance.Y = L_shank*ones(length(dsmp_hip_interp),1).*cosd(offset);

foottip_pos_stance.X =  L_foot.*ones(length(dsmp_hip_interp),1).*cosd(offset);
foottip_pos_stance.Y = -L_foot.*ones(length(dsmp_hip_interp),1).*sind(offset);
hip_pos_stance.X = knee_pos_stance.X - sind(180-dsmp_knee_interp+offset) .* L_thigh;
hip_pos_stance.Y = knee_pos_stance.Y - cosd(180-dsmp_knee_interp + offset) .* L_thigh;

trunk_pos_stance.X = hip_pos_stance.X + sind(dsmp_hip_interp - dsmp_knee_interp+offset).*L_trunk;
trunk_pos_stance.Y = hip_pos_stance.Y + cosd(dsmp_hip_interp  - dsmp_knee_interp +offset).*L_trunk;

figure(hfig)

for i = 1:length(dsmp_hip_interp)
    
        legX_stance = [foottip_pos_stance.X(i);
                       0;
                       knee_pos_stance.X(i);
                       hip_pos_stance.X(i);
                       trunk_pos_stance.X(i)];
    
        legY_stance = [foottip_pos_stance.Y(i)
                       0
                       knee_pos_stance.Y(i);
                       hip_pos_stance.Y(i);
                       trunk_pos_stance.Y(i)];
    
    subplot(211);
    hold off
    plot(legX_stance, legY_stance,'linewidth',4);
%     hold on
%     plot(legX_swing, legY_swing,'linewidth',4);
%     
%     plot(heel_pos_swing.X(length(dsmp_hip_interp)/2+1:length(dsmp_hip_interp)/2+i)+hip_pos_stance.X(1:i) + hor_offset,...
%         hip_pos_stance.Y(1:i) + heel_pos_swing.Y(length(dsmp_hip_interp)/2+1:length(dsmp_hip_interp)/2+i)+vert_offset,'linewidth',2)
%     plot([-1500 1500], [0 0], 'linewidth', 1.5)
    
%     if strcmp(mode,'stairs_second_step') || strcmp(mode, 'stairs_first_step') || strcmp(mode, 'fast_stairs')
%         plot([0 0],[0 vert_offset],'k',[0 L_stair],[vert_offset vert_offset],'k',...
%             [L_stair L_stair],[vert_offset 2*vert_offset],'k', [L_stair 2*L_stair],[2*vert_offset 2*vert_offset],'k','linewidth',2);
%     elseif strcmp(mode,'slope') || strcmp(mode, 'first_step_backward_slope') || strcmp(mode, 'slope_first_step')
%         plot([0 1000],[0 tand(20)*1000],'k','linewidth',2)
%         plot([1000 1500], [tand(20)*1000 tand(20)*1000],'k','linewidth',2)
%     end
%     
    ylim([-100 1600]);
    axis equal
    
    subplot(212);
    hold off
    plot(1:i,dsmp_hip_interp(1:i),'linewidth',2);
    hold on
    plot(1:i,dsmp_knee_interp(1:i),'linewidth',2);
    xlim([0 50]);
    le = legend;
%     legend([le.String {'Hip[deg]' ...
%         'Knee[deg]'}]);
    
    
    
    pause(0.05)
    
end

end

