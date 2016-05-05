(scene
	:name				"Test"
	:camera				"Camera"
	
	;; ------------------------------------------------ Scene Tree
	(entity
		:name			"Camera"
		:type			"camera"
		;:projection		"orthographic"
		:projection		"perspective"
		
		:width			1.33333
		:height			1
		:lensDistance	1
		
		:lookAt			[0,1,0]
		:position		[0,1,-3.5]
	)
	
	(entity
		:name			"Light1"
		:type			"plane"
		
		:material		"Light"
		:xAxis			1.5
		:yAxis			[0, 0, -1.5]
		:position		[-0.75,3.65,0.75]
	)
	
	(entity
		:name 			"Content1"
		:type			"sphere"
		:radius			0.5
		:material		"Mirror"
		:position		[-1,0.5,0.5]
	)
	
	(entity
		:name 			"Content2"
		:type			"sphere"
		:radius			0.5
		:material		"Glass"
		:position		[1,0.5,-0.5]
	)
	
	(entity
		:name 			"Block1"
		:type			"box"
		:size			[1, 2, 1]
		:position		[0.5,1,1]
		:rotation		(euler 0 45 0)
		:material		"Mat1"
	)
	
	(entity
		:name			"Ground"
		:type			"box"
		:position		[0, -0.25, 0]
		:size			[6, 0.5, 6]
		:material		"Grid"
	)
	
	(entity
		:name			"Top"
		:type			"box"
		:position		[0, 4.25, 0]
		:size			[6, 0.5, 6]
		:material		"White"
	)

	(entity
		:name			"Back"
		:type			"box"
		:position		[0, 2, 3]
		:size			[6, 4, 0.5]
		:material		"White"
	)

	; (entity
		; :name			"Front"
		; :type			"box"
		; :position		[0, 2, -3]
		; :size			[6, 4, 0.5]
		; :material		"Grid"
	; )
	
	(entity
		:name			"Left"
		:type			"box"
		:position		[-3, 2, 0]
		:size			[0.5, 4, 6]
		:material		"Green"
	)
	
	(entity
		:name			"Right"
		:type			"box"
		:position		[3, 2, 0]
		:size			[0.5, 4, 6]
		:material		"Red"
	)
	
	;; ------------------------------------------------ Meshes
	
	;; ------------------------------------------------ Materials
	
	(material
		:name			"Gray1"
		:type			"standard"
		
		:albedo			"Gray1"
		:roughness		1
		:reflectivity 	0
	)
	
	(material
		:name			"Gray2"
		:type			"standard"
		
		:albedo			"Gray2"
		:roughness		1
		:reflectivity 	0
	)
	
	(material
		:name			"Grid"
		:type			"grid"
		:first			"Gray1"
		:second			"Gray2"
		:gridCount		10
	)
	
	(material
		:name			"White"
		:type			"standard"
		
		:albedo			"White"
		:roughness		1
		:reflectivity 	0
	)
	
	(material
		:name			"Red"
		:type			"standard"
		
		:albedo			"Red"
		:roughness		1
		:reflectivity 	0
	)
	
	(material
		:name			"Green"
		:type			"standard"
		
		:albedo			"Green"
		:roughness		1
		:reflectivity 	0
	)
	
	(material
		:name			"Light"
		:type			"light"

		:emission		"Sun"
		:shading		false
		:light			true
		:selfShadow		false
		:cameraVisible	true
	)
	
	(material
		:name			"Mirror"
		:type			"mirror"
	)
	
	(material
		:name			"Glass"
		:type			"glass"
		:specularity	"White"
		:index			1
	)
	
	(material
		:name			"Mat1"
		:type			"standard"
		:albedo			"White"
		:roughness		1
		:reflectivity	0
		:specularity	"White"
	)
	
	(material
		:name			"DebugNormal"
		:type			"debug"
		:show			"normal"
	)
	
	(material
		:name			"DebugUV"
		:type			"debug"
		:show			"uv"
	)
	
	(material
		:name			"DebugBoundingBox"
		:type			"debugBoundingBox"
		:color			"Red"
		:density		0.4
	)
	
	;; ------------------------------------------------ Spectrums
	(spectrum			;; Reflexive
		:name			"Red"
		:data			(rgb 1 0 0)
	)
	
	(spectrum			;; Reflexive
		:name			"Green"
		:data			(rgb 0 1 0)
	)
	
	(spectrum			;; Reflexive
		:name			"Blue"
		:data			(rgb 0 0 1)
	)
	
	(spectrum			;; Reflexive
		:name			"Gray1"
		:data			(rgb 0.6 0.6 0.6)
	)
	
	(spectrum			;; Reflexive
		:name			"Gray2"
		:data			(rgb 0.3 0.3 0.3)
	)
	
	(spectrum			;; Reflexive
		:name			"White"
		:data			(rgb 1 1 1)
	)
	
	(spectrum			;; Reflexive
		:name			"Mirror"
		:data			(field
			:default	1
			)
	)
	
	(spectrum			;; Emissive
		:name			"Red_E"
		:emissive		true
		:data			(rgb 10 0 0)
	)
	
	(spectrum			;; Emissive
		:name			"Blue_E"
		:emissive		true
		:data			(rgb 0 0 2)
	)
	
	(spectrum			;; Emissive
		:name			"White_E"
		:emissive		true
		:data			(rgb 2 2 2)
	)
	
	(spectrum			;; Emissive
		:name			"Sun"
		:data			(temperature_norm 5500 1)
	)
)