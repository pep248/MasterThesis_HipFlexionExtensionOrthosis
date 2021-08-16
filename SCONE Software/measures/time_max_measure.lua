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

	real_time = 0

	logistic_time = 1

	logistic_score = 0

end



function update( model )

	-- logistic_time = ( 1 - ( 1 / (1+math.exp(1)^(-0.5*(real_time-10))) ) )

	real_time = model:time()

	if (real_time <= 1) then

		logistic_score = 200

	else

		logistic_score = 100 / real_time

	end

	-- GATHER DATA
	-- update right heel (force)


end 

function result( model )

	return logistic_score
end

function store_data( frame )
	frame:set_value("logistic_score", logistic_score);
	frame:set_value("logistic_time", logistic_time);
	frame:set_value("real_time", real_time);
end