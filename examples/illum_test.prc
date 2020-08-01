(scene
	:name 'illum_test'
	:render_width 100
	:render_height 100
	:camera 'Camera'
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
		:sample_count 1024
	)
	(sampler
		:slot 'spectral'
		:type 'random'
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
		:position [0,0,-1.00005]
	)
	; Background
	(light
		:name 'background'
		:type 'env'
		:radiance (illuminant "D65")
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
		:albedo "white"
	)
)
