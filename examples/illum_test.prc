(scene
	:name 'color_test'
	:renderWidth 1000
	:renderHeight 1000
	:camera 'Camera'
	:spectrum 'srgb'
	; Settings
	(integrator
		:type 'DIRECT'
		:max_ray_depth 1
		:light_sampe_count 1
		:msi false
	)
	(sampler
		:slot 'aa'
		:type 'UNIFORM'
		:sample_count 32
	)
	(sampler
		:slot 'lens'
		:type 'MULTI_JITTER'
		:sample_count 1
	)
	(sampler
		:slot 'time'
		:type 'MULTI_JITTER'
		:sample_count 1
	)
	(filter
		:slot 'pixel'
		:type 'BLOCK'
		:radius 0
	)
	; Outputs
	(output
		:name 'image'
		(channel
			:type 'color'
			:color 'xyz'
			:gamma 'none'
			:mapper 'none'
		)
		(channel
			:type 'feedback'
		)
	)
	; Camera
	(camera
		:name 'Camera'
		:type 'standard'
		:width 0.720000
		:height 0.720000
		:zoom 1.000000
		:fstop 0.000000
		:apertureRadius 0.500000
		:localDirection [0,0,1]
		:localUp [0,1,0]
		:localRight [1,0,0]
		:near 0.100000
		:far 100.000000
		:transform [1.0,0.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,0.0,1.0,-3.141592,0.0,0.0,0.0,1.0]
	)
	; Background
	(spectrum
		:name 'background_radiance'
		:data (illum 1.000000 1.000000 1.000000)
	)
	(light
		:name 'background'
		:type 'env'
		:radiance 'background_radiance'
		:background 'black'
		:factor 1.000000
	)
	; Lights
	; Primitives
	(entity
		:type "sphere"
		:name "Unit Sphere"
		:radius 1
		:material "Diffuse"
	)
	; Materials
	(spectrum
		:name 'kd_color'
		:data (refl 1.00000 1.00000 1.00000)
	)
	(material
		:name 'Diffuse'
		:type 'diffuse'
		:albedo 'kd_color'
	)
)