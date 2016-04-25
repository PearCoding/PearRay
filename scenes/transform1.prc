(scene
	:name				"Transform"
	:camera				"Camera"
	
	;; ------------------------------------------------ Scene Tree
	(entity
		:name			"Camera"
		:type			"camera"
		:projection		"perspective"
		
		:width			1.33333
		:height			1
		:lensDistance	1
		
		:lookAt			[0,0,1]
		:position		[0,0,-2]
	)
		
	(entity
		:name 			"Test"
		:type			"sphere"
		:material		"DebugUV"
		:radius			0.5
		:position		[0,0,0]
		:rotation		(euler 0 45 0)
		:scale			1
	)
	
	;; ------------------------------------------------ Meshes
	
	;; ------------------------------------------------ Materials
	
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
	
	;; ------------------------------------------------ Spectrums
)