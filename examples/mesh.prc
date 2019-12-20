(scene
	:name 'example_mesh'
	:camera 'Camera'
	; Settings
	(registry '/renderer/film/width' 400)
	(registry '/renderer/film/height' 400)
	(registry '/renderer/common/type' 'occlusion')
	(registry '/renderer/common/max_ray_depth' 8)
	(registry '/renderer/common/tile/mode' 0)
	(registry '/renderer/common/sampler/aa/count' 32)
	(registry '/renderer/common/sampler/lens/count' 1)
	(registry '/renderer/common/sampler/time/count' 1)
	(registry '/renderer/common/sampler/time/mapping' 0)
	(registry '/renderer/common/sampler/time/scale' 1.0)
	(registry '/renderer/integrator/direct/light/sample_count' 4)
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
		(channel
			:type 'p'
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
	; Entity Mesh
	(entity
		:name 'Mesh'
		:type 'mesh'
		:materials 'Material'
		:mesh 'Cube'
		:transform [0.7071,0,0.7071,0,
		0,1,0,0,
		-0.7071,0,0.7071,-4,
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
		:data (refl 0 0.800000 0.800000)
	)
	(material
		:name 'Material'
		:type 'diffuse'
		:albedo 'Material_diffuse_color'
	)
	; Mesh
	(mesh
		:name 'Cube'
		:type 'triangles'
		(attribute
			:type 'p', [1.000000, 1.000000, -1.000000], [1.000000, -1.000000, -1.000000], [-1.000000, -1.000000, -1.000000], [-1.000000, 1.000000, -1.000000], [1.000000, 0.999999, 1.000000], [-1.000000, 1.000000, 1.000000], [-1.000000, -1.000000, 1.000000], [0.999999, -1.000001, 1.000000], [1.000000, 1.000000, -1.000000], [1.000000, 0.999999, 1.000000], [0.999999, -1.000001, 1.000000], [1.000000, -1.000000, -1.000000], [1.000000, -1.000000, -1.000000], [0.999999, -1.000001, 1.000000], [-1.000000, -1.000000, 1.000000], [-1.000000, -1.000000, -1.000000], [-1.000000, -1.000000, -1.000000], [-1.000000, -1.000000, 1.000000], [-1.000000, 1.000000, 1.000000], [-1.000000, 1.000000, -1.000000], [1.000000, 0.999999, 1.000000], [1.000000, 1.000000, -1.000000], [-1.000000, 1.000000, -1.000000], [-1.000000, 1.000000, 1.000000]
		)
		(attribute
			:type 'n', [0.000000, 0.000000, -1.000000], [0.000000, 0.000000, -1.000000], [0.000000, 0.000000, -1.000000], [0.000000, 0.000000, -1.000000], [0.000000, 0.000000, 1.000000], [0.000000, 0.000000, 1.000000], [0.000000, 0.000000, 1.000000], [0.000000, 0.000000, 1.000000], [1.000000, -0.000000, 0.000000], [1.000000, -0.000000, 0.000000], [1.000000, -0.000000, 0.000000], [1.000000, -0.000000, 0.000000], [-0.000000, -1.000000, -0.000000], [-0.000000, -1.000000, -0.000000], [-0.000000, -1.000000, -0.000000], [-0.000000, -1.000000, -0.000000], [-1.000000, 0.000000, -0.000000], [-1.000000, 0.000000, -0.000000], [-1.000000, 0.000000, -0.000000], [-1.000000, 0.000000, -0.000000], [0.000000, 1.000000, 0.000000], [0.000000, 1.000000, 0.000000], [0.000000, 1.000000, 0.000000], [0.000000, 1.000000, 0.000000]
		)
		(materials, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 )
		(faces
			[0, 1, 2], [0, 2, 3], [4, 5, 6], [4, 6, 7], [8, 9, 10], [8, 10, 11], [12, 13, 14], [12, 14, 15], [16, 17, 18], [16, 18, 19], [20, 21, 22], [20, 22, 23]
		)
	)
)
