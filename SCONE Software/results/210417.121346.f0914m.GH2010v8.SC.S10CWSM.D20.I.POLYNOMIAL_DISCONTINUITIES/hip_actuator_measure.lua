function init( model, par )
	actuator_r = model:find_actuator( "hip_flexion_r");
	actuator_l = model:find_actuator( "hip_flexion_l");

	-- MUCLE IMPLEMENTATION
	--discarted because is already performed inside the Geyer Controller
	muscle_hamstrings_r = model:find_muscle( "hamstrings_r");
	muscle_glut_max_r = model:find_muscle( "glut_max_r");
	muscle_iliopsoas_r = model:find_muscle( "iliopsoas_r");
	force_hamstrings_r = 0
	force_glut_max_r = 0
	force_iliopsoas_r = 0
	--]]
	sum_r = 0
	sum_l = 0
	steps = 0
	effort = 0
end

function update( model )
	sum_r = sum_r + actuator_r:input()
	sum_l = sum_l + actuator_l:input()

	force_hamstrings_r = muscle_hamstrings_r:force()
	force_glut_max_r = muscle_glut_max_r:force()
	force_iliopsoas_r = muscle_iliopsoas_r:force()
	-- should we include vasti_r?

	steps = steps + 1

	-- We don't care about motor energy, we care about human energy
	--[[
	if( (force_hamstrings_r*force_hamstrings_r + force_glut_max_r*force_glut_max_r + force_iliopsoas_r*force_iliopsoas_r)/(2 * steps) == 0 ) then
		effort = 100
	else
		effort = math.min( 1 / ( (sum_r*sum_r + sum_l*sum_l)/(2 * steps) ) , 100) -- we MAXIMIZE the torque of the exoskeleton
	end
	--]]

	effort = (force_hamstrings_r + force_glut_max_r + force_iliopsoas_r)/(2 * steps) --not squared because muscle forces can only be positive
	--implement comparison duration of the steps 

	-- MOTOR EFFORT REDUCTION
	--[[
	sum_r = sum_r + actuator_r:input()
	sum_l = sum_l + actuator_l:input()
	steps = steps + 1
	effort = (sum_r*sum_r + sum_l*sum_l)/(2 * steps)
	-- effort = math.min( 1 / ( (sum_r*sum_r + sum_l*sum_l)/(2 * steps) ) , 100) -- we MAXIMIZE the torque of the exoskeleton
	--]]

	return false -- return false to keep going
end

function result( model )

	return effort
end

function store_data( frame )
	frame:set_value("hip_flexion_actuator_r", actuator_r:input()/10);
	frame:set_value("hip_flexion_actuator_l", actuator_l:input()/10);

	frame:set_value("force_hamstrings_r", force_hamstrings_r/1000);
	frame:set_value("force_glut_max_r", force_glut_max_r/1000);
	frame:set_value("force_iliopsoas_r", force_iliopsoas_r/1000);

	frame:set_value("effort", effort );
	frame:set_value("steps", steps );
end