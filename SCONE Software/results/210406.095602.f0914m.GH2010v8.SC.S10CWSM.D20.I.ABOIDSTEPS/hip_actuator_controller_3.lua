-- File where to change the actuator inputs

function init( model, par )
	actuator_r = model:find_actuator( "hip_flexion_r");
	actuator_l = model:find_actuator( "hip_flexion_l");


	target_body = "calcn_r"
	body = model:find_body(target_body)


	

		--[[ FOURIER
	if ( scone.a1 and scone.b1 and scone.c1 and scone.a2 and scone.b2 and scone.c2 and scone.a3 and scone.b3 and scone.c3 and scone.time_offset) then
		
		time_offset = par:create_from_string( "time_offset", scone.time_offset )

		a1 = par:create_from_string( "a1", scone.a1 )
		b1 = par:create_from_string( "b1", scone.b1 )
		c1 = par:create_from_string( "c1", scone.c1 )

		a2 = par:create_from_string( "a2", scone.a2 )
		b2 = par:create_from_string( "b2", scone.b2 )
		c2 = par:create_from_string( "c2", scone.c2 )

		a3 = par:create_from_string( "a3", scone.a3 )
		b3 = par:create_from_string( "b3", scone.b3 )
		c3 = par:create_from_string( "c3", scone.c3 )

		--]]

		--[[ POLYNOMIAL
		--]]
	if ( scone.p1 and scone.p2 and scone.p3 and scone.p4 and scone.time_offset and scone.period) then
		
		time_offset = par:create_from_string( "time_offset", scone.time_offset )

		period = par:create_from_string( "period", scone.period ) --period of the healthy gait (constant)

		p0 = par:create_from_string( "p0", scone.p0 )
		p1 = par:create_from_string( "p1", scone.p1 )
		p2 = par:create_from_string( "p2", scone.p2 )
		p3 = par:create_from_string( "p3", scone.p3 )
		p4 = par:create_from_string( "p4", scone.p4 )
	else
		-- produce an error and abort the simulation
		error( "Must set coeficient parameters!" )
	end

	current_period = period -- duration of the last step
	control_ratio = 0.1 -- feedback constant

	state_angle = 0 -- variables used for the state machines
	state_force = 0 -- variables used for the state machines

	last_heel_strike_angle = 0 -- time of the previous heel strike computed using angles
	current_heel_strike_angle = 0 -- time of the current heel strike computed using angles

	last_heel_strike_force = 0 -- time of the previous heel strike computed using forces
	current_heel_strike_force = 0 -- time of the current heel strike computed using forces

	previous_ground_reaction_force_right = 10 -- used to detect a heel strike (we start at a random value different from 0)
	current_ground_reaction_force_right = 10 -- used to detect a heel strike (to avoid triggering a fake heel strike)
	previous_ground_reaction_force_left = 10 -- used to detect a heel strike (we start at a random value different from 0)
	current_ground_reaction_force_left = 10 -- used to detect a heel strike (to avoid triggering a fake heel strike)

	step_angle = 0 --counter of steps using the angle information
	step_force = 0 --counter of steps using the ground force information

	first_step = 0 --variable to activate the actuator
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
	hip_angle = model:find_dof("hip_flexion_r"):position()
	hip_angle_vel = model:find_dof("hip_flexion_r"):velocity()

	-- DETECT A HEEL STRIKE USING THE ANGLE AND VELOCITY (NOT WORKING YET)
	--[[
	-- We use a state machine to avoir a particular case where the position and velocity coincide with a fake trigger of a heel strike
	if (state_angle == 0) then
		if ( (hip_angle > 26.5 and hip_angle < 28) and (hip_angle_vel < -80 and hip_angle_vel > -90) ) then
			state_angle = 1
		end
	elseif (state_angle == 1) then
		if ( hip_angle < -20) then
			last_heel_strike_angle = current_heel_strike_angle
			current_heel_strike_angle = model:time();
			step_angle = step_angle + 1;
			state_angle = 0
		end
	end
	--]]

	-- DETECT A HEEL STRIKE USING GROUND REACTION FORCE
	if (state_force == 0) then -- we use this state machine to aboid some unpredictable bouncing of the feet counting as 2 steps
		if ( (previous_ground_reaction_force_right == 0) and (current_ground_reaction_force_right ~= 0) ) then
			last_heel_strike_force = current_heel_strike_force
			current_heel_strike_force = model:time();
			current_period = period + (current_heel_strike_force - last_heel_strike_force)*control_ratio -- compute the current period using a control strategy
			--control scheme
			state_force = 1 -- our right heel has touched the ground, we wait for a left heel impact now
			step_force = step_force + 1; --we count another step achieved with force measurement
			
			time_offset = model:time() --we update the time_offset (used to compute the polynomial time-scaling-factor) every time e detect a heel strike

			if ( first_step == 0 ) then -- we execute this just one we wave the first heel strike
				first_step = 1
			end
		end
	elseif (state_force == 1) then	
		if ( (previous_ground_reaction_force_left == 0) and (current_ground_reaction_force_left ~= 0) ) then
			state_force = 0 -- our left heel has touched the ground, we wait for a right heel impact now
		end
	end
	
	-- APPLY THE ACTUATORS TORQUE
	if ( first_step == 1 ) then -- We start when we detect the first heel-strike
	-- if ( (model:time() < time_offset) and (current_period ~= 0) ) then -- don't do anything until we reach the activation timming and we computed the first period
	-- get rid of time_offset (try to run everything on gait %)

		--[[ Fourier
		else
			actuator_r:add_input( a1*math.sin(b1*model:time()+c1+time_offset) + a2*math.sin(b2*model:time()+c2+time_offset) + a3*math.sin(b3*model:time()+c3+time_offset) )
		end 
		--]]

		--[[ Polynomial --]]

		--if ( (model:time()-time_offset) > period ) then -- make hour polynomial function run periodically
		--	time_offset = model:time() -- we reset the periodic counter
		--end

		--if ( state_force == 0 ) then -- only execute this part of the code when the 

		time = (model:time()-time_offset)*(current_period/period) -- we compute the escalated time

		if ( (model:time()-time_offset) < current_period ) then -- we apply the torque only for a short time, to aboid the SUPER torques of a polynomial
			torque = ( 0.1*( p4*(time)^4 + p3*(time)^3 + p2*(time)^2 + p1*(time) + p0 ) )
			actuator_r:add_input( torque ) 
		end

	end

	return false; -- return false to keep going
end 


function store_data( frame )
	frame:set_value("time_offset", time_offset);
	--frame:set_value("time", time);
	--frame:set_value("torque", torque);
	frame:set_value("step_force", step_force);
	frame:set_value("step_angle", step_angle);
	frame:set_value("current_ground_reaction_force_right", current_ground_reaction_force_right/1000);
	frame:set_value("current_ground_reaction_force_left", current_ground_reaction_force_left/1000);
	frame:set_value("state_force", state_force);
	frame:set_value("hip_angle", hip_angle);
	frame:set_value("hip_angle_vel", hip_angle_vel);
	frame:set_value("current_period", current_period);
	frame:set_value("period_ratio", current_period/period);
	--frame:set_value("current_point", current_point);

end