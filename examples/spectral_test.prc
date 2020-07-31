(scene
	:name 'illum_test'
	:render_width 1000
	:render_height 1000
	:camera 'Camera'
	:spectral_domain 520

	; Settings
	(integrator
		:type 'DIRECT'
		:max_ray_depth 2
		:light_sampe_count 1
		:msi true
	)
	(sampler
		:slot 'aa'
		:type 'hammersley'
		:sample_count 32
	)
	(filter
		:slot 'pixel'
		:type 'BLOCK'
		:radius 0
	)
	; Outputs
	(output
		:name 'image'
		(channel :type 'color' :color 'srgb' )
	)
	; Camera
	(camera
		:name 'Camera'
		:type 'orthographic'
		:width 2
		:height 2
		:localDirection [0,0,1]
		:localUp [0,1,0]
		:localRight [1,0,0]
		:position [0,0,-1.0005]
	)
	; Background
	(light
		:name 'background'
		:type 'env'
		:radiance 1
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
	(material
		:name 'Diffuse'
		:type 'diffuse'
		:albedo 1
	)
)
