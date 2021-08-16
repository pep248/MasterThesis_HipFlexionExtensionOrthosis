function wkv_export2gpx(wkv, filename)
%WKV_EXPORT2GPX Exports the WKV GPS track to a GPX file.

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
duplicatesIndices = (diff(latitude) == 0) & ...
                    (diff(longitude) == 0) & ...
                    (diff(elevation) == 0);
timestamp(duplicatesIndices) = [];
latitude(duplicatesIndices) = [];
longitude(duplicatesIndices) = [];
elevation(duplicatesIndices) = [];

%% Write to a GPX file.
fid = fopen(filename, 'w');

fprintf(fid, ['<?xml version="1.0" encoding="ISO-8859-1"?>\n' ...
              '<?xml-stylesheet type="text/xsl" href="details.xsl"?>\n' ...
              '<gpx\n' ...
              'version="1.0"\n' ...
              'creator="Walki wkv_export2gpx.m script"\n' ...
              'xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"\n' ...
              'xmlns="http://www.topografix.com/GPX/1/0"\n' ...
              'xmlns:topografix="http://www.topografix.com/GPX/Private/TopoGrafix/0/1" xsi:schemaLocation="http://www.topografix.com/GPX/1/0 http://www.topografix.com/GPX/1/0/gpx.xsd http://www.topografix.com/GPX/Private/TopoGrafix/0/1 http://www.topografix.com/GPX/Private/TopoGrafix/0/1/topografix.xsd">\n' ...
              '<trk>\n' ...
              '<name><![CDATA[WKV GPS track]]></name>\n' ...
              '<desc><![CDATA[]]></desc>\n' ...
              '<trkseg>\n']);

for i=1:length(timestamp)
    fprintf(fid, ['<trkpt lat="%f" lon="%f">\n'...
                  '<ele>%f</ele>\n' ...
                  '<time>%s</time>\n' ...
                  '</trkpt>\n'], ...
                  latitude(i), longitude(i), elevation(i), ...
                  datestr(timestamp(i), 'yyyy-mm-ddTHH:MM:SS.FFF'));
end

fprintf(fid, '</trkseg></trk>\n');
fprintf(fid, '</gpx>');

fclose(fid); 

end


