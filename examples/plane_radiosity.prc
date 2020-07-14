(scene
	:name 'radiosity_test'

	(integrator 
	  :type 'direct'
	  :max_ray_depth 0
	  :light_sample_count 1
	  :msi false
	)

	; New sensor/camera interface
	(sensor 
		:name 'Camera'
		:type 'orthographic'
		:sensor_width 1
		:sensor_height 1
		:localDirection [0,0,-1]
		:localUp [0,1,0]
		:localRight [1,0,0]
		:transform [1.0,0.0,0.0,0.0, 0.0,1.0,0.0,0.0, 0.0,0.0,1.0,1.0, 0.0,0.0,0.0,1.0]

		:spectral_range 520 ; Set wavelength to monotonic 520nm
		; :spectral_range [320 780]

		:film_width 1000
		:film_height 1000
		(sampler 
			:slot 'aa'
			:type 'hammersley'
			:sample_count 512
		)
		(filter 
			:slot 'pixel'
			:type 'mitchell'
			:radius 0
		)

		(file
			:name 'image'
			(channel :type 'color' :color 'monotonic' ) ; Will ignore all other wavelength channels except the hero channel
		)
	)

	; Light Area
	(emission
		:name 'Area_em'
		:type 'standard'
		:radiance (illuminant "E")
	)
	(entity
		:name 'Area'
		:type 'plane'
		:centering true
		:width 1.000000
		:height -1.000000
		:emission 'Area_em'
		:transform [1.0,0.0,0.0,0.0, 0.0,1.0,0.0,0.0, 0.0,0.0,1.0,2.0, 0.0,0.0,0.0,1.0]
	)
	; Pure diffuse object
	(material
		:name 'Diffuse'
		:type 'diffuse'
		:albedo 1.0
	)
	(entity
		:name 'Plane'
		:type 'plane'
		:centering true
		:width 1.000000
		:height 1.000000
		:material 'Diffuse'
		:transform [1.0,0.0,0.0,0.0, 0.0,1.0,0.0,0.0, 0.0,0.0,1.0,0.0, 0.0,0.0,0.0,1.0]
	)
)

