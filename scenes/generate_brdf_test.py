import sys

HEADER = """
(scene
	:name				"Test"
	:camera				"Camera"
"""

FOOTER = """
	;; ------------------------------------------------ Util Materials
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
		:gridCount		20
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
)"""

LIGHT_POS = [0, 4, 0]
CAMERA_POS = [0, 20, -10]
CAMERA_TARGET = [0, 0, 0]
CAMERA_PERS = True

SPHERE_RADIUS = 0.5
ENTITY_PADDING = 0.5

ROUGHNESS_SIZE = 10
REFLECTIVITY_SIZE = 10

GRID_PADDING = 5

if __name__ == '__main__':
    output = HEADER
    output += """
        (entity
		:name			"Camera"
		:type			"camera"
		:projection		"{0}"

		:width			1.33333
		:height			1
		:lensDistance	1

		:lookAt			{1}
		:position		{2}
	)

	(entity
		:name			"Light1"
		:type			"pointLight"

		:material		"Light"

		:position		{3}
	)
    """.format( "perspective" if CAMERA_PERS else "orthographic",
        "[{0},{1},{2}]".format(CAMERA_TARGET[0], CAMERA_TARGET[1], CAMERA_TARGET[2]),
        "[{0},{1},{2}]".format(CAMERA_POS[0], CAMERA_POS[1], CAMERA_POS[2]),
        "[{0},{1},{2}]".format(LIGHT_POS[0], LIGHT_POS[1], LIGHT_POS[2]),
    )

    output += ";; Spheres:"
    entity_size = (SPHERE_RADIUS * 2 + ENTITY_PADDING)

    for i in range(0, ROUGHNESS_SIZE + 1):
        for j in range(0, REFLECTIVITY_SIZE + 1):
            roughness = i/ROUGHNESS_SIZE
            reflectivity = j/REFLECTIVITY_SIZE

            posX = (i - ROUGHNESS_SIZE / 2) * entity_size
            posY = (j - REFLECTIVITY_SIZE / 2) * entity_size

            index = j * (ROUGHNESS_SIZE + 1) + i
            output += """
    (entity
        :name 			"Entitiy_{0}"
        :type			"sphere"
        :radius			{1}
        :material		"Mat_{0}"
        :position		{2}
    )
            """.format(index, SPHERE_RADIUS,
                       "[{0},{1},{2}]".format(posX, SPHERE_RADIUS, posY))

            output += """
    (material
        :name			"Mat_{0}"
        :type			"standard"
        :albedo			"White"
        :roughness		{1}
        :reflectivity	{2}
        :specularity	"White"
    )
            """.format(index, roughness, reflectivity)

    output += """
	(entity
		:name			"Ground"
		:type			"box"
		:position		[0, -0.25, 0]
		:size			[{0}, 0.5, {1}]
		:material		"Grid"
	)
    """.format(ROUGHNESS_SIZE*entity_size + GRID_PADDING * 2, REFLECTIVITY_SIZE*entity_size + GRID_PADDING * 2)

    output += FOOTER

    filename = "test_scene.prc"
    if len(sys.argv) > 1:
        filename = sys.argv[1]

    f = open(filename, "w")
    f.write(output + "\n")
