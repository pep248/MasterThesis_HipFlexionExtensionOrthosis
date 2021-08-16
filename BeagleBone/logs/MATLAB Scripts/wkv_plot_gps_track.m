function wkv_plot_gps_track(wkv)
%WKV_PLOT_GPS_TRACK Plots the GPS track from a WKV.

points = geopoint(wkv_get(wkv, 'sensors/gps/latitude'), ...
                  wkv_get(wkv, 'sensors/gps/longitude'));
              
points = points(points.Latitude ~= 0 | points.Longitude ~= 0);

webmap('World Imagery');
wmline(points);

end

