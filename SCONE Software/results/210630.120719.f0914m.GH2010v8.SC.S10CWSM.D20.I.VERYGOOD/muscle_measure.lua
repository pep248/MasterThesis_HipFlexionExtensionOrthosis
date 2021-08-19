function init( model, par )
	actuator_r = model:find_actuator( "hip_flexion_r");
	actuator_l = model:find_actuator( "hip_flexion_l");

	-- MUCLE IMPLEMENTATION
	--discarted because is already performed inside the Geyer Controller
	muscle_hamstrings_r = model:find_muscle( "hamstrings_r");
	muscle_glut_max_r = model:find_muscle( "glut_max_r");
	muscle_iliopsoas_r = model:find_muscle( "iliopsoas_r");

	muscle_hamstrings_l = model:find_muscle( "hamstrings_l");
	muscle_glut_max_l = model:find_muscle( "glut_max_l");
	muscle_iliopsoas_l = model:find_muscle( "iliopsoas_l");

	force_hamstrings_r = 0
	force_glut_max_r = 0
	force_iliopsoas_r = 0

	force_hamstrings_l = 0
	force_glut_max_l = 0
	force_iliopsoas_l = 0
	--]]
	sum_r = 0
	sum_l = 0
	steps = 0

	muscle_effort = 0
	muscle_score = 0
end

function update( model )
	--sum_r = sum_r + actuator_r:input()
	--sum_l = sum_l + actuator_l:input()

	force_hamstrings_r = muscle_hamstrings_r:activation()
	force_glut_max_r = muscle_glut_max_r:activation()
	force_iliopsoas_r = muscle_iliopsoas_r:activation()
	force_hamstrings_l = muscle_hamstrings_l:activation()
	force_glut_max_l = muscle_glut_max_l:activation()
	force_iliopsoas_l = muscle_iliopsoas_l:activation()
	-- should we include vasti_r?

	steps = steps + 1

    muscle_effort = ( muscle_effort + math.abs(force_hamstrings_l) - math.abs(force_hamstrings_r) + math.abs(force_glut_max_l) - math.abs(force_glut_max_r) + math.abs(force_iliopsoas_l) - math.abs(force_iliopsoas_r) ) 
	muscle_score = muscle_effort / steps


	return false -- return false to keep going
end

function result( model )

	return muscle_score
end

function store_data( frame )
	frame:set_value("hip_flexion_actuator_r", actuator_r:input()/10);
	frame:set_value("hip_flexion_actuator_l", actuator_l:input()/10);

	frame:set_value("force_hamstrings_r", force_hamstrings_r/1000);
	frame:set_value("force_glut_max_r", force_glut_max_r/1000);
	frame:set_value("force_iliopsoas_r", force_iliopsoas_r/1000);

	frame:set_value("muscle_score", muscle_score );
	frame:set_value("steps", steps );

	frame:set_value("hamstring_difference", math.abs(force_hamstrings_l) - math.abs(force_hamstrings_r) );
	frame:set_value("iliopsoas_difference", math.abs(force_iliopsoas_l) - math.abs(force_iliopsoas_r) );
	frame:set_value("gluteus_difference", math.abs(force_glut_max_l) - math.abs(force_glut_max_r) );
end