(scene
	:name 'example_sphere'
	:camera 'Camera'
	; Settings
	(registry '/renderer/film/width' 400)
	(registry '/renderer/film/height' 400)
	(registry '/renderer/common/type' 'occlusion')
	(registry '/renderer/common/max_ray_depth' 8)
	(registry '/renderer/common/tile/mode' 0)
	(registry '/renderer/common/sampler/aa/count' 2)
	(registry '/renderer/common/sampler/aa/type' 3)
	(registry '/renderer/common/sampler/lens/count' 1)
	(registry '/renderer/common/sampler/lens/type' 3)
	(registry '/renderer/common/sampler/time/count' 1)
	(registry '/renderer/common/sampler/time/type' 3)
	(registry '/renderer/common/sampler/time/mapping' 0)
	(registry '/renderer/common/sampler/time/scale' 1.0)
	(registry '/renderer/common/sampler/spectral/count' 1)
	(registry '/renderer/common/sampler/spectral/type' 3)
	(registry '/renderer/integrator/direct/diffuse/max_depth' 0)
	(registry '/renderer/integrator/direct/light/sample_count' 4)
	(registry '/renderer/integrator/bidirect/diffuse/max_depth' 1)
	(registry '/renderer/integrator/bidirect/light/max_depth' 1)
	(registry '/renderer/integrator/bidirect/light/sample_count' 1)
	(registry '/renderer/integrator/ao/sample_count' 16)
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
			:type 'ng'
		)
		(channel
			:type 'nx'
		)
		(channel
			:type 'ny'
		)
		(channel
			:type 'uv'
		)
	)
	(output
		:name 'depth'
		(channel
			:type 'depth'
		)
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
	; Entity Sphere
	(entity
		:name 'Sphere'
		:type 'sphere'
		:material 'Material'
		:radius 1
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
	; Materials
	(spectrum
		:name 'Material_diffuse_color'
		:data (rgb 0 0.800000 0.800000)
	)
	(material
		:name 'Material'
		:type 'diffuse'
		:albedo 'Material_diffuse_color'
	)
)
