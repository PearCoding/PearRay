(scene
	; This scene file contains one Klems material references
	; - aerc6216_kf.xml
	; and requires the plugin klems_measured to be enabled.
	; The one xml file is available as a part of the ladybug-tools bsdf-viewer 
	; https://github.com/ladybug-tools/bsdf-viewer/blob/master/bsdf/klems/aerc6216_kf.xml

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
		:name 'background'
		:type 'env'
		:radiance (illuminant "d65")
	)
	;(light
	;	:name 'sky'
	;	:type 'sky'
	;	:turbidity 3
	;)
	;(light
	;	:name 'sun'
	;	:type 'sun'
	;	:turbidity 3
	;	:radius 4
	;)
	; Materials
	(material
		:name 'Material1'
		:type 'klems_measured'
		:filename "CS-TBK7-12_112916.xml"
		:swap_side false
	)
	;(material
	;	:name 'Material1'
	;	:type 'glass'
	;	:index (lookup_index "bk7")
	;	:thin true
	;)
	(material
		:name 'Ground'
		:type 'diffuse'
		:albedo 0.95
	)
	; Entity Sphere
	(entity
		:name 'Plane1'
		:type 'plane'
		:material 'Material1'
		:x_axis [5,0,0]
		:y_axis [0,0,5]
		:position [-2.5,7,-2.0]
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
)
