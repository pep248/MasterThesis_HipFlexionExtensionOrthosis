-- File where to change the actuator inputs

function init( model, par )
	actuator_r = model:find_actuator( "hip_flexion_r");
	actuator_l = model:find_actuator( "hip_flexion_l");
end

function update( model )
	
	
	actuator_r:add_input(0 * model:time());
	actuator_l:add_input(0 * model:time());
	
	-- scone.debug( "R " .. actuator_r:name() );
	-- scone.debug( "L " .. actuator_l:name() );

	return false; -- return false to keep going
end