; generated by pearray exporter v0.6 with blender 2.82 (sub 7)
; at 2020-07-12 10:52:25.910211
(scene
	:render_width 1000
	:render_height 1000
	:camera 'Camera'
	; Settings
	(integrator
		:type 'DIRECT'
		:max_ray_depth 8
		:light_sampe_count 1
		:msi false
	)
	(sampler
		:slot 'aa'
		:type 'HALTON'
		:sample_count 512
	)
	(filter
		:slot 'pixel'
		:type 'MITCHELL'
		:radius 0
	)
	; Outputs
	(output
		:name 'image'
		(channel :type 'color' :color 'xyz' )
	)
	; Camera
	(camera
		:name 'Camera'
		:type 'standard'
		:width 0.720000
		:height 0.720000
		:zoom 1.000000
		:fstop 0.000000
		:apertureRadius 0.500000
		:localDirection [0,0,-1]
		:localUp [0,1,0]
		:localRight [1,0,0]
		:near 0.100000
		:far 100.000000
		:transform [0.6859206557273865,-0.32401347160339355,0.6515582203865051,2.552889823913574,0.7276763319969177,0.305420845746994,-0.6141703724861145,-2.3955678939819336,0.0,0.8953956365585327,0.44527140259742737,1.6739132404327393,0.0,0.0,0.0,1.0]
	)
	; Background
	; Lights
	; Light Area
	(emission
		:name 'Area_em'
		:type 'standard'
		:radiance (smul (illuminant 'D65') 10)
	)
	(entity
		:name 'Area'
		:type 'plane'
		:centering true
		:width 0.500000
		:height -0.500000
		:emission 'Area_em'
		:camera_visible false
		:transform [1.0,0.0,0.0,0.0,0.0,-4.371138828673793e-08,-1.0,-1.0,0.0,1.0,-4.371138828673793e-08,0.25,0.0,0.0,0.0,1.0]
	)
	; Primitives
	(entity
		:name 'Sphere'
		:type 'sphere'
		:radius 1.00000
		:material 'SphereLeft'
		:transform [0.3999999761581421,0.0,0.0,-0.5,0.0,0.3999999761581421,0.0,0.5,0.0,0.0,0.3999999761581421,0.4000000059604645,0.0,0.0,0.0,1.0]
	)
	(entity
		:name 'Sphere.001'
		:type 'sphere'
		:radius 1.00000
		:material 'SphereRight'
		:transform [0.3999999761581421,0.0,0.0,0.5,0.0,0.3999999761581421,0.0,0.5,0.0,0.0,0.3999999761581421,0.4000000059604645,0.0,0.0,0.0,1.0]
	)
	; Meshes
	(mesh
		:name 'Plane'
		(attribute
			:type 'p'
			[-1.000000, -1.000000, 0.000000],[1.000000, -1.000000, 0.000000],[1.000000, 1.000000, 0.000000],[-1.000000, 1.000000, 0.000000]
		)
		(attribute
			:type 'n'
			[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[0.000000, 0.000000, 1.000000],[-0.000000, 0.000000, 1.000000]
		)
		(attribute
			:type 'uv'
			[0.000000, 0.000000],[1.000000, 0.000000],[1.000000, 1.000000],[0.000000, 1.000000]
		)
		(faces
			[0,1,2],[0,2,3]
		)
	)
	(entity
		:name 'Plane'
		:type 'mesh'
		:materials 'Floor'
		:mesh 'Plane'
		:transform [1.0,0.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,0.0,1.0]
	)
	; Curves
	; Particle Systems
	; Materials
	(spectrum
		:name 'Floor_diffuse_color'
		:data (refl 0.800000 0.800000 0.800000)
	)
	(material
		:name 'Floor'
		:type 'diffuse'
		:albedo 'Floor_diffuse_color'
	)
	(spectrum
		:name 'SphereLeft_diffuse_color'
		:data (refl 0.000000 0.5000000 1.000000)
	)
	(material
		:name 'SphereLeft'
		:type 'diffuse'
		:albedo 'SphereLeft_diffuse_color'
	)
	(spectrum
		:name 'SphereRight_diffuse_color'
		:data (refl 1.000000 0.500000 0.000000)
	)
	(material
		:name 'SphereRight'
		:type 'diffuse'
		:albedo 'SphereRight_diffuse_color'
	)
)