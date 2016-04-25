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
		
		:lookAt			[0,0,1]
		:position		[0,0,-2]
	)
	
	(entity
		:name			"Light1"
		:type			"pointLight"
		
		:material		"Light"
		
		:position		[2,2,-1]
	)
	
	(entity
		:name 			"Test"
		:type			"sphere"
		:radius			0.5
		:material		"DebugDirectLight"
		:position		[0,0,0]
	)

	(entity
		:name			"Plane1"
		:type			"plane"
		:position		[-5, -0.5, 5]
		:xAxis			10
		:yAxis			10
		:material		"DebugDirectLight"
	)
	
	(entity
		:name			"Plane2"
		:type			"plane"
		:position		[-5, -0.5, -5]
		:xAxis			10
		:yAxis			[0, 0, 10]
		:material		"DebugDirectLight"
	)
	
	(entity
		:name			"Plane3"
		:type			"plane"
		:position		[-5, -0.5, 5]
		:rotation		(euler 0 45 0)
		:xAxis			[0, 0, -10]
		:yAxis			10
		:material		"DebugDirectLight"
	)
	
	(entity
		:name			"Plane4"
		:type			"plane"
		:position		[5, -0.5, 5]
		:xAxis			[0, 0, -10]
		:yAxis			10
		:material		"DebugDirectLight"
	)
	
	;; ------------------------------------------------ Meshes
	
	;; ------------------------------------------------ Materials
	(material
		:name			"Red"
		:type			"standard"
		
		:reflectance	"Red"
		:roughness		0.3
	)
	
	(material
		:name			"Blue"
		:type			"standard"
		
		:reflectance	"Blue"
		:roughness		0.3
	)
	
	(material
		:name			"Gray1"
		:type			"standard"
		
		:reflectance	"Gray1"
		:roughness		1
	)
	
	(material
		:name			"Gray2"
		:type			"standard"
		
		:reflectance	"Gray2"
		:roughness		1
	)
	
	(material
		:name			"White"
		:type			"standard"
		
		:reflectance	"White"
		:roughness		1
	)
	
	(material
		:name			"Grid"
		:type			"grid"
		:first			"Gray1"
		:second			"Gray2"
		:gridCount		10
	)
	
	(material
		:name			"Light"
		:type			"standard"

		:emission		"White_E"
		:shading		false
		:light			true
		:selfShadow		false
		:cameraVisible	false
	)
	
	(material
		:name			"Light2"
		:type			"standard"

		:emission		"Blue_E"
		:shading		false
		:light			true
		:selfShadow		false
	)
	
	(material
		:name			"Env"
		:type			"standard"

		:reflectance	"Mirror"
		:roughness		0
		:emission		"Blue_E"
	)
	
	(material
		:name			"Mirror"
		:type			"standard"
		
		:reflectance	"Mirror"
		:roughness		0
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
		:name			"DebugDirectLight"
		:type			"debug"
		:show			"directLight"
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
)