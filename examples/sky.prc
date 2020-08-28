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
	)
	; Camera
	;(camera
	;	:name 'Camera'
	;	:type 'standard'
	;	:width 1
	;	:height 1
	;)
	(camera
		:name 'Camera'
		:type 'spherical'
		:theta_start -1.570796
	)
	(light
		:name 'sky'
		:type 'cloudy_sky'
		:turbidity 3
		:extend true

		:zenith (illuminant "D65")
	)
	(light
		:name 'sun'
		:type 'sun'
		:turbidity 3
		:radius 1
	)
	; Materials
	(material
		:name 'Sphere'
		:type 'glass'
		:index (lookup_index "bk7")
	)
	;(material
	;	:name 'Ground'
	;	:type 'diffuse'
	;	:albedo 1
	;)
	; Entity Sphere
	(entity
		:name 'Sphere'
		:type 'sphere'
		:material 'Sphere'
		:radius 1.5
		:position [0,4,0]
	)
	; Entity Plane
	;(entity
	;	:name 'Ground'
	;	:type 'plane'
	;	:material 'Ground'
	;	:x_axis [200,0,0]
	;	:y_axis [0,100,0]
	;	:position [-100,0,-0.85]
	;)
)
