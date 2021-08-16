function [hr, t_hr] = wkv_heartrate_from_rr(wkv)
%WKV_HEARTRATE_FROM_RR HR signal from the RR-intervals.
%   Computes the heartbeat rate from the RR-intervals. This gives a higher
%   time resolution the HR computed by the sensor, but also a higher noise.

%% Extract the heartbeat rate data.
t = wkv_get(wkv, 'timestamp');
hr_base = wkv_get(wkv, 'sensors/heartbeat_sensor/heartbeat_rate');
rr1 = wkv_get(wkv, 'sensors/heartbeat_sensor/rr_interval_1');
rr2 = wkv_get(wkv, 'sensors/heartbeat_sensor/rr_interval_2');
rr3 = wkv_get(wkv, 'sensors/heartbeat_sensor/rr_interval_3');
rr4 = wkv_get(wkv, 'sensors/heartbeat_sensor/rr_interval_4');

%% Remove the duplicates.
duplicatesIndices = ([1 diff(hr_base)] == 0) & ...
                    ([1 diff(rr1)] == 0) & ...
                    ([1 diff(rr2)] == 0) & ...
                    ([1 diff(rr3)] == 0) & ...
                    ([1 diff(rr4)] == 0);
                
t(duplicatesIndices) = [];
rr1(duplicatesIndices) = [];
rr2(duplicatesIndices) = [];
rr3(duplicatesIndices) = [];
rr4(duplicatesIndices) = [];

%%
t_hr = [];
hr = [];

for i = 2:length(t)
    
    if rr1(i) < 10000 && rr2(i) < 10000 && rr3(i) < 10000 && rr4(i) < 10000
        dt = (t(i)-t(i-1)) / 4;
        t_hr = [t_hr  t(i-1)+1*dt t(i-1)+2*dt t(i-1)+3*dt t(i)]; %#ok<AGROW>
        hr = [hr 1024/rr1(i) 1024/rr2(i) 1024/rr3(i) 1024/rr4(i)]; %#ok<AGROW>
    elseif rr1(i) < 10000 && rr2(i) < 10000 && rr3(i) < 10000
        dt = (t(i)-t(i-1)) / 3;
        t_hr = [t_hr t(i-1)+1*dt t(i-1)+2*dt t(i)]; %#ok<AGROW>
        hr = [hr 1024/rr1(i) 1024/rr2(i) 1024/rr3(i)]; %#ok<AGROW>
    elseif rr1(i) < 10000 && rr2(i) < 10000
        dt = (t(i)-t(i-1)) / 2;
        t_hr = [t_hr t(i-1)+dt t(i)]; %#ok<AGROW>
        hr = [hr 1024/rr1(i) 1024/rr2(i)]; %#ok<AGROW>
    elseif rr1(i) < 10000
        t_hr = [t_hr t(i)]; %#ok<AGROW>
        hr = [hr 1024/rr1(i)]; %#ok<AGROW>
    end

end

hr = hr * 60;

end