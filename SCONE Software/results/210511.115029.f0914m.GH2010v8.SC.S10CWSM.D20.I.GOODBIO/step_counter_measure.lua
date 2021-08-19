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


    -- DETECT A HEEL STRIKE USING GROUND REACTION FORCE
    if (state_force == 0) then -- we use this state machine to aboid some unpredictable bouncing of the feet counting as 2 steps
        if ((previous_ground_reaction_force_right == 0) and (current_ground_reaction_force_right ~= 0)) then
            -- gather the time of the current step
            state_force = 1 -- our right heel has touched the ground, we wait for a left heel impact now
            step_force = step_force + 1; -- we count another step achieved with force measurement
        end
    elseif (state_force == 1) then
        if ((previous_ground_reaction_force_left == 0) and (current_ground_reaction_force_left ~= 0)) then
            state_force = 0 -- our left heel has touched the ground, we wait for a right heel impact now
        end
    end

	score = - step_force * 100

    return false; -- return false to keep going
end

function result( model )

	return score
end

function store_data( frame )
	frame:set_value("step_force", step_force);
	frame:set_value("score", score);
end