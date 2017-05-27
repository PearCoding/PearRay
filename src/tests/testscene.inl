R"(
(scene
	:name 'project'
	:renderWidth 100
	:renderHeight 100
	:camera 'Camera'
	(output
		:name 'image'
		(channel
			:type 'rgb'
			:gamma 'False'
			:mapper 'none'
		)
	)
	(output
		:name 'depth'
		(channel
			:type 'depth'
		)
	)
	(entity
		:name 'Camera'
		:type 'camera'
		:width 1.000000
		:height 1.000000
		:zoom 1.000000
		:fstop 0.000000
		:apertureRadius 0.500000
		:localDirection [0,0,-1]
		:localUp [0,-1,0]
		:localRight [1,0,0]
		:position [0.000000,-2.000000,0.000000]
		:rotation (euler 90.0000,0.0000,0.0000)
		:scale [1.000000,1.000000,1.000000]
	)
	(spectrum
		:name 'Lamp_color'
		:data (rgb 1.000000 1.000000 1.000000)
	)
	(material
		:name 'Lamp_mat'
		:type 'light'
		:camera_visible false
		:emission 'Lamp_color'
	)
	(entity
		:name 'Lamp'
		:type 'sphere'
		:radius 0.100000
		:material 'Lamp_mat'
		:position [1.000000,-1.000000,1.000000]
	)
	(mesh
		:name 'Plane'
		:type 'triangles'
		(attribute
			:type 'p', [-1.000000, -1.000000, 0.000000], [1.000000, -1.000000, 0.000000], [1.000000, 1.000000, 0.000000], [-1.000000, 1.000000, 0.000000]
		)
		(attribute
			:type 'n', [0.000000, 0.000000, 1.000000], [0.000000, 0.000000, 1.000000], [0.000000, 0.000000, 1.000000], [0.000000, 0.000000, 1.000000]
		)
		(materials, 0, 0
		)
		(faces, 0, 1, 2, 0, 2, 3
		)
	)
	(entity
		:name 'Plane'
		:type 'mesh'
		:materials 'Material'
		:mesh 'Plane'
		:position [0.000000,0.000000,0.000000]
		:rotation (euler 90.0000,-0.0000,-0.0000)
		:scale [1.000000,1.000000,1.000000]
	)
	(spectrum
		:name 'Material_diffuse_color'
		:data (rgb 1.000000 1.000000 1.000000)
	)
	(material
		:name 'Material'
		:type 'diffuse'
		:albedo 'Material_diffuse_color'
	)
))"