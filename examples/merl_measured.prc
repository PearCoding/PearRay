(scene
	; This scene file contains three MERL material references
	; - blue-fabric.binary
	; - orange-paint.binary
	; - gold-metallic-paint.binary
	; and requires the plugin merl_measured to be enabled.
	; The three binary files are not included into the repository and have to be acquired from the MERL database
	; and copied next to this file

	:name 'material_array'
	:render_width 1000
	:render_height 1000
	(sampler 
	  :slot 'aa'
	  :type 'hammersley'
	  :sample_count 128
	)
	(filter 
	  :slot 'pixel'
	  :type 'mitchell'
	  :radius 1
	)
	(integrator 
	  :type 'direct'
	)
	; Outputs
	(output
		:name 'image'
		(channel :type 'color' :color 'srgb' )
	)
	; Camera
	(camera
		:name 'Camera'
		:type 'standard'
	)
	(light
		:name 'sky'
		:type 'sky'
		:turbidity 3
	)
	(light
		:name 'sun'
		:type 'sun'
		:turbidity 3
		:radius 4
	)
	; Entity Sphere
	(entity
		:name 'Sphere1'
		:type 'sphere'
		:material 'Material1'
		:radius 1
		:position [-2,7,-1]
	)
	(entity
		:name 'Sphere2'
		:type 'sphere'
		:material 'Material2'
		:radius 1
		:position [0,7,-1]
	)
	(entity
		:name 'Sphere3'
		:type 'sphere'
		:material 'Material3'
		:radius 1
		:position [2,7,-1]
	)
	; Entity Plane
	(entity
		:name 'Ground'
		:type 'plane'
		:material 'Ground'
		:x_axis [200,0,0]
		:y_axis [0,100,0]
		:position [-100,0,-1.85]
	)
	; Materials
	(material
		:name 'Material1'
		:type 'merl_measured'
		:filename "blue-fabric.binary"
	)
	(material
		:name 'Material2'
		:type 'merl_measured'
		:filename "orange-paint.binary"
	)
	(material
		:name 'Material3'
		:type 'merl_measured'
		:filename "gold-metallic-paint.binary"
	)
	(material
		:name 'Ground'
		:type 'diffuse'
		:albedo 0.95
	)
)
