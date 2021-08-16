function init( model, par )
    
    performed_gait = 0 -- "tan per one" of gait 

    state_angle = 0 -- variables used for the state machines
    state_force = 1 -- variables used for the state machines

    last_heel_strike_angle = 0 -- time of the previous heel strike computed using angles
    current_heel_strike_angle = 0 -- time of the current heel strike computed using angles

    last_heel_strike_force = 0 -- time of the previous heel strike computed using forces
    current_heel_strike_force = 0 -- time of the current heel strike computed using forces

    previous_ground_reaction_force_right = 10 -- used to detect a heel strike (we start at a random value different from 0)
    current_ground_reaction_force_right = 10 -- used to detect a heel strike (to avoid triggering a fake heel strike)
    previous_ground_reaction_force_left = 10 -- used to detect a heel strike (we start at a random value different from 0)
    current_ground_reaction_force_left = 10 -- used to detect a heel strike (to avoid triggering a fake heel strike)

    step_angle = 0 -- counter of steps using the angle information
    step_force = 0 -- counter of steps using the ground force information

    first_step = 0 -- variable to activate the actuator
	sine_torque = 0
	time_offset = 0

	score = 0

    previous_right_position = model:find_body("calcn_r"):com_pos()
    previous_right_position_x = previous_right_position.x
    previous_left_position = model:find_body("calcn_l"):com_pos();
    previous_left_position_x = previous_left_position.x

    current_right_position = 0
    current_right_position_x = 0
    current_left_position = 0
    current_left_position_x = 0

    right_length = 0
    left_length = 0

    length_difference = 0
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
    buffer_right = model:find_body("calcn_r"):com_pos()
    heel_right = buffer_right.y
    buffer_left = model:find_body("calcn_l"):com_pos()
    heel_left = buffer_left.y



    -- DETECT A HEEL STRIKE USING GROUND REACTION FORCE
    if (state_force == 0) then -- we use this state machine to aboid some unpredictable bouncing of the feet counting as 2 steps
        if ((previous_ground_reaction_force_right == 0) and (current_ground_reaction_force_right ~= 0)) then
            -- gather the time of the current step
            state_force = 1 -- our right heel has touched the ground, we wait for a left heel impact now
            step_force = step_force + 1; -- we count another step achieved with force measurement

            previous_right_position_x = current_right_position_x
            buffer_right = model:find_body("calcn_r"):com_pos()
            current_right_position_x = buffer_right.x

            right_length = current_right_position_x - previous_right_position_x

            length_difference = length_difference + ((right_length - left_length)*(right_length - left_length))*10
        end
    elseif (state_force == 1) then
        if ((previous_ground_reaction_force_left == 0) and (current_ground_reaction_force_left ~= 0)) then
            state_force = 0 -- our left heel has touched the ground, we wait for a right heel impact now

            previous_left_position_x = current_left_position_x
            buffer_left = model:find_body("calcn_l"):com_pos()
            current_left_position_x = buffer_left.x

            left_length = current_left_position_x - previous_left_position_x

            length_difference = length_difference + (right_length - left_length)*(right_length - left_length)
        end
    end

	

    return false; -- return false to keep going
end

function result( model )

    
	return length_difference
end

function store_data( frame )
	frame:set_value("right_length", right_length);
	frame:set_value("left_length", left_length);
    frame:set_value("length_difference", length_difference);

    frame:set_value("current_right_position_x", current_right_position_x);
    frame:set_value("current_left_position_x", current_left_position_x);

end