-- File where to change the actuator inputs
function init(model, par)
    actuator_r = model:find_actuator("hip_flexion_r");
    actuator_l = model:find_actuator("hip_flexion_l");

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

		a1 = par:create_from_string( "a1", scone.a1 )
		b1 = par:create_from_string( "b1", scone.b1 )
		c1 = par:create_from_string( "c1", scone.c1 )

		a2 = par:create_from_string( "a2", scone.a2 )
		b2 = par:create_from_string( "b2", scone.b2 )
		c2 = par:create_from_string( "c2", scone.c2 )

		a3 = par:create_from_string( "a3", scone.a3 )
		b3 = par:create_from_string( "b3", scone.b3 )
		c3 = par:create_from_string( "c3", scone.c3 )

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

    a1 = par:create_from_string("a1", scone.a1)
    b1 = par:create_from_string("b1", scone.b1)
    c1 = par:create_from_string("c1", scone.c1)

    a2 = par:create_from_string("a2", scone.a2)
    b2 = par:create_from_string("b2", scone.b2)
    c2 = par:create_from_string("c2", scone.c2)

    a3 = par:create_from_string("a3", scone.a3)
    b3 = par:create_from_string("b3", scone.b3)
    c3 = par:create_from_string("c3", scone.c3)

        --[[

    a4 = par:create_from_string("a4", scone.a4)
    b4 = par:create_from_string("b4", scone.b4)
    c4 = par:create_from_string("c4", scone.c4)

    a5 = par:create_from_string("a5", scone.a5)
    b5 = par:create_from_string("b5", scone.b5)
    c5 = par:create_from_string("c5", scone.c5)

    a6 = par:create_from_string("a6", scone.a6)
    b6 = par:create_from_string("b6", scone.b6)
    c6 = par:create_from_string("c6", scone.c6)

    a7 = par:create_from_string("a7", scone.a7)
    b7 = par:create_from_string("b7", scone.b7)
    c7 = par:create_from_string("c7", scone.c7)

    a8 = par:create_from_string("a8", scone.a8)
    b8 = par:create_from_string("b8", scone.b8)
    c8 = par:create_from_string("c8", scone.c8)
        --]]
        
    body_mass = par:create_from_string("body_mass", scone.body_mass)

    torque_multiplier = par:create_from_string("torque_multiplier", scone.torque_multiplier)

    control_ratio = par:create_from_string("control_ratio", scone.control_ratio) -- feedback constant

    angle_phase = par:create_from_string("angle_phase", scone.angle_phase)

    
	original_period = 1/(b1/(2*math.pi))  -- duration of the theoretical step period
    previous_step_period = previous_step_period -- period of the previous step
    theoretical_period = original_period
    theoretical_period_corrected = original_period -- duration of the step after one control loop
    new_period = original_period -- period used for the corrected timming
    previous_step_duration = original_period
    
    performed_gait = 0 -- "tan per one" of gait 
    excedent_gait = 0
    previous_excedent_gait = 0

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
    time = 0
    
end

function update(model)
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
            last_heel_strike_force = current_heel_strike_force
            current_heel_strike_force = model:time();

            -- gather information from the past step
            previous_step_period = new_period -- period of the previous step
            previous_step_duration = current_heel_strike_force - last_heel_strike_force --real duration of the previou step
            
            -- we check how different was our step from what we expected
            performed_gait = performed_gait + previous_step_duration/previous_step_period -1 -- if bigger  than 1 -> the step was slower than the torque
                                                                                             -- if smaller than 1 -> the step was faster than the torque
            --previous_excedent_gait = excedent_gait
            --excedent_gait = previous_excedent_gait + previous_step_duration/previous_step_period - math.floor(previous_step_duration/previous_step_period)
            --performed_gait = previous_excedent_gait + previous_step_duration/previous_step_period
            

            -- we compute the upcoming period
            
            theoretical_period = previous_step_duration/(2-performed_gait)
            -- theoretical_period = previous_step_duration/(performed_gait)

            theoretical_period_corrected = theoretical_period_corrected + ( - theoretical_period_corrected + theoretical_period ) * control_ratio -- desired duration of the following gait
            new_period = theoretical_period_corrected -- period of the upcoming wave

            -- we update the time_offset (used to compute wave reset point) every time we detect a heel strike
            time_offset = model:time() 

            -- state variable (we use it to control that we do one step after the other)
            state_force = 1 -- our right heel has touched the ground, we wait for a left heel impact now
            step_force = step_force + 1; -- we count another step achieved with force measurement

            if (first_step == 0) then -- we execute this at the first step to get a good first period
                theoretical_period_corrected = previous_step_duration;
                performed_gait = 0;
            end

            if (first_step == 0) then -- we execute this just once we wave the first heel strike
                first_step = first_step + 1
            end

            
        end
    elseif (state_force == 1) then
        if ((previous_ground_reaction_force_left == 0) and (current_ground_reaction_force_left ~= 0)) then
            state_force = 0 -- our left heel has touched the ground, we wait for a right heel impact now
        end
    end


    -- APPLY THE ACTUATORS TORQUE
    if (first_step == 1) then -- We start when we detect the first heel-strike
		time = (model:time() - time_offset + performed_gait*new_period) * ( original_period / new_period );
		sine_torque = torque_multiplier*body_mass*(a1 * math.sin(b1 * (time + angle_phase) + c1) + a2 * math.sin(b2 * (time + angle_phase) + c2) + a3 * math.sin(b3 * (time + angle_phase) + c3)) --+ a4 * math.sin(b4 * time + c4) + a5 * math.sin(b5 * time + c5) + a6 * math.sin(b6 * time + c6) + a7 * math.sin(b7 * time + c7) + a8 * math.sin(b8 * time + c8) )
        actuator_r:add_input(sine_torque)
    end


    return false; -- return false to keep going
end

function store_data(frame)
    frame:set_value("time_offset", time_offset);
    frame:set_value("time", time);
    -- frame:set_value("time", time);
    frame:set_value("sine_torque", sine_torque);
    frame:set_value("step_force", step_force);
    frame:set_value("step_angle", step_angle);
    frame:set_value("current_ground_reaction_force_right",current_ground_reaction_force_right / 1000);
    frame:set_value("current_ground_reaction_force_left",current_ground_reaction_force_left / 1000);
    frame:set_value("state_force", state_force);
    frame:set_value("hip_angle", hip_angle);
    frame:set_value("hip_angle_vel", hip_angle_vel);
    frame:set_value("theoretical_period", theoretical_period);
    frame:set_value("theoretical_period_corrected", theoretical_period_corrected);
    frame:set_value("new_period", new_period);
    frame:set_value("performed_gait", performed_gait);
    frame:set_value("first_step", first_step);
    frame:set_value("previous_step_duration", previous_step_duration);
    frame:set_value("excedent_gait", excedent_gait);
    -- frame:set_value("current_point", current_point);

    

end
