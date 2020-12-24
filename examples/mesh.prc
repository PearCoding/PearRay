(scene
	:name 'example_mesh'
	:render_width 1000
	:render_height 1000
	(sampler 
	  :slot 'aa'
	  :type 'sobol'
	  :sample_count 1
	)
	(filter 
	  :slot 'pixel'
	  :type 'mitchell'
	  :radius 1
	)
	(integrator 
	  :type 'ao'
	  :sample_count 32
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
		:local_direction [0,0,-1]
		:local_up [0,1,0]
		:local_right [1,0,0]
		:transform [1,0,0,0,
		0,1,0,0,
		0,0,1,0,
		0,0,0,1]
	)
	; Materials
	(material
		:name 'Material'
		:type 'diffuse'
		:albedo (refl 0 0.800000 0.800000)
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
)
