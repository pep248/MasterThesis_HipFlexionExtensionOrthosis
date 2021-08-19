function init( model, par )
	actuator_r = model:find_actuator( "hip_flexion_r");
	actuator_l = model:find_actuator( "hip_flexion_l");
	sum_r = 0
	sum_l = 0
	steps = 0
	target_body1 = "torso"
	body1 = model:find_body(target_body1)
	target_body2 = "torso"
	body2 = model:find_body(target_body1)
	pos_coef = 0
end

function update( model )
	body_pos_x=model:com_pos().x
	body_pos_y=model:com_pos().y
	pos_coef = math.min(100/body_pos_x,100) - body_pos_y/10
	-- pos_coef = math.min(10/body:com_pos().x,10)
	return false -- return false to keep going
end

function result( model )
	return pos_coef
end

function store_data( frame )
	frame:set_value("pelvis_position_x", pos_coef);
	scone.info( 'This is a info message!' )
end