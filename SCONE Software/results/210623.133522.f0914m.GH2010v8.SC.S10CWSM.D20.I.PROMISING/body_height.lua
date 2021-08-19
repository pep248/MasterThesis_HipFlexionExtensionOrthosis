function init( model, par )
	actuator_r = model:find_actuator( "hip_flexion_r");
	actuator_l = model:find_actuator( "hip_flexion_l");
	sum_r = 0
	sum_l = 0
	steps = 0
	--target_body1 = "torso"
	--body1 = model:find_body(target_body1)
	--target_body2 = "torso"
	--body2 = model:find_body(target_body1)
	pos_coef = 0
	body_pos_x = 0
	body_pos_y = 0
	body_y_score = 0
	temp_body_y_score = 0
end

function update( model )
	-- move to result
	body_pos_y=model:find_body("pelvis"):com_pos().y

	temp_body_y_score = (0.85 - body_pos_y)

	if (temp_body_y_score < 0) then
		body_y_score = body_y_score + temp_body_y_score * 0.1
	else
		body_y_score = body_y_score + temp_body_y_score * 1000000
	end

	

	return false -- return false to keep going
end

function result( model )

	--[[
	if (body_pos_x <= 0) then
		pos_coef = 100
	else
		pos_coef = math.min( 100/(body_pos_x*10) , 100)
	end
	--]]

	--return pos_coef
	return body_y_score
end

function store_data( frame )
	frame:set_value("pelvis_position_y", body_pos_y);
	frame:set_value("temp_body_y_score", temp_body_y_score);
	frame:set_value("body_y_score", body_y_score);
end