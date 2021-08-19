-- File where to change the actuator inputs

function init( model, par )
	state_force = 1 -- variable used for the state machines

	last_heel_strike_right = 0 -- time of the previous heel strike computed using forces
	current_heel_strike_right = 0 -- time of the current heel strike computed using forces
	last_heel_strike_left = 0 -- time of the previous heel strike computed using forces
	current_heel_strike_left = 0 -- time of the current heel strike computed using forces

	previous_ground_reaction_force_right = 10 -- used to detect a heel strike (we start at a random value different from 0)
	current_ground_reaction_force_right = 10 -- used to detect a heel strike (to avoid triggering a fake heel strike)
	previous_ground_reaction_force_left = 10 -- used to detect a heel strike (we start at a random value different from 0)
	current_ground_reaction_force_left = 10 -- used to detect a heel strike (to avoid triggering a fake heel strike)

	first_step_right = 0
	first_step_left = 0
	steps_right = 0
	steps_left = 0

	difference = 0

	angle_pos_right = {0}
	angle_pos_right_other = {0}
	angle_vel_right = {0}
	angle_pos_left = {0}
	angle_pos_left_other = {0}
	angle_vel_left = {0}

	size_right = 0
	size_left = 0
	previous_size_right = 0
	previous_size_left = 0

	time_steps_right = 0
	time_steps_right_other = 0
	time_steps_left = 0
	time_steps_left_other = 0

	hip_angle_right = 0
	hip_angle_left = 0
	hip_angle_vel_right = 0
	hip_angle_vel_left = 0
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

	-- update right step (angle)
	hip_angle_right = model:find_dof("hip_flexion_r"):position()
	hip_angle_vel_right = model:find_dof("hip_flexion_r"):velocity()

		-- update right step (angle)
	hip_angle_left = model:find_dof("hip_flexion_l"):position()
	hip_angle_vel_left = model:find_dof("hip_flexion_l"):velocity()


	-- DETECT A HEEL STRIKE USING GROUND REACTION FORCE
	if (state_force == 0) then -- we use this state machine to aboid some unpredictable bouncing of the feet counting as 2 steps
		if ((previous_ground_reaction_force_right == 0) and (current_ground_reaction_force_right ~= 0)) then
			state_force = 1 -- our right heel has touched the ground, we wait for a left heel impact now
			--time_steps_right = 1 --reset time steps
			if ( first_step_right == 0) then -- we execute this just one we wave the first heel strike
				first_step_right = 1
			end

			-- steps counter
			steps_right = steps_right + 1;
			-- time steps reset
			if(steps_right%2 == 0) then
				time_steps_right = 0;

				if (time_steps_right_other <= time_steps_left) then
					for i=0, time_steps_right_other-1 do
						difference = difference + (angle_pos_right_other[i] - angle_pos_left[i])*(angle_pos_right_other[i] - angle_pos_left[i])
					end
				else
					for i=0, time_steps_left-1 do
						difference = difference + (angle_pos_right_other[i] - angle_pos_left[i])*(angle_pos_right_other[i] - angle_pos_left[i])
					end
				end

			elseif(steps_right%2 == 1) then
				time_steps_right_other = 0;

				if (time_steps_right <= time_steps_left_other) then
					for i=0, time_steps_right-1 do
						difference = difference + (angle_pos_right[i] - angle_pos_left_other[i])*(angle_pos_right[i] - angle_pos_left_other[i])
					end
				else
					for i=0, time_steps_left_other-1 do
						difference = difference + (angle_pos_right[i] - angle_pos_left_other[i])*(angle_pos_right[i] - angle_pos_left_other[i])
					end
				end

			end

		end
	elseif (state_force == 1) then
		if ((previous_ground_reaction_force_left == 0) and (current_ground_reaction_force_left ~= 0)) then
			state_force = 0 -- our left heel has touched the ground, we wait for a right heel impact now
			time_steps_left = 1 --reset time steps
			if (first_step_left == 0) then -- we execute this just one we wave the first heel strike
				first_step_left = 1
			end
				
			-- steps counter
			steps_left = steps_left + 1;
			-- time steps reset
			if(steps_left%2 == 0) then
				time_steps_left = 0;

				if (time_steps_right_other <= time_steps_left_other) then
					for i=0, time_steps_right_other-1 do
						difference = difference + (angle_pos_right_other[i] - angle_pos_left_other[i])*(angle_pos_right_other[i] - angle_pos_left_other[i])
					end
				else
					for i=0, time_steps_left_other-1 do
						difference = difference + (angle_pos_right_other[i] - angle_pos_left_other[i])*(angle_pos_right_other[i] - angle_pos_left_other[i])
					end
				end

			elseif(steps_left%2 == 1) then
				time_steps_left_other = 0;

				if (time_steps_right <= time_steps_left) then
					for i=0, time_steps_right-1 do
						difference = difference + (angle_pos_right[i] - angle_pos_left[i])*(angle_pos_right[i] - angle_pos_left[i])
					end
				else
					for i=0, time_steps_left-1 do
						difference = difference + (angle_pos_right[i] - angle_pos_left[i])*(angle_pos_right[i] - angle_pos_left[i])
					end
				end


			end

		end
	end

	    -- Store angle values for right leg
	--if (first_step_right == 1) then -- We start after we detect the first heel-strike
		-- Switch between odd and even step
		if(steps_right%2 == 0) then
			angle_pos_right[time_steps_right] = hip_angle_right
			--angle_vel_right[time_steps_right] = hip_angle_vel_right
			time_steps_right = time_steps_right + 1;
		elseif(steps_right%2 == 1) then
			angle_pos_right_other[time_steps_right_other] = hip_angle_right
			time_steps_right_other = time_steps_right_other + 1;
		end
	--end
		-- Store angle values for left leg
	--if (first_step_left == 1) then -- We start after we detect the first heel-strike
		-- Switch between odd and even step
		if(steps_left%2 == 0) then
			angle_pos_left[time_steps_left] = hip_angle_left
			--angle_vel_left[time_steps_left] = hip_angle_vel_left
			time_steps_left = time_steps_left + 1;
		elseif(steps_left%2 == 1) then
			angle_pos_left_other[time_steps_left_other] = hip_angle_left
			time_steps_left_other = time_steps_left_other + 1;
		end
	--end

	return false; -- return false to keep going
end 

function result( model )

	if (time_steps_right >= time_steps_right_other) and (time_steps_left >= time_steps_left_other) then
		if (time_steps_right <= time_steps_left) then
			for i=0, time_steps_right-1 do
				difference = difference + (angle_pos_right[i] - angle_pos_left[i])*(angle_pos_right[i] - angle_pos_left[i])
			end
		else
			for i=0, time_steps_left-1 do
				difference = difference + (angle_pos_right[i] - angle_pos_left[i])*(angle_pos_right[i] - angle_pos_left[i])
			end
		end

	elseif (time_steps_right < time_steps_right_other) and (time_steps_left >= time_steps_left_other) then
		if (time_steps_right_other <= time_steps_left) then
			for i=0, time_steps_right_other-1 do
				difference = difference + (angle_pos_right_other[i] - angle_pos_left[i])*(angle_pos_right_other[i] - angle_pos_left[i])
			end
		else
			for i=0, time_steps_left-1 do
				difference = difference + (angle_pos_right_other[i] - angle_pos_left[i])*(angle_pos_right_other[i] - angle_pos_left[i])
			end
		end

	elseif (time_steps_right >= time_steps_right_other) and (time_steps_left < time_steps_left_other) then
		if (time_steps_right <= time_steps_left_other) then
			for i=0, time_steps_right-1 do
				difference = difference + (angle_pos_right[i] - angle_pos_left_other[i])*(angle_pos_right[i] - angle_pos_left_other[i])
			end
		else
			for i=0, time_steps_left_other-1 do
				difference = difference + (angle_pos_right[i] - angle_pos_left_other[i])*(angle_pos_right[i] - angle_pos_left_other[i])
			end
		end

	elseif (time_steps_right < time_steps_right_other) and (time_steps_left < time_steps_left_other) then
		if (time_steps_right_other <= time_steps_left_other) then
			for i=0, time_steps_right_other-1 do
				difference = difference + (angle_pos_right_other[i] - angle_pos_left_other[i])*(angle_pos_right_other[i] - angle_pos_left_other[i])
			end
		else
			for i=0, time_steps_left_other-1 do
				difference = difference + (angle_pos_right_other[i] - angle_pos_left_other[i])*(angle_pos_right_other[i] - angle_pos_left_other[i])
			end
		end
		
	end

	return difference
end

function store_data( frame )

	frame:set_value("difference", difference);
	frame:set_value("time_steps_right", time_steps_right);
	frame:set_value("time_steps_left", time_steps_left);

	frame:set_value("time_steps_right_other", time_steps_right_other);
	frame:set_value("time_steps_left_other", time_steps_left_other);

	frame:set_value("hip_angle_right", hip_angle_right);
	frame:set_value("hip_angle_left", hip_angle_left);

	frame:set_value("steps_right", steps_right);
	frame:set_value("steps_left", steps_left);

end