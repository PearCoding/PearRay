; Original CornellBox converted to PearRay format
(scene
	:name 'cornellbox'
	:render_width 1000
	:render_height 1000
	:camera 'Camera'
	; Settings
	(integrator
		:type 'bidi'
		:max_ray_depth 16
		:max_camera_ray_depth 4
		:max_light_ray_depth 1
		:mis 'power'
		:contract_ratio 0.2
		:max_light_ray_depth 16
		:soft_max_light_ray_depth 6
	)
	(sampler
		:slot 'aa'
		:type 'mjitt'
		:sample_count 128
	)
	; Outputs
	(output
		:name 'image'
		(channel :type 'color' :color 'srgb' )
		(channel :type 'blend')
		(channel :type 'var')
	)
	; Camera
	(camera
		:name 'Camera'
		:type 'standard'
		:width 0.720000
		:height 0.720000
		:localDirection [0,0,-1]
		:localUp [0,1,0]
		:localRight [1,0,0]
		:near 0.100000
		:far 100.000000
		:transform [1.0,0.0,0.0,0.0,0.0,0.0,-1.0,-3.93462872505188,0.0,1.0,0.0,1.0,0.0,0.0,0.0,1.0]
	)
	; Lights
	(light
		:name 'sky'
		:type 'sky'
		:turbidity 3
	)

	(emission
		:name 'light_em'
		:type 'standard'
		:radiance (smul (illuminant "D65") (illum 17 12 4))
	)
	; Materials
	(material
		:name 'backWall'
		:type 'diffuse'
		:albedo (refl 0.725000 0.710000 0.680000)
	)
	(material
		:name 'ceiling'
		:type 'diffuse'
		:albedo (refl 0.725000 0.710000 0.680000)
	)
	(material
		:name 'floor'
		:type 'diffuse'
		:albedo (refl 0.725000 0.710000 0.680000)
	)
	(material
		:name 'leftWall'
		:type 'diffuse'
		:albedo (refl 0.630000 0.065000 0.050000)
	)
	(material
		:name 'light'
		:type 'diffuse'
		:albedo (refl 0.780000 0.780000 0.780000)
	)
	(material
		:name 'rightWall'
		:type 'diffuse'
		:albedo (refl 0.140000 0.450000 0.091000)
	)
	(material
		:name 'shortBox'
		:type 'diffuse'
		:albedo (refl 0.725000 0.710000 0.680000)
	)
	(material
		:name 'tallBox'
		:type 'diffuse'
		:albedo (refl 0.725000 0.710000 0.680000)
	)

	; Meshes
	(mesh
		:name 'CornellBox-Original'
		(attribute
			:type 'p'
			[-0.240000, -0.160000, 1.980000],[-0.240000, 0.220000, 1.980000],[0.230000, 0.220000, 1.980000],[0.230000, -0.160000, 1.980000]
		)
		(attribute
			:type 'n'
			[-0.000000, -0.000000, -1.000000],[-0.000000, -0.000000, -1.000000],[-0.000000, -0.000000, -1.000000],[-0.000000, 0.000000, -1.000000]
		)
		(faces
			[0,1,2],[0,2,3]
		)
	)
	(entity
		:name 'CornellBox-Original'
		:type 'mesh'
		:materials 'light'
		;:emission 'light_em'
		:mesh 'CornellBox-Original'
		:transform [1.0,0.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,0.0,1.0]
	)
	(mesh
		:name 'CornellBox-Original.001'
		(attribute
			:type 'p'
			[-1.010000, -0.990000, -0.000000],[1.000000, -0.990000, -0.000000],[1.000000, 1.040000, 0.000000],[-0.990000, 1.040000, 0.000000]
		)
		(attribute
			:type 'n'
			[0.000000, -0.000000, 1.000000],[0.000000, -0.000000, 1.000000],[0.000000, -0.000000, 1.000000],[0.000000, -0.000000, 1.000000]
		)
		(faces
			[0,1,2],[0,2,3]
		)
	)
	(entity
		:name 'CornellBox-Original.001'
		:type 'mesh'
		:materials 'floor'
		:mesh 'CornellBox-Original.001'
		:transform [1.0,0.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,0.0,1.0]
	)
	(mesh
		:name 'CornellBox-Original.002'
		(attribute
			:type 'p'
			[-1.020000, -0.990000, 1.990000],[-1.020000, 1.040000, 1.990000],[1.000000, 1.040000, 1.990000],[1.000000, -0.990000, 1.990000]
		)
		(attribute
			:type 'n'
			[-0.000000, -0.000000, -1.000000],[-0.000000, -0.000000, -1.000000],[-0.000000, -0.000000, -1.000000],[-0.000000, 0.000000, -1.000000]
		)
		(faces
			[0,1,2],[0,2,3]
		)
	)
	(entity
		:name 'CornellBox-Original.002'
		:type 'mesh'
		:materials 'ceiling'
		:mesh 'CornellBox-Original.002'
		:transform [1.0,0.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,0.0,1.0]
	)
	(mesh
		:name 'CornellBox-Original.003'
		(attribute
			:type 'p'
			[-0.990000, 1.040000, 0.000000],[1.000000, 1.040000, 0.000000],[1.000000, 1.040000, 1.990000],[-1.020000, 1.040000, 1.990000]
		)
		(attribute
			:type 'n'
			[-0.000000, -1.000000, -0.000000],[-0.000000, -1.000000, -0.000000],[-0.000000, -1.000000, -0.000000],[0.000000, -1.000000, -0.000000]
		)
		(faces
			[0,1,2],[0,2,3]
		)
	)
	(entity
		:name 'CornellBox-Original.003'
		:type 'mesh'
		:materials 'backWall'
		:mesh 'CornellBox-Original.003'
		:transform [1.0,0.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,0.0,1.0]
	)
	(mesh
		:name 'CornellBox-Original.004'
		(attribute
			:type 'p'
			[1.000000, 1.040000, 0.000000],[1.000000, -0.990000, -0.000000],[1.000000, -0.990000, 1.990000],[1.000000, 1.040000, 1.990000]
		)
		(attribute
			:type 'n'
			[-1.000000, 0.000000, 0.000000],[-1.000000, 0.000000, 0.000000],[-1.000000, 0.000000, 0.000000],[-1.000000, -0.000000, -0.000000]
		)
		(faces
			[0,1,2],[0,2,3]
		)
	)
	(entity
		:name 'CornellBox-Original.004'
		:type 'mesh'
		:materials 'rightWall'
		:mesh 'CornellBox-Original.004'
		:transform [1.0,0.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,0.0,1.0]
	)
	(mesh
		:name 'CornellBox-Original.005'
		(attribute
			:type 'p'
			[-1.010000, -0.990000, -0.000000],[-0.990000, 1.040000, 0.000000],[-1.020000, 1.040000, 1.990000],[-1.010000, -0.990000, -0.000000],[-1.020000, 1.040000, 1.990000],[-1.020000, -0.990000, 1.990000]
		)
		(attribute
			:type 'n'
			[0.999838, -0.009851, 0.015073],[0.999838, -0.009851, 0.015073],[0.999838, -0.009851, 0.015073],[0.999987, -0.000000, 0.005025],[0.999987, -0.000000, 0.005025],[0.999987, -0.000000, 0.005025]
		)
		(faces
			[0,1,2],[3,4,5]
		)
	)
	(entity
		:name 'CornellBox-Original.005'
		:type 'mesh'
		:materials 'leftWall'
		:mesh 'CornellBox-Original.005'
		:transform [1.0,0.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,0.0,1.0]
	)
	(mesh
		:name 'CornellBox-Original.006'
		(attribute
			:type 'p'
			[0.530000, -0.750000, 0.600000],[0.700000, -0.170000, 0.600000],[0.130000, -0.000000, 0.600000],[-0.050000, -0.570000, 0.600000],[-0.050000, -0.570000, -0.000000],[-0.050000, -0.570000, 0.600000],[0.130000, -0.000000, 0.600000],[0.130000, 0.000000, 0.000000],[0.530000, -0.750000, -0.000000],[0.530000, -0.750000, 0.600000],[-0.050000, -0.570000, 0.600000],[-0.050000, -0.570000, -0.000000],[0.700000, -0.170000, -0.000000],[0.700000, -0.170000, 0.600000],[0.530000, -0.750000, 0.600000],[0.530000, -0.750000, -0.000000],[0.130000, 0.000000, 0.000000],[0.130000, -0.000000, 0.600000],[0.700000, -0.170000, 0.600000],[0.700000, -0.170000, -0.000000]
		)
		(attribute
			:type 'n'
			[-0.000000, -0.000000, 1.000000],[-0.000000, -0.000000, 1.000000],[-0.000000, -0.000000, 1.000000],[0.000000, -0.000000, 1.000000],[-0.953583, 0.301131, 0.000000],[-0.953583, 0.301131, 0.000000],[-0.953583, 0.301131, 0.000000],[-0.953583, 0.301131, 0.000000],[-0.296399, -0.955064, -0.000000],[-0.296399, -0.955064, -0.000000],[-0.296399, -0.955064, -0.000000],[-0.296399, -0.955064, 0.000000],[0.959629, -0.281270, -0.000000],[0.959629, -0.281270, -0.000000],[0.959629, -0.281270, -0.000000],[0.959629, -0.281270, 0.000000],[0.285805, 0.958288, 0.000000],[0.285805, 0.958288, 0.000000],[0.285805, 0.958288, 0.000000],[0.285805, 0.958288, 0.000000]
		)
		(faces
			[0,1,2],[0,2,3],[4,5,6],[4,6,7],[8,9,10],[8,10,11],[12,13,14],[12,14,15],[16,17,18],[16,18,19]
		)
	)
	(entity
		:name 'CornellBox-Original.006'
		:type 'mesh'
		:materials 'shortBox'
		:mesh 'CornellBox-Original.006'
		:transform [1.0,0.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,0.0,1.0]
	)
	(mesh
		:name 'CornellBox-Original.007'
		(attribute
			:type 'p'
			[-0.530000, -0.090000, 1.200000],[0.040000, 0.090000, 1.200000],[-0.140000, 0.670000, 1.200000],[-0.710000, 0.490000, 1.200000],[-0.530000, -0.090000, -0.000000],[-0.530000, -0.090000, 1.200000],[-0.710000, 0.490000, 1.200000],[-0.710000, 0.490000, 0.000000],[-0.710000, 0.490000, 0.000000],[-0.710000, 0.490000, 1.200000],[-0.140000, 0.670000, 1.200000],[-0.140000, 0.670000, 0.000000],[-0.140000, 0.670000, 0.000000],[-0.140000, 0.670000, 1.200000],[0.040000, 0.090000, 1.200000],[0.040000, 0.090000, 0.000000],[0.040000, 0.090000, 0.000000],[0.040000, 0.090000, 1.200000],[-0.530000, -0.090000, 1.200000],[-0.530000, -0.090000, -0.000000]
		)
		(attribute
			:type 'n'
			[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[-0.000000, 0.000000, 1.000000],[-0.955064, -0.296399, -0.000000],[-0.955064, -0.296399, -0.000000],[-0.955064, -0.296399, -0.000000],[-0.955064, -0.296399, -0.000000],[-0.301131, 0.953583, 0.000000],[-0.301131, 0.953583, 0.000000],[-0.301131, 0.953583, 0.000000],[-0.301131, 0.953583, 0.000000],[0.955064, 0.296399, 0.000000],[0.955064, 0.296399, 0.000000],[0.955064, 0.296399, 0.000000],[0.955064, 0.296399, 0.000000],[0.301131, -0.953583, -0.000000],[0.301131, -0.953583, -0.000000],[0.301131, -0.953583, -0.000000],[0.301131, -0.953583, -0.000000]
		)
		(faces
			[0,1,2],[0,2,3],[4,5,6],[4,6,7],[8,9,10],[8,10,11],[12,13,14],[12,14,15],[16,17,18],[16,18,19]
		)
	)
	(entity
		:name 'CornellBox-Original.007'
		:type 'mesh'
		:materials 'tallBox'
		:mesh 'CornellBox-Original.007'
		:transform [1.0,0.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,0.0,1.0]
	)
)
