; Original CornellBox converted to PearRay format
(scene
	:name 'cornellbox'
	:render_width 1000
	:render_height 1000
	:camera 'Camera'
	; Settings
	(integrator
		:type 'vcm'
		:max_ray_depth 16
		:max_camera_ray_depth 4
		:max_light_ray_depth 4
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
		:local_direction [0,0,-1]
		:local_up [0,1,0]
		:local_right [1,0,0]
		:near 0.100000
		:far 100.000000
		:transform [1.0,0.0,0.0,0.0,0.0,0.0,-1.0,-3.93462872505188,0.0,1.0,0.0,1.0,0.0,0.0,0.0,1.0]
	)
	; Lights
	;(light
	;	:name 'sky'
	;	:type 'sky'
	;	:turbidity 3
	;)

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
		:type 'glass'
		:index (lookup_index "bk7")
		:roughness 0.02
	)
	(material
		:name 'tallBox'
		:type 'glass'
		:index (lookup_index "bk7")
	)

	(include "cornellbox_mesh.prc.inc")
)
