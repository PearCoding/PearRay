; generated by pearray exporter v0.7 with blender 2.83.5
; at 2020-12-05 15:25:31.853448
(scene
	:render_width 1000
	:render_height 1000
	:camera 'Camera'
	; Settings
	(integrator
		:type 'DIRECT'
		:max_ray_depth 64
		:soft_max_ray_depth 4
		:max_light_ray_depth 16
		:soft_max_light_ray_depth 2
		:light_sample_count 1
	)
	(sampler
		:slot 'aa'
		:type 'MULTI_JITTER'
		:sample_count 512
	)
	(filter
		:slot 'pixel'
		:type 'MITCHELL'
		:radius 1
	)
	; Outputs
	(output
		:name 'image'
		(channel :type 'color' :color 'xyz')
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
		:transform [0.6859206557273865,-0.32401347160339355,0.6515582203865051,6.030767440795898,0.7276763319969177,0.305420845746994,-0.6141703724861145,-5.673877239227295,0.0,0.8953956365585327,0.44527140259742737,4.050676345825195,0.0,0.0,0.0,1.0]
	)
	; Light
	(light
		:name 'env'
		:type 'environment'
		:radiance (smul (illuminant 'd65') 0.4)
	)
	(emission
		:name 'Light_em'
		:type 'standard'
		:radiance (smul (illuminant 'd65') (illum 1000.000000 1000.000000 1000.000000))
	)
	(entity
		:name 'Light'
		:type 'plane'
		:centering true
		:width 0.100000
		:height -0.100000
		:emission 'Light_em'
		:transform [-0.29086464643478394,-0.7711008191108704,0.5663931965827942,4.076245307922363,0.9551711678504944,-0.1998833566904068,0.21839119493961334,1.0054539442062378,-0.05518905818462372,0.6045247316360474,0.7946722507476807,5.903861999511719,0.0,0.0,0.0,1.0]
	)
	; Materials
	(material
		; A platin conductor... Quite expensive in real life :P
		:name 'BoltMetal'
		:type 'conductor'
		:eta 1.3400
		:k 1.0300
		:specularity (refl 0.926298157 0.894117647 0.88627451)
		:roughness_x 0.04
		:roughness_y 0.12
	)
	(material
		:name 'Floor'
		:type 'diffuse'
		:albedo (checkerboard (refl 0.8 0.6 0.4) (refl 0.0 0.2 0.4) 40 40)
		:specularity 0.36
		:roughness 0.024
	)
	(material
		:name 'GearMetal'
		; A gold conductor... Quite expensive in real life, but not as expensive as platin :P
		:name 'BoltMetal'
		:type 'conductor'
		:eta 1.2500
		:k 0.87000
		:specularity (refl 1 0.784313725 0.349019608)
		:roughness 0.004
	)
	(material
		:name 'Glass'
		:type 'glass'
		:index (lookup_index "bk7")
		:roughness_x 0.02
		:roughness_y 0.1
		:vndf true
	)
	; Primitives
	(include 'meshes/Floor.prc')
	(entity
		:name 'Floor'
		:type 'plane'
		:material 'Floor'
		:width 15
		:height 15
		:centering true
		:transform [4.800000190734863,0.0,0.0,0.0,0.0,4.800000190734863,0.0,0.0,0.0,0.0,4.800000190734863,0.0,0.0,0.0,0.0,1.0]
	)
	(entity
		:name 'GlassPlane1'
		:type 'plane'
		:material 'Glass'
		:width 2
		:height 2
		:centering true
		:transform [0.48812252283096313,0.7071067690849304,0.5116018056869507,1.9197214841842651,-0.48812249302864075,0.7071067690849304,-0.5116018056869507,-1.6557358503341675,-0.723514199256897,2.9802320611338473e-08,0.690309464931488,1.5923221111297607,0.0,0.0,0.0,1.0]
	)
	(entity
		:name 'GlassPlane2'
		:type 'plane'
		:material 'Glass'
		:width 2
		:height -2
		:centering true
		:transform [0.48812252283096313,0.7071067690849304,0.5116018056869507,2.9197214841842651,-0.48812249302864075,0.7071067690849304,-0.5116018056869507,-2.1557358503341675,-0.723514199256897,2.9802320611338473e-08,0.690309464931488,1.1923221111297607,0.0,0.0,0.0,1.0]
	)
	(include 'meshes/Bolt.prc')
	(entity
		:name 'Bolt'
		:type 'mesh'
		:materials 'BoltMetal'
		:mesh 'Bolt'
		:transform [0.10356860607862473,-0.15493150055408478,-0.07259318232536316,-3.00951997299535e-17,-0.001082072383724153,0.08426227420568466,-0.18137997388839722,0.1082143560051918,0.1710917055606842,0.09431911259889603,0.04279639944434166,0.26446327567100525,0.0,0.0,0.0,1.0]
	)
	(include 'meshes/Gear.prc')
	(entity
		:name 'Gear'
		:type 'mesh'
		:materials 'GearMetal'
		:mesh 'Gear'
		:transform [0.9998347163200378,0.009709128178656101,-0.015370063483715057,7.982322689638769e-17,-0.003625559853389859,0.9349521994590759,0.3547552227973938,0.5403232574462891,0.0178146380931139,-0.3546408712863922,0.9348328709602356,0.5720992684364319,0.0,0.0,0.0,1.0]
	)
)
