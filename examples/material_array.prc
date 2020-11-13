(scene
	; This scene file contains three glass materials with different refraction indices
	:name 'material_array'
	:render_width 1000
	:render_height 1000
	(sampler 
	  :slot 'aa'
	  :type 'hammersley'
	  :sample_count 1024
	)
	(filter 
	  :slot 'pixel'
	  :type 'mitchell'
	  :radius 1
	)
	(integrator 
	  :type 'bidi'
	  ;:max_ray_depth 64
	  :gather_mode 'sphere'
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
	;(light
	;	:name 'env'
	;	:type 'env'
	;	:radiance (illuminant "D65")
	;)
	;(light
	;	:name 'dist'
	;	:type 'distant'
	;	:radiance 1
	;)
	; Materials
	(material
		:name 'Material1'
		;:type 'roughglass'
		:type 'glass'
		:roughness 0.0
		:specularity 1
		:transmission 1
		:spectral_varying false
		:index (lookup_index "bk7")
	)
	(material
		:name 'Material2'
		:type 'glass'
		:specularity 1
		:transmission 1
		:spectral_varying false
		:index (lookup_index "bk7")
	)
	(material
		:name 'Material3'
		:type 'glass'
		:index (lookup_index "diamond")
	)
	(material
		:name 'Ground'
		:type 'diffuse'
		:albedo 0.95
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
		:x_axis [20,0,0]
		:y_axis [0,10,0]
		:position [-10,0,-1.85]
	)
)
