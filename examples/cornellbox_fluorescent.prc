; Original CornellBox converted to PearRay format
(scene
	:name 'cornellbox'
	:render_width 1000
	:render_height 1000
	:camera 'Camera'
	; Settings
	(integrator
		:type 'direct'
		:max_ray_depth 16
		:max_camera_ray_depth 4
		:max_light_ray_depth 4
		:contract_ratio 0.2
		:max_light_ray_depth 16
		:soft_max_light_ray_depth 6
	)
	(sampler
		:slot 'aa'
		:type 'mjitt'
		:sample_count 128
	)
	(spectral_mapper
		:type 'spd'
		;:weighting 'none'
		;:smooth_iterations 2
	)
	; Outputs
	(output
		:name 'image'
		(channel :type 'color' :color 'srgb' )
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
		:radiance (smul (spd 'UVLight.csv') 100)
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
		:type 'fluorescent'
		:albedo 1
		:absorption (spd 'fluorophores/FixableDye.csv' 1 true)
		:emission (spd 'fluorophores/FixableDye.csv' 2 true)
	)
	(material
		:name 'tallBox'
		:type 'fluorescent'
		:albedo 1
		:absorption (spd 'fluorophores/BrilliantUltraviolet.csv' 1 true)
		:emission (spd 'fluorophores/BrilliantUltraviolet.csv' 2 true)
	)

	(include "cornellbox_mesh.prc.inc")
)
