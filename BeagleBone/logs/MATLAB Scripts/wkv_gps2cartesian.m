function [x,y,z,t] = wkv_gps2cartesian(wkv, removeDuplicates)
%WKV_GPS2CARTESIAN Projects the GPS coordinates to a plane.
%   Uses WGS84 geodetic system, and the UTM projection. Is inaccurate if
%   the GPS path spans accross several UTM zones.
%   If removeDuplicates is set to 1, then the consecutive duplicate
%   position values are removed. This is useful because the GPS data is
%   typically updated at ~1Hz.

%% Prepare the GPS data points.
timestamp = wkv(1).values;
latitude = wkv_get(wkv, 'sensors/gps/latitude');
longitude = wkv_get(wkv, 'sensors/gps/longitude');
elevation = wkv_get(wkv, 'sensors/gps/elevation');

% Remove invalid data points (e.g. no GPS fix).
invalidIndices = (latitude == 0) & (longitude == 0) & (elevation == 0);
timestamp(invalidIndices) = [];
latitude(invalidIndices) = [];
longitude(invalidIndices) = [];
elevation(invalidIndices) = [];

% Remove duplicate points.
if ~exist('removeDuplicates', 'var') || removeDuplicates
    duplicatesIndices = ([1 diff(latitude)] == 0) & ...
                        ([1 diff(longitude)] == 0) & ...
                        ([1 diff(elevation)] == 0);
    timestamp(duplicatesIndices) = [];
    latitude(duplicatesIndices) = [];
    longitude(duplicatesIndices) = [];
    elevation(duplicatesIndices) = [];
end

%% Project the polar coordinates to a cartesian space.
dczone = utmzone(mean(latitude), mean(longitude));

utmstruct = defaultm('utm'); 
utmstruct.zone = dczone;  
utmstruct.geoid = wgs84Ellipsoid;
utmstruct = defaultm(utmstruct);

[x,y,z] = mfwdtran(utmstruct, latitude, longitude, elevation);
t = timestamp;

end

