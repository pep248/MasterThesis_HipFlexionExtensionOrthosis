-- File where to change the actuator inputs

function init( model, par )
	state_force = 0 -- variable used for the state machines

	last_heel_strike_right = 0 -- time of the previous heel strike computed using forces
	current_heel_strike_right = 0 -- time of the current heel strike computed using forces
	last_heel_strike_left = 0 -- time of the previous heel strike computed using forces
	current_heel_strike_left = 0 -- time of the current heel strike computed using forces

	previous_ground_reaction_force_right = 10 -- used to detect a heel strike (we start at a random value different from 0)
	current_ground_reaction_force_right = 10 -- used to detect a heel strike (to avoid triggering a fake heel strike)
	previous_ground_reaction_force_left = 10 -- used to detect a heel strike (we start at a random value different from 0)
	current_ground_reaction_force_left = 10 -- used to detect a heel strike (to avoid triggering a fake heel strike)

	step_force = 0 --counter of steps using the ground force information

	right_duration = 1.4 --invented step duration
	left_duration = 1.4 --invented step duration

	difference = 0

end



function update( model )
	-- GATHER DATA
	-- update right step (force)
	previous_ground_reaction_force_right = current_ground_reaction_force_right
	buffer_right = model:find_body("calcn_r"):contact_force()
	current_ground_reaction_force_right = buffer_right.y

	-- update left step (force)
	previous_ground_reaction_force_left = current_ground_reaction_force_left
	buffer_left = model:find_body("calcn_l"):contact_force()
	current_ground_reaction_force_left = buffer_left.y


	-- DETECT A HEEL STRIKE USING GROUND REACTION FORCE
	if (state_force == 0) then -- we use this state machine to aboid some unpredictable bouncing of the feet counting as 2 steps
		if ( (previous_ground_reaction_force_right == 0) and (current_ground_reaction_force_right ~= 0) ) then
			last_heel_strike_right = current_heel_strike_right
			current_heel_strike_right = model:time();
			right_duration = current_heel_strike_right - last_heel_strike_right

			state_force = 1 -- our right heel has touched the ground, we wait for a left heel impact now
		end
	elseif (state_force == 1) then	
		if ( (previous_ground_reaction_force_left == 0) and (current_ground_reaction_force_left ~= 0) ) then
			last_heel_strike_left = current_heel_strike_left
			current_heel_strike_left = model:time();
			left_duration = current_heel_strike_left - last_heel_strike_left

			state_force = 0 -- our left heel has touched the ground, we wait for a right heel impact now
		end
	end

	difference = (right_duration - left_duration) * (right_duration - left_duration)

	return false; -- return false to keep going
end 

function result( model )
	return difference
end

function store_data( frame )
	frame:set_value("right_duration", right_duration);
	frame:set_value("left_duration", left_duration);
	frame:set_value("difference", difference);
end