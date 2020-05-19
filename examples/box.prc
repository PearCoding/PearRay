(scene
	:name 'box_test_scene'
	:renderWidth 1920
	:renderHeight 1080

	(sampler 
	  :slot 'aa'
	  :type 'sobol'
	  :sample_count 8
	)
	(filter 
	  :slot 'pixel'
	  :type 'mitchell'
	  :radius 1
	)
	(integrator 
	  :type 'debug'
	  :max_ray_depth 4
	  :light_sample_count 1
	  :msi true
	  :mode "validate_material"
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
		(channel :type 'depth')
		(channel :type 'n')
		(channel :type 'uvw')
		(channel :type 'ng')
		(channel :type 'nx')
		(channel :type 'ny')
		(channel :type 'feedback')
	)
	; Camera
	(camera
		:name 'Camera'
		:type 'standard'
		:width 0.914286
		:height 0.514286
		:zoom 1.000000
		:fstop 0.000000
		:apertureRadius 0.500000
		:localDirection [0,0,-1]
		:localUp [0,-1,0]
		:localRight [1,0,0]
		:transform [1.0,0.0,0.0,0.0,0.0,-4.371138828673793e-08,-1.0,-5.050459384918213,0.0,1.0,-4.371138828673793e-08,1.0,0.0,0.0,0.0,1.0]
	)
	; Light Area
	(emission
		:name 'Area_em'
		:type 'standard'
		:radiance (illuminant "D65")
	)
	(entity
		:name 'Area'
		:type 'plane'
		:centering true
		:width 0.800000
		:height -0.800000
		:emission 'Area_em'
		:transform [1.0,0.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,0.0,1.0,1.9618895053863525,0.0,0.0,0.0,1.0]
	)
	; Primitives
	(entity
		:name 'Cone'
		:type 'cone'
		:radius 1.000000
		:height 1.000000
		:center_on true
		:material 'Material.004'
		:transform [0.31341552734375,0.0,0.0,0.4868149757385254,0.0,0.31341552734375,0.0,-0.619674026966095,0.0,0.0,0.6805660128593445,0.33832693099975586,0.0,0.0,0.0,1.0]
	)
	(entity
		:name 'Sphere'
		:type 'sphere'
		:radius 1.000000
		:material 'Glass'
		:transform [0.3499999940395355,0.0,0.0,-0.49090614914894104,0.0,0.3499999940395355,0.0,-0.43932753801345825,0.0,0.0,0.3499999940395355,0.34749284386634827,0.0,0.0,0.0,1.0]
	)
	; Meshes
	(mesh
		:name 'Cube.001'
		(attribute
			:type 'p', [-1.000000, -1.000000, -1.000000], [-1.000000, -1.000000, 1.000000], [-1.000000, 1.000000, 1.000000], [-1.000000, 1.000000, -1.000000], [-1.000000, 1.000000, -1.000000], [-1.000000, 1.000000, 1.000000], [1.000000, 1.000000, 1.000000], [1.000000, 1.000000, -1.000000], [1.000000, 1.000000, -1.000000], [1.000000, 1.000000, 1.000000], [1.000000, -1.000000, 1.000000], [1.000000, -1.000000, -1.000000], [-1.000000, -1.000000, 1.000000], [-1.000000, -1.000000, -1.000000], [1.000000, -1.000000, -1.000000], [1.000000, -1.000000, 1.000000], [1.000000, -1.000000, -1.000000], [-1.000000, -1.000000, -1.000000], [-1.000000, 1.000000, -1.000000], [1.000000, 1.000000, -1.000000], [1.000000, 1.000000, 1.000000], [-1.000000, 1.000000, 1.000000], [-1.000000, -1.000000, 1.000000], [1.000000, -1.000000, 1.000000]
		)
		(attribute
			:type 'n', [-1.000000, -0.000000, 0.000000], [-1.000000, -0.000000, 0.000000], [-1.000000, -0.000000, 0.000000], [-1.000000, -0.000000, 0.000000], [0.000000, 1.000000, 0.000000], [0.000000, 1.000000, 0.000000], [0.000000, 1.000000, 0.000000], [0.000000, 1.000000, 0.000000], [1.000000, -0.000000, 0.000000], [1.000000, -0.000000, 0.000000], [1.000000, -0.000000, 0.000000], [1.000000, -0.000000, 0.000000], [-0.000000, -1.000000, 0.000000], [-0.000000, -1.000000, 0.000000], [-0.000000, -1.000000, 0.000000], [-0.000000, -1.000000, 0.000000], [0.000000, -0.000000, -1.000000], [0.000000, -0.000000, -1.000000], [0.000000, -0.000000, -1.000000], [0.000000, -0.000000, -1.000000], [0.000000, -0.000000, 1.000000], [0.000000, -0.000000, 1.000000], [0.000000, -0.000000, 1.000000], [0.000000, -0.000000, 1.000000]
		)
		(faces
			[0,1,2,3],[4,5,6,7],[8,9,10,11],[12,13,14,15],[16,17,18,19],[20,21,22,23]
		)
	)
	(entity
		:name 'Cube'
		:type 'mesh'
		:materials 'Mirror'
		:mesh 'Cube.001'
		:transform [0.36022281646728516,-0.17389510571956635,0.0,0.412863552570343,0.17389510571956635,0.36022281646728516,0.0,0.24348364770412445,0.0,0.0,0.3999999761581421,0.37800320982933044,0.0,0.0,0.0,1.0]
	)
	(mesh
		:name 'Plane'
		(attribute
			:type 'p', [-1.000000, -1.000000, 0.000000], [1.000000, -1.000000, 0.000000], [1.000000, 1.000000, 0.000000], [-1.000000, 1.000000, 0.000000]
		)
		(attribute
			:type 'n', [0.000000, 0.000000, 1.000000], [0.000000, 0.000000, 1.000000], [0.000000, 0.000000, 1.000000], [0.000000, 0.000000, 1.000000]
		)
		(faces
			[0,1,2,3]
		)
	)
	(entity
		:name 'Plane'
		:type 'mesh'
		:materials 'Material'
		:mesh 'Plane'
		:transform [1.0,0.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,0.0,1.0]
	)
	(entity
		:name 'Plane.001'
		:type 'mesh'
		:materials 'Material.003'
		:mesh 'Plane'
		:transform [1.0,0.0,0.0,0.0,0.0,-4.371138828673793e-08,-1.0,1.0,0.0,1.0,-4.371138828673793e-08,1.0,0.0,0.0,0.0,1.0]
	)
	(entity
		:name 'Plane.002'
		:type 'mesh'
		:materials 'Material.002'
		:mesh 'Plane'
		:transform [-4.371138828673793e-08,4.371138828673793e-08,1.0,-1.0,1.0,1.910685676922942e-15,4.371138828673793e-08,0.0,0.0,1.0,-4.371138828673793e-08,1.0,0.0,0.0,0.0,1.0]
	)
	(entity
		:name 'Plane.003'
		:type 'mesh'
		:materials 'Material.001'
		:mesh 'Plane'
		:transform [-4.371138828673793e-08,4.371138828673793e-08,1.0,1.0,1.0,1.910685676922942e-15,4.371138828673793e-08,0.0,0.0,1.0,-4.371138828673793e-08,1.0,0.0,0.0,0.0,1.0]
	)
	(entity
		:name 'Plane.004'
		:type 'mesh'
		:materials 'Material'
		:mesh 'Plane'
		:transform [1.0,0.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,0.0,1.0,2.0,0.0,0.0,0.0,1.0]
	)
	; Materials
	(spectrum
		:name 'Glass_specular_color'
		:data (refl 1.000000 1.000000 1.000000)
	)
	(material
		:name 'Glass'
		:type 'glass'
		:specularity 'Glass_specular_color'
		:index 1.550000
	)
	(spectrum
		:name 'Material_diffuse_color'
		:data (refl 1.000000 1.000000 1.000000)
	)
	(material
		:name 'Material'
		:type 'diffuse'
		:albedo 'Material_diffuse_color'
	)
	(spectrum
		:name 'Material.001_diffuse_color'
		:data (refl 0.000000 0.000000 1.000000)
	)
	(material
		:name 'Material.001'
		:type 'diffuse'
		:albedo 'Material.001_diffuse_color'
	)
	(spectrum
		:name 'Material.002_diffuse_color'
		:data (refl 1.000000 0.000000 0.000000)
	)
	(material
		:name 'Material.002'
		:type 'diffuse'
		:albedo 'Material.002_diffuse_color'
	)
	(spectrum
		:name 'Material.003_diffuse_color'
		:data (refl 0.000000 1.000000 0.000000)
	)
	(material
		:name 'Material.003'
		:type 'diffuse'
		:albedo 'Material.003_diffuse_color'
	)
	(spectrum
		:name 'Material.004_diffuse_color'
		:data (refl 1.000000 0.500000 0.000000)
	)
	(material
		:name 'Material.004'
		:type 'diffuse'
		:albedo 'Material.004_diffuse_color'
	)
	(spectrum
		:name 'Mirror_specular_color'
		:data (refl 1.000000 1.000000 1.000000)
	)
	(material
		:name 'Mirror'
		:type 'mirror'
		:specularity 'Mirror_specular_color'
		:index 1.550000
	)
)

