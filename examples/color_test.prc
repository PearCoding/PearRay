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
		:localDirection [0,0,-1]
		:localUp [0,-1,0]
		:localRight [1,0,0]
		:near 0.100000
		:far 100.000000
		:transform [1.0,0.0,0.0,0.0,0.0,0.0,-1.0,-2.5,0.0,1.0,0.0,1.0,0.0,0.0,0.0,1.0]
	)
	; Background
	(spectrum
		:name 'background_color'
		:data (illum 1.000000 1.000000 1.000000)
	)
	(spectrum
		:name 'background_radiance'
		:data (illum 1.000000 1.000000 1.000000)
	)
	(light
		:name 'background'
		:type 'env'
		:radiance 'background_radiance'
		:background 'background_color'
		:factor 1.000000
	)
	; Lights
	; Primitives
	; Meshes
	(mesh
		:name 'Plane'
		(attribute
			:type 'p'
			[0.750000, 0.750000, 0.000000],[1.000000, 0.750000, 0.000000],[1.000000, 1.000000, 0.000000],[0.750000, 1.000000, 0.000000],[-0.250000, 1.000000, 0.000000],[-0.250000, 0.750000, 0.000000],[-0.125000, 0.750000, 0.000000],[0.000000, 0.750000, 0.000000],[0.000000, 1.000000, 0.000000],[-0.125000, -0.125000, 0.000000],[0.000000, -0.125000, 0.000000],[0.000000, 0.000000, 0.000000],[-0.125000, 0.000000, 0.000000],[0.750000, -0.125000, 0.000000],[0.750000, -0.250000, 0.000000],[1.000000, -0.250000, 0.000000],[1.000000, 0.000000, 0.000000],[0.750000, 0.000000, 0.000000],[0.375000, -0.125000, 0.000000],[0.500000, -0.125000, 0.000000],[0.500000, 0.000000, 0.000000],[0.375000, 0.000000, 0.000000],[0.375000, -0.625000, 0.000000],[0.500000, -0.625000, 0.000000],[0.500000, -0.500000, 0.000000],[0.375000, -0.500000, 0.000000],[0.750000, -0.625000, 0.000000],[0.750000, -0.750000, 0.000000],[1.000000, -0.750000, 0.000000],[1.000000, -0.500000, 0.000000],[0.750000, -0.500000, 0.000000],[-0.625000, -0.125000, 0.000000],[-0.500000, -0.125000, 0.000000],[-0.500000, 0.000000, 0.000000],[-0.625000, 0.000000, 0.000000],[-0.625000, -0.625000, 0.000000],[-0.500000, -0.625000, 0.000000],[-0.500000, -0.500000, 0.000000],[-0.625000, -0.500000, 0.000000],[-0.125000, -0.625000, 0.000000],[0.000000, -0.625000, 0.000000],[0.000000, -0.500000, 0.000000],[-0.125000, -0.500000, 0.000000],[-0.750000, 1.000000, 0.000000],[-0.750000, 0.750000, 0.000000],[-0.625000, 0.750000, 0.000000],[-0.500000, 0.750000, 0.000000],[-0.500000, 1.000000, 0.000000],[-0.625000, 0.375000, 0.000000],[-0.500000, 0.375000, 0.000000],[-0.500000, 0.500000, 0.000000],[-0.625000, 0.500000, 0.000000],[-0.125000, 0.375000, 0.000000],[0.000000, 0.375000, 0.000000],[0.000000, 0.500000, 0.000000],[-0.125000, 0.500000, 0.000000],[0.250000, 1.000000, 0.000000],[0.250000, 0.750000, 0.000000],[0.375000, 0.750000, 0.000000],[0.500000, 0.750000, 0.000000],[0.500000, 1.000000, 0.000000],[0.375000, 0.375000, 0.000000],[0.500000, 0.375000, 0.000000],[0.500000, 0.500000, 0.000000],[0.375000, 0.500000, 0.000000],[0.750000, 0.375000, 0.000000],[0.750000, 0.250000, 0.000000],[1.000000, 0.250000, 0.000000],[1.000000, 0.500000, 0.000000],[0.750000, 0.500000, 0.000000],[0.625000, 0.375000, 0.000000],[0.625000, 0.500000, 0.000000],[0.625000, 0.125000, 0.000000],[0.750000, 0.125000, 0.000000],[0.625000, 0.250000, 0.000000],[0.125000, 0.375000, 0.000000],[0.250000, 0.375000, 0.000000],[0.250000, 0.500000, 0.000000],[0.125000, 0.500000, 0.000000],[0.125000, 0.125000, 0.000000],[0.250000, 0.125000, 0.000000],[0.250000, 0.250000, 0.000000],[0.125000, 0.250000, 0.000000],[0.375000, 0.125000, 0.000000],[0.500000, 0.125000, 0.000000],[0.500000, 0.250000, 0.000000],[0.375000, 0.250000, 0.000000],[0.125000, 0.750000, 0.000000],[0.125000, 0.625000, 0.000000],[0.250000, 0.625000, 0.000000],[0.375000, 0.625000, 0.000000],[0.500000, 0.625000, 0.000000],[-0.375000, 0.375000, 0.000000],[-0.250000, 0.375000, 0.000000],[-0.250000, 0.500000, 0.000000],[-0.375000, 0.500000, 0.000000],[-0.375000, 0.125000, 0.000000],[-0.250000, 0.125000, 0.000000],[-0.250000, 0.250000, 0.000000],[-0.375000, 0.250000, 0.000000],[-0.125000, 0.125000, 0.000000],[0.000000, 0.125000, 0.000000],[0.000000, 0.250000, 0.000000],[-0.125000, 0.250000, 0.000000],[-1.000000, 0.500000, 0.000000],[-1.000000, 0.250000, 0.000000],[-0.750000, 0.250000, 0.000000],[-0.750000, 0.375000, 0.000000],[-0.750000, 0.500000, 0.000000],[-1.000000, 0.000000, 0.000000],[-0.750000, 0.000000, 0.000000],[-0.750000, 0.125000, 0.000000],[-0.625000, 0.125000, 0.000000],[-0.500000, 0.125000, 0.000000],[-0.500000, 0.250000, 0.000000],[-0.625000, 0.250000, 0.000000],[-1.000000, 0.750000, 0.000000],[-1.000000, 1.000000, 0.000000],[-0.750000, 0.625000, 0.000000],[-0.625000, 0.625000, 0.000000],[-0.500000, 0.625000, 0.000000],[-0.375000, -0.625000, 0.000000],[-0.250000, -0.625000, 0.000000],[-0.250000, -0.500000, 0.000000],[-0.375000, -0.500000, 0.000000],[-0.500000, -0.750000, 0.000000],[-0.500000, -1.000000, 0.000000],[-0.250000, -1.000000, 0.000000],[-0.250000, -0.750000, 0.000000],[-0.375000, -0.750000, 0.000000],[0.000000, -1.000000, 0.000000],[0.000000, -0.750000, 0.000000],[-0.125000, -0.750000, 0.000000],[-1.000000, -0.500000, 0.000000],[-1.000000, -0.750000, 0.000000],[-0.750000, -0.750000, 0.000000],[-0.750000, -0.625000, 0.000000],[-0.750000, -0.500000, 0.000000],[-1.000000, -1.000000, 0.000000],[-0.750000, -1.000000, 0.000000],[-0.625000, -0.750000, 0.000000],[-1.000000, -0.250000, 0.000000],[-0.750000, -0.250000, 0.000000],[-0.750000, -0.125000, 0.000000],[-0.750000, -0.375000, 0.000000],[-0.625000, -0.375000, 0.000000],[-0.500000, -0.375000, 0.000000],[-0.500000, -0.250000, 0.000000],[-0.625000, -0.250000, 0.000000],[0.625000, -0.625000, 0.000000],[0.625000, -0.500000, 0.000000],[0.500000, -0.750000, 0.000000],[0.500000, -1.000000, 0.000000],[0.750000, -1.000000, 0.000000],[0.625000, -0.750000, 0.000000],[1.000000, -1.000000, 0.000000],[0.125000, -0.625000, 0.000000],[0.250000, -0.625000, 0.000000],[0.250000, -0.500000, 0.000000],[0.125000, -0.500000, 0.000000],[0.250000, -1.000000, 0.000000],[0.250000, -0.750000, 0.000000],[0.125000, -0.750000, 0.000000],[0.375000, -0.750000, 0.000000],[0.125000, -0.125000, 0.000000],[0.250000, -0.125000, 0.000000],[0.250000, 0.000000, 0.000000],[0.125000, 0.000000, 0.000000],[0.125000, -0.375000, 0.000000],[0.250000, -0.375000, 0.000000],[0.250000, -0.250000, 0.000000],[0.125000, -0.250000, 0.000000],[0.375000, -0.375000, 0.000000],[0.500000, -0.375000, 0.000000],[0.500000, -0.250000, 0.000000],[0.375000, -0.250000, 0.000000],[0.625000, -0.125000, 0.000000],[0.625000, 0.000000, 0.000000],[0.625000, -0.375000, 0.000000],[0.750000, -0.375000, 0.000000],[0.625000, -0.250000, 0.000000],[-0.375000, -0.125000, 0.000000],[-0.250000, -0.125000, 0.000000],[-0.250000, 0.000000, 0.000000],[-0.375000, 0.000000, 0.000000],[-0.375000, -0.375000, 0.000000],[-0.250000, -0.375000, 0.000000],[-0.250000, -0.250000, 0.000000],[-0.375000, -0.250000, 0.000000],[-0.125000, -0.375000, 0.000000],[0.000000, -0.375000, 0.000000],[0.000000, -0.250000, 0.000000],[-0.125000, -0.250000, 0.000000],[-0.375000, 0.750000, 0.000000],[-0.375000, 0.625000, 0.000000],[-0.250000, 0.625000, 0.000000],[-0.125000, 0.625000, 0.000000],[0.000000, 0.625000, 0.000000],[0.625000, 0.750000, 0.000000],[0.625000, 0.625000, 0.000000],[0.750000, 0.625000, 0.000000]
		)
		(attribute
			:type 'n'
			[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[-0.000000, 0.000000, 1.000000],[0.000000, -0.000000, 1.000000],[0.000000, -0.000000, 1.000000],[0.000000, -0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[-0.000000, 0.000000, 1.000000],[0.000000, -0.000000, 1.000000],[0.000000, -0.000000, 1.000000],[0.000000, -0.000000, 1.000000],[-0.000000, 0.000000, 1.000000],[-0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[-0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[-0.000000, 0.000000, 1.000000],[0.000000, -0.000000, 1.000000],[0.000000, -0.000000, 1.000000],[0.000000, -0.000000, 1.000000],[-0.000000, 0.000000, 1.000000],[-0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[-0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[-0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[-0.000000, 0.000000, 1.000000],[0.000000, -0.000000, 1.000000],[0.000000, -0.000000, 1.000000],[0.000000, -0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[-0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[-0.000000, 0.000000, 1.000000],[0.000000, -0.000000, 1.000000],[0.000000, -0.000000, 1.000000],[0.000000, -0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[-0.000000, 0.000000, 1.000000],[0.000000, -0.000000, 1.000000],[0.000000, -0.000000, 1.000000],[0.000000, -0.000000, 1.000000],[-0.000000, 0.000000, 1.000000],[-0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[-0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[-0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[-0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[-0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[-0.000000, 0.000000, 1.000000],[0.000000, -0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[-0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[-0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[-0.000000, 0.000000, 1.000000],[0.000000, -0.000000, 1.000000],[0.000000, -0.000000, 1.000000],[0.000000, -0.000000, 1.000000],[-0.000000, 0.000000, 1.000000],[-0.000000, 0.000000, 1.000000],[0.000000, -0.000000, 1.000000],[0.000000, -0.000000, 1.000000],[-0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[-0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[-0.000000, 0.000000, 1.000000],[-0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[-0.000000, 0.000000, 1.000000],[0.000000, -0.000000, 1.000000],[0.000000, -0.000000, 1.000000],[0.000000, -0.000000, 1.000000],[-0.000000, 0.000000, 1.000000],[-0.000000, 0.000000, 1.000000],[0.000000, -0.000000, 1.000000],[-0.000000, 0.000000, 1.000000],[-0.000000, 0.000000, 1.000000],[0.000000, -0.000000, 1.000000],[0.000000, -0.000000, 1.000000],[0.000000, -0.000000, 1.000000],[-0.000000, 0.000000, 1.000000],[-0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[-0.000000, 0.000000, 1.000000],[0.000000, -0.000000, 1.000000],[0.000000, -0.000000, 1.000000],[-0.000000, 0.000000, 1.000000],[-0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[-0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[-0.000000, 0.000000, 1.000000],[0.000000, -0.000000, 1.000000],[0.000000, -0.000000, 1.000000],[0.000000, -0.000000, 1.000000],[-0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[-0.000000, 0.000000, 1.000000],[0.000000, -0.000000, 1.000000],[-0.000000, 0.000000, 1.000000],[-0.000000, 0.000000, 1.000000],[-0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[-0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[-0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[-0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[-0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[-0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[-0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[-0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[-0.000000, 0.000000, 1.000000],[0.000000, -0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, -0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000]
		)
		(attribute
			:type 'uv'
			[0.875000, 0.875000],[1.000000, 0.875000],[1.000000, 1.000000],[0.875000, 1.000000],[0.375000, 1.000000],[0.375000, 0.875000],[0.437500, 0.875000],[0.500000, 0.875000],[0.500000, 1.000000],[0.437500, 0.437500],[0.500000, 0.437500],[0.500000, 0.500000],[0.437500, 0.500000],[0.875000, 0.437500],[0.875000, 0.375000],[1.000000, 0.375000],[1.000000, 0.500000],[0.875000, 0.500000],[0.687500, 0.437500],[0.750000, 0.437500],[0.750000, 0.500000],[0.687500, 0.500000],[0.687500, 0.187500],[0.750000, 0.187500],[0.750000, 0.250000],[0.687500, 0.250000],[0.875000, 0.187500],[0.875000, 0.125000],[1.000000, 0.125000],[1.000000, 0.250000],[0.875000, 0.250000],[0.187500, 0.437500],[0.250000, 0.437500],[0.250000, 0.500000],[0.187500, 0.500000],[0.187500, 0.187500],[0.250000, 0.187500],[0.250000, 0.250000],[0.187500, 0.250000],[0.437500, 0.187500],[0.500000, 0.187500],[0.500000, 0.250000],[0.437500, 0.250000],[0.125000, 1.000000],[0.125000, 0.875000],[0.187500, 0.875000],[0.250000, 0.875000],[0.250000, 1.000000],[0.187500, 0.687500],[0.250000, 0.687500],[0.250000, 0.750000],[0.187500, 0.750000],[0.437500, 0.687500],[0.500000, 0.687500],[0.500000, 0.750000],[0.437500, 0.750000],[0.625000, 1.000000],[0.625000, 0.875000],[0.687500, 0.875000],[0.750000, 0.875000],[0.750000, 1.000000],[0.687500, 0.687500],[0.750000, 0.687500],[0.750000, 0.750000],[0.687500, 0.750000],[0.875000, 0.687500],[0.875000, 0.625000],[1.000000, 0.625000],[1.000000, 0.750000],[0.875000, 0.750000],[0.812500, 0.687500],[0.812500, 0.750000],[0.812500, 0.562500],[0.875000, 0.562500],[0.812500, 0.625000],[0.562500, 0.687500],[0.625000, 0.687500],[0.625000, 0.750000],[0.562500, 0.750000],[0.562500, 0.562500],[0.625000, 0.562500],[0.625000, 0.625000],[0.562500, 0.625000],[0.687500, 0.562500],[0.750000, 0.562500],[0.750000, 0.625000],[0.687500, 0.625000],[0.562500, 0.875000],[0.562500, 0.812500],[0.625000, 0.812500],[0.687500, 0.812500],[0.750000, 0.812500],[0.312500, 0.687500],[0.375000, 0.687500],[0.375000, 0.750000],[0.312500, 0.750000],[0.312500, 0.562500],[0.375000, 0.562500],[0.375000, 0.625000],[0.312500, 0.625000],[0.437500, 0.562500],[0.500000, 0.562500],[0.500000, 0.625000],[0.437500, 0.625000],[0.000000, 0.750000],[0.000000, 0.625000],[0.125000, 0.625000],[0.125000, 0.687500],[0.125000, 0.750000],[0.000000, 0.500000],[0.125000, 0.500000],[0.125000, 0.562500],[0.187500, 0.562500],[0.250000, 0.562500],[0.250000, 0.625000],[0.187500, 0.625000],[0.000000, 0.875000],[0.000000, 1.000000],[0.125000, 0.812500],[0.187500, 0.812500],[0.250000, 0.812500],[0.312500, 0.187500],[0.375000, 0.187500],[0.375000, 0.250000],[0.312500, 0.250000],[0.250000, 0.125000],[0.250000, 0.000000],[0.375000, 0.000000],[0.375000, 0.125000],[0.312500, 0.125000],[0.500000, 0.000000],[0.500000, 0.125000],[0.437500, 0.125000],[0.000000, 0.250000],[0.000000, 0.125000],[0.125000, 0.125000],[0.125000, 0.187500],[0.125000, 0.250000],[0.000000, 0.000000],[0.125000, 0.000000],[0.187500, 0.125000],[0.000000, 0.375000],[0.125000, 0.375000],[0.125000, 0.437500],[0.125000, 0.312500],[0.187500, 0.312500],[0.250000, 0.312500],[0.250000, 0.375000],[0.187500, 0.375000],[0.812500, 0.187500],[0.812500, 0.250000],[0.750000, 0.125000],[0.750000, 0.000000],[0.875000, 0.000000],[0.812500, 0.125000],[1.000000, 0.000000],[0.562500, 0.187500],[0.625000, 0.187500],[0.625000, 0.250000],[0.562500, 0.250000],[0.625000, 0.000000],[0.625000, 0.125000],[0.562500, 0.125000],[0.687500, 0.125000],[0.562500, 0.437500],[0.625000, 0.437500],[0.625000, 0.500000],[0.562500, 0.500000],[0.562500, 0.312500],[0.625000, 0.312500],[0.625000, 0.375000],[0.562500, 0.375000],[0.687500, 0.312500],[0.750000, 0.312500],[0.750000, 0.375000],[0.687500, 0.375000],[0.812500, 0.437500],[0.812500, 0.500000],[0.812500, 0.312500],[0.875000, 0.312500],[0.812500, 0.375000],[0.312500, 0.437500],[0.375000, 0.437500],[0.375000, 0.500000],[0.312500, 0.500000],[0.312500, 0.312500],[0.375000, 0.312500],[0.375000, 0.375000],[0.312500, 0.375000],[0.437500, 0.312500],[0.500000, 0.312500],[0.500000, 0.375000],[0.437500, 0.375000],[0.312500, 0.875000],[0.312500, 0.812500],[0.375000, 0.812500],[0.437500, 0.812500],[0.500000, 0.812500],[0.812500, 0.875000],[0.812500, 0.812500],[0.875000, 0.812500]
		)
		(materials
			10,10,10,10,10,5,5,10,10,10,6,6,9,9,10,10,10,4,4,7,7,8,8,10,10,10,1,1,2,2,10,10,10,3,3,10,10,10,0,0,0,0,10,10,10,0,0,0,0,0,0,10,10,10,0,0,0,0,0,0,0,0,0,0,10,10,10,10,10,10,0,0,10,10,10,10,10,0,0,0,0,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,0,0,0,0,10,10,10,10,10,0,0,10,10,10,10,10,10,0,0,0,0,0,0,0,0,0,0,10,10,10,0,0,0,0,0,0,10,10,10,0,0,0,0,10,10,10,0,0,10,10,10,0,0,3,3,0,0,0,0,0,0,2,2,0,0,1,1,0,0,0,0,0,0,8,8,0,0,7,7,0,0,4,4,0,0,0,0,0,0,9,9,0,0,6,6,0,0,0,0,0,0,0,0,9,9,0,0,8,8,0,0,5,5,0,0,0,0,8,8,0,0,0,0,9,9,0,0,0,0,0,0,0,0,7,7,7,7,0,0,0,0,0,0,0,0,1,1,0,0,0,0,4,4,0,0,0,0,5,5,0,0,4,4,0,0,1,1,0,0,0,0,0,0,0,0,3,3,0,0,2,2,0,0,0,0,0,0,6,6,0,0,5,5,0,0,2,2,0,0,0,0,0,0,6,6,0,0,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
		)
		(faces
			[0,1,2],[0,2,3],[4,5,6],[6,7,8],[6,8,4],[9,10,11],[9,11,12],[13,14,15],[15,16,17],[15,17,13],[18,19,20],[18,20,21],[22,23,24],[22,24,25],[26,27,28],[28,29,30],[28,30,26],[31,32,33],[31,33,34],[35,36,37],[35,37,38],[39,40,41],[39,41,42],[43,44,45],[45,46,47],[45,47,43],[48,49,50],[48,50,51],[52,53,54],[52,54,55],[56,57,58],[58,59,60],[58,60,56],[61,62,63],[61,63,64],[65,66,67],[67,68,69],[67,69,65],[70,65,69],[70,69,71],[72,73,66],[72,66,74],[73,17,16],[16,67,66],[16,66,73],[75,76,77],[75,77,78],[79,80,81],[79,81,82],[83,84,85],[83,85,86],[8,7,87],[87,57,56],[87,56,8],[88,89,57],[88,57,87],[90,91,59],[90,59,58],[92,93,94],[92,94,95],[96,97,98],[96,98,99],[100,101,102],[100,102,103],[104,105,106],[107,108,104],[106,107,104],[105,109,110],[111,106,105],[110,111,105],[112,113,114],[112,114,115],[116,44,43],[116,43,117],[116,104,108],[118,44,116],[108,118,116],[119,120,46],[119,46,45],[121,122,123],[121,123,124],[125,126,127],[127,128,129],[127,129,125],[128,127,130],[130,131,132],[130,132,128],[133,134,135],[136,137,133],[135,136,133],[138,139,135],[138,135,134],[135,139,126],[126,125,140],[126,140,135],[109,141,142],[143,110,109],[142,143,109],[141,133,137],[144,142,141],[137,144,141],[145,146,147],[145,147,148],[149,26,30],[149,30,150],[151,152,153],[153,27,154],[153,154,151],[153,155,28],[153,28,27],[156,157,158],[156,158,159],[131,130,160],[160,161,162],[160,162,131],[161,160,152],[152,151,163],[152,163,161],[164,165,166],[164,166,167],[168,169,170],[168,170,171],[172,173,174],[172,174,175],[176,13,17],[176,17,177],[178,179,14],[178,14,180],[179,30,29],[29,15,14],[29,14,179],[181,182,183],[181,183,184],[185,186,187],[185,187,188],[189,190,191],[189,191,192],[47,46,193],[193,5,4],[193,4,47],[194,195,5],[194,5,193],[196,197,7],[196,7,6],[60,59,198],[198,0,3],[198,3,60],[199,200,0],[199,0,198],[200,69,68],[68,1,0],[68,0,200],[91,199,198],[91,198,59],[63,71,199],[63,199,91],[71,69,200],[71,200,199],[195,196,6],[195,6,5],[94,55,196],[94,196,195],[55,54,197],[55,197,196],[120,194,193],[120,193,46],[50,95,194],[50,194,120],[95,94,195],[95,195,194],[186,189,192],[186,192,187],[123,42,189],[123,189,186],[42,41,190],[42,190,189],[146,185,188],[146,188,147],[37,124,185],[37,185,146],[124,123,186],[124,186,185],[32,181,184],[32,184,33],[147,188,181],[147,181,32],[188,187,182],[188,182,181],[173,178,180],[173,180,174],[24,150,178],[24,178,173],[150,30,179],[150,179,178],[19,176,177],[19,177,20],[174,180,176],[174,176,19],[180,14,13],[180,13,176],[169,172,175],[169,175,170],[158,25,172],[158,172,169],[25,24,173],[25,173,172],[190,168,171],[190,171,191],[41,159,168],[41,168,190],[159,158,169],[159,169,168],[10,164,167],[10,167,11],[191,171,164],[191,164,10],[171,170,165],[171,165,164],[40,156,159],[40,159,41],[131,162,156],[131,156,40],[162,161,157],[162,157,156],[23,149,150],[23,150,24],[151,154,149],[151,149,23],[154,27,26],[154,26,149],[144,145,148],[144,148,142],[137,38,145],[137,145,144],[38,37,146],[38,146,145],[36,121,124],[36,124,37],[125,129,121],[125,121,36],[129,128,122],[129,122,121],[118,119,45],[118,45,44],[108,51,119],[108,119,118],[51,50,120],[51,120,119],[111,112,115],[111,115,106],[110,34,112],[110,112,111],[34,33,113],[34,113,112],[97,100,103],[97,103,98],[183,12,100],[183,100,97],[12,11,101],[12,101,100],[113,96,99],[113,99,114],[33,184,96],[33,96,113],[184,183,97],[184,97,96],[49,92,95],[49,95,50],[114,99,92],[114,92,49],[99,98,93],[99,93,92],[89,90,58],[89,58,57],[77,64,90],[77,90,89],[64,63,91],[64,91,90],[197,88,87],[197,87,7],[54,78,88],[54,88,197],[78,77,89],[78,89,88],[80,83,86],[80,86,81],[166,21,83],[166,83,80],[21,20,84],[21,84,83],[101,79,82],[101,82,102],[11,167,79],[11,79,101],[167,166,80],[167,80,79],[53,75,78],[53,78,54],[102,82,75],[102,75,53],[82,81,76],[82,76,75],[84,72,74],[84,74,85],[20,177,72],[20,72,84],[177,17,73],[177,73,72],[62,70,71],[62,71,63],[85,74,70],[85,70,62],[74,66,65],[74,65,70],[76,61,64],[76,64,77],[81,86,61],[81,61,76],[86,85,62],[86,62,61],[93,52,55],[93,55,94],[98,103,52],[98,52,93],[103,102,53],[103,53,52],[107,48,51],[107,51,108],[106,115,48],[106,48,107],[115,114,49],[115,49,48],[122,39,42],[122,42,123],[128,132,39],[128,39,122],[132,131,40],[132,40,39],[136,35,38],[136,38,137],[135,140,35],[135,35,136],[140,125,36],[140,36,35],[143,31,34],[143,34,110],[142,148,31],[142,31,143],[148,147,32],[148,32,31],[157,22,25],[157,25,158],[161,163,22],[161,22,157],[163,151,23],[163,23,22],[165,18,21],[165,21,166],[170,175,18],[170,18,165],[175,174,19],[175,19,18],[182,9,12],[182,12,183],[187,192,9],[187,9,182],[192,191,10],[192,10,9]
		)
	)
	(entity
		:name 'Plane'
		:type 'mesh'
		:materials ["White", "Red", "Green", "Blue", "Yellow", "Cyan", "Magenta", "Brown", "GreenCyan", "DarkMagenta", "Border"]
		:mesh 'Plane'
		:transform [1.0,0.0,0.0,0.0,0.0,0.0,-1.0,0.0,0.0,1.0,0.0,1.0,0.0,0.0,0.0,1.0]
	)
	; Materials
	(spectrum
		:name 'Blue_diffuse_color'
		:data (refl 0.000000 0.000000 1.000000)
	)
	(material
		:name 'Blue'
		:type 'diffuse'
		:albedo 'Blue_diffuse_color'
	)
	(spectrum
		:name 'Border_diffuse_color'
		:data (refl 0.500000 0.500000 0.500000)
	)
	(material
		:name 'Border'
		:type 'diffuse'
		:albedo 'Border_diffuse_color'
	)
	(spectrum
		:name 'Brown_diffuse_color'
		:data (refl 1.000000 0.500000 0.250000)
	)
	(material
		:name 'Brown'
		:type 'diffuse'
		:albedo 'Brown_diffuse_color'
	)
	(spectrum
		:name 'Cyan_diffuse_color'
		:data (refl 0.000000 1.000000 1.000000)
	)
	(material
		:name 'Cyan'
		:type 'diffuse'
		:albedo 'Cyan_diffuse_color'
	)
	(spectrum
		:name 'DarkMagenta_diffuse_color'
		:data (refl 0.500000 0.250000 1.000000)
	)
	(material
		:name 'DarkMagenta'
		:type 'diffuse'
		:albedo 'DarkMagenta_diffuse_color'
	)
	(spectrum
		:name 'Green_diffuse_color'
		:data (refl 0.000000 1.000000 0.000000)
	)
	(material
		:name 'Green'
		:type 'diffuse'
		:albedo 'Green_diffuse_color'
	)
	(spectrum
		:name 'GreenCyan_diffuse_color'
		:data (refl 0.250000 1.000000 0.500000)
	)
	(material
		:name 'GreenCyan'
		:type 'diffuse'
		:albedo 'GreenCyan_diffuse_color'
	)
	(spectrum
		:name 'Magenta_diffuse_color'
		:data (refl 1.000000 0.000000 1.000000)
	)
	(material
		:name 'Magenta'
		:type 'diffuse'
		:albedo 'Magenta_diffuse_color'
	)
	(spectrum
		:name 'Red_diffuse_color'
		:data (refl 1.000000 0.000000 0.000000)
	)
	(material
		:name 'Red'
		:type 'diffuse'
		:albedo 'Red_diffuse_color'
	)
	(spectrum
		:name 'White_diffuse_color'
		:data (refl 1.000000 1.000000 1.000000)
	)
	(material
		:name 'White'
		:type 'diffuse'
		:albedo 'White_diffuse_color'
	)
	(spectrum
		:name 'Yellow_diffuse_color'
		:data (refl 1.000000 1.000000 0.000000)
	)
	(material
		:name 'Yellow'
		:type 'diffuse'
		:albedo 'Yellow_diffuse_color'
	)
)