; Original CornellBox converted to PearRay format
(scene
	:render_width 500
	:render_height 500
	:camera 'Camera'
	
	; Settings
	(integrator
		:type 'vcm'
		:max_ray_depth 6	
	)
	(sampler
		:slot 'aa'
		:type 'mjitt'
		:sample_count 1024
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
		:type 'diffuse'
		:albedo (refl 0.725000 0.710000 0.680000)
	)
	(material
		:name 'tallBox'
		:type 'diffuse'
		:albedo (refl 0.725000 0.710000 0.680000)
	)

	
	(include "cornellbox_mesh.prc.inc")
)
