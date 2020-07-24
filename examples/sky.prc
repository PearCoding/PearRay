(scene
	:name 'sky_sphere'
	:render_width 1000
	:render_height 1000
	(sampler 
	  :slot 'aa'
	  :type 'sobol'
	  :sample_count 16
	)
	(filter 
	  :slot 'pixel'
	  :type 'mitchell'
	  :radius 1
	)
	(integrator 
	  :type 'direct'
	  :msi true
	)
	; Outputs
	(output
		:name 'image'
		(channel :type 'color' :color 'srgb')
		(channel :type 'ng')
		(channel :type 'nx')
		(channel :type 'ny')
		(channel :type 'uv')
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
		:localDirection [0,0,-1]
		:localUp [0,-1,0]
		:localRight [1,0,0]
		:transform [1,0,0,0,
		0,1,0,0,
		0,0,1,0,
		0,0,0,1]
	)
	(light
		:name 'env'
		:type 'env'
		:radiance (sky :turbidity 8)
	)
	; Entity Sphere
	(entity
		:name 'Sphere'
		:type 'sphere'
		:material 'Sphere'
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
		:material 'Ground'
		:x_axis [8,0,0]
		:y_axis [0,0,-8]
		:transform [1,0,0,-4,
		0,1,0,-0.85,
		0,0,1,1,
		0,0,0,1]
	)
	; Materials
	(material
		:name 'Sphere'
		:type 'glass'
		:index (lookup_index "bk7")
	)
	(material
		:name 'Ground'
		:type 'diffuse'
		:albedo 1
	)
)
