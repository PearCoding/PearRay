(scene
	:name 'sky_sphere'
	:render_width 1000
	:render_height 1000
	(sampler 
	  :slot 'aa'
	  :type 'sobol'
	  :sample_count 64
	)
	(filter 
	  :slot 'pixel'
	  :type 'mitchell'
	  :radius 0 ; Off
	)
	(integrator :type 'direct' )
	; Outputs
	(output
		:name 'image'
		(channel :type 'color' :color 'srgb')
	)
	; Camera
	(camera
		:name 'Camera'
		:type 'fisheye'
		:fov (deg2rad 180)
		:local_direction [0,0,1]
		:local_up [0,1,0]
		:local_right [1,0,0]
	)
	(light
		:name 'sky'
		;:type 'uniform_sky'
		:zenith (illuminant "D65")

		:type 'sky'
		:turbidity 3
	)
)
