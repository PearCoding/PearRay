(scene
	:name				"Test"
	:camera				"Camera"
	
	;; ------------------------------------------------ Scene Tree
	(entity
		:name			"Camera"
		:type			"camera"
		:projection		"orthographic"
		;:projection		"perspective"
		
		:width			1.33333
		:height			1
		:lensDistance	1
		
		:lookAt			[0,0,0]
		:position		[0,10,-3]
	)
	
	(entity
		:name			"Light1"
		:type			"pointLight"
		
		:material		"Light"
		
		:position		[0,10,0]
	)
	
	(entity
		:name 			"1"
		:type			"sphere"
		:radius			0.5
		:material		"Mat1"
		:position		[-1,0,-1]
	)
	
	(entity
		:name 			"2"
		:type			"sphere"
		:radius			0.5
		:material		"Mat2"
		:position		[0,0,-1]
	)
	
	(entity
		:name 			"3"
		:type			"sphere"
		:radius			0.5
		:material		"Mat3"
		:position		[1,0,-1]
	)
	
	(entity
		:name 			"4"
		:type			"sphere"
		:radius			0.5
		:material		"Mat4"
		:position		[-1,0,0]
	)
	
	(entity
		:name 			"5"
		:type			"sphere"
		:radius			0.5
		:material		"Mat5"
		:position		[0,0,0]
	)
	
	(entity
		:name 			"6"
		:type			"sphere"
		:radius			0.5
		:material		"Mat6"
		:position		[1,0,0]
	)
	
	(entity
		:name 			"7"
		:type			"sphere"
		:radius			0.5
		:material		"Mat7"
		:position		[-1,0,1]
	)
	
	(entity
		:name 			"8"
		:type			"sphere"
		:radius			0.5
		:material		"Mat8"
		:position		[0,0,1]
	)
	
	(entity
		:name 			"9"
		:type			"sphere"
		:radius			0.5
		:material		"Mat9"
		:position		[1,0,1]
	)
	
	(entity
		:name			"Ground"
		:type			"box"
		:position		[0, -0.75, 0]
		:size			[10, 0.5, 10]
		:material		"Grid"
	)

	(entity
		:name			"Background"
		:type			"box"
		:position		[0, 4.5, 5]
		:size			[10, 10, 1]
		:material		"Grid"
	)
	
	(entity
		:name			"Left"
		:type			"box"
		:position		[-5, 4.5, 0]
		:size			[1, 10, 10]
		:material		"Grid"
	)
	
	(entity
		:name			"Right"
		:type			"box"
		:position		[5, 4.5, 0]
		:size			[1, 10, 10]
		:material		"Grid"
	)
	
	;; ------------------------------------------------ Meshes
	
	;; ------------------------------------------------ Materials
	
	(material
		:name			"Gray1"
		:type			"standard"
		
		:albedo			"Gray1"
		:roughness		1
	)
	
	(material
		:name			"Gray2"
		:type			"standard"
		
		:albedo			"Gray2"
		:roughness		0
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
		:type			"light"

		:emission		"White_E"
		:shading		false
		:light			true
		:selfShadow		false
		:cameraVisible	false
	)
	
	(material
		:name			"Mat1"
		:type			"standard"
		:albedo			"Blue"
		:roughness		0
		:reflectivity	0
		:specularity	"White"
	)
	
	(material
		:name			"Mat2"
		:type			"standard"
		:albedo			"Blue"
		:roughness		0.5
		:reflectivity	0
		:specularity	"White"
	)
	
	(material
		:name			"Mat3"
		:type			"standard"
		:albedo			"Blue"
		:roughness		1
		:reflectivity	0
		:specularity	"White"
	)
	
	(material
		:name			"Mat4"
		:type			"standard"
		:albedo			"Blue"
		:roughness		0
		:reflectivity	0.5
		:specularity	"White"
	)
	
	(material
		:name			"Mat5"
		:type			"standard"
		:albedo			"Blue"
		:roughness		0.5
		:reflectivity	0.5
		:specularity	"White"
	)
	
	(material
		:name			"Mat6"
		:type			"standard"
		:albedo			"Blue"
		:roughness		1
		:reflectivity	0.5
		:specularity	"White"
	)
	
	(material
		:name			"Mat7"
		:type			"standard"
		:albedo			"Blue"
		:roughness		0
		:reflectivity	1
		:specularity	"White"
	)
	
	(material
		:name			"Mat8"
		:type			"standard"
		:albedo			"Blue"
		:roughness		0.5
		:reflectivity	1
		:specularity	"White"
	)
	
	(material
		:name			"Mat9"
		:type			"standard"
		:albedo			"Blue"
		:roughness		1
		:reflectivity	1
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
		:data			(rgb 1 1 1)
	)
)