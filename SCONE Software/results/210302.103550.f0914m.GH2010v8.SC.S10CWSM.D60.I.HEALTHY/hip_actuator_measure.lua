function init( model, par )
	actuator_r = model:find_actuator( "hip_flexion_r");
	actuator_l = model:find_actuator( "hip_flexion_l");
	sum_r = 0
	sum_l = 0
	steps = 0
end

function update( model )
	sum_r = sum_r + actuator_r:input()
	sum_l = sum_l + actuator_l:input()
	steps = steps + 1
	return false -- return false to keep going
end

function result( model )
	effort = (sum_r*sum_r + sum_l*sum_l) / (2 * steps)
	return effort
end

function store_data( frame )
	frame:set_value("hip_flexion_actuator_r", actuator_r:input());
	frame:set_value("hip_flexion_actuator_l", actuator_l:input());
end
