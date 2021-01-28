(scene
	:name 'example_sphere'
	:render_width 1000
	:render_height 1000
	(sampler 
	  :slot 'aa'
	  :type 'sobol'
	  :sample_count 4
	)
	(filter 
	  :slot 'pixel'
	  :type 'mitchell'
	  :radius 1
	)
	(integrator 
	  :type 'direct'
	  :max_ray_depth 2
	)
	; Outputs
	(output
		:name 'image'
		(channel :type 'color' :color 'xyz' )
	)
	; Camera
	(camera
		:name 'Camera'
		:type 'standard'
		:width 1
		:height 1
		:zoom 1.000000
		:fstop 0.000000
		:apertureRadius 0.500000
		:local_direction [0,0,-1]
		:local_up [0,1,0]
		:local_right [1,0,0]
		:transform [1,0,0,0,
		0,1,0,0,
		0,0,1,0,
		0,0,0,1]
	)
	(light
		:name 'env'
		:type 'env'
		:radiance (illuminant 'D65')
	)
	; Materials
	(material
		:name 'Material'
		:type 'diffuse'
		:albedo (smul (div (add 1 (perlin 16)) 2) (refl 0.8 0.8 0.0))
	)
	; Entity Sphere
	(entity
		:name 'Sphere'
		:type 'sphere'
		:material 'Material'
		:radius 1.5
		:transform [1,0,0,0,
		0,1,0,0,
		0,0,1,-4,
		0,0,0,1]
	)
	; Entity Plane
	(entity
		:name 'Ground'
		:type 'plane'
		:material 'Material'
		:x_axis [8,0,0]
		:y_axis [0,0,-8]
		:transform [1,0,0,-4,
		0,1,0,-0.85,
		0,0,1,1,
		0,0,0,1]
	)
)
