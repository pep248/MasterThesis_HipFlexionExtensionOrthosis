-- File where to change the actuator inputs

function init( model, par )
	

	previous_heel_ground_reaction_force_right = 0
	current_heel_ground_reaction_force_right = 0

	previous_heel_ground_reaction_force_left = 0
	current_heel_ground_reaction_force_left = 0
	buffer_heel_left = 0

	previous_toe_ground_reaction_force_right = 0
	current_toe_ground_reaction_force_right = 0

	previous_toe_ground_reaction_force_left = 0
	current_toe_ground_reaction_force_left = 0
	buffer_toe_left = 0

	buffer_heel_heigh_right = 0
	heel_heigh_right = 0.1
	buffer_toe_heigh_right = 0
	toe_heigh_right = 0.1

	buffer_heel_heigh_left = 0
	heel_heigh_left = 0.1
	buffer_toe_heigh_left = 0
	toe_heigh_left = 0.1

	state_force = 1 -- variable used for the state machines
	first_step = 0

	strike_error = 0
	strike = 0

end



function update( model )
	-- GATHER DATA
	-- update right heel (force)
	previous_heel_ground_reaction_force_right = current_heel_ground_reaction_force_right
	buffer_heel_right = model:find_body("calcn_r"):contact_force()
	current_heel_ground_reaction_force_right = buffer_heel_right.y

	buffer_heel_heigh_right = model:find_body("calcn_r"):com_pos()
	heel_heigh_right = buffer_heel_heigh_right.y

	buffer_toe_heigh_right = model:find_body("toes_r"):com_pos()
	toe_heigh_right = buffer_toe_heigh_right.y

	-- update left heel (force)
	previous_heel_ground_reaction_force_left = current_heel_ground_reaction_force_left
	buffer_heel_left = model:find_body("calcn_l"):contact_force()
	current_heel_ground_reaction_force_left = buffer_heel_left.y

	buffer_heel_heigh_left = model:find_body("calcn_l"):com_pos()
	heel_heigh_left = buffer_heel_heigh_left.y

	buffer_toe_heigh_left = model:find_body("toes_l"):com_pos()
	toe_heigh_left = buffer_toe_heigh_left.y


	-- DETECT A HEEL STRIKE USING GROUND REACTION FORCE INSTEAD OF A TOE STRIKE
	if (state_force == 0) then -- we use this state machine to aboid some unpredictable bouncing of the feet counting as 2 steps
		if ((previous_heel_ground_reaction_force_right == 0) and (current_heel_ground_reaction_force_right ~= 0)) then
			state_force = 1 -- our right heel has touched the ground, we wait for a left heel impact now
			if (heel_heigh_right >= toe_heigh_right) then
				strike_error = 1;
			end
		end	
	elseif (state_force == 1) then
		if ((previous_heel_ground_reaction_force_left == 0) and (current_heel_ground_reaction_force_left ~= 0)) then
			state_force = 0 -- our left heel has touched the ground, we wait for a right heel impact now
			if (heel_heigh_left >= toe_heigh_left) then
				strike_error = 1;
			end
		end
	end

end 

function result( model )

	if (strike_error == 0) then
		strike = 0
	elseif (strike_error == 1) then
		strike = 800
	end

	return strike
end

function store_data( frame )
	frame:set_value("strike_error", strike_error);
	frame:set_value("state_force", state_force);

	frame:set_value("toe_heigh_right", toe_heigh_right);
	frame:set_value("heel_heigh_right", heel_heigh_right);

	frame:set_value("toe_heigh_left", toe_heigh_left);
	frame:set_value("heel_heigh_left", heel_heigh_left);

end