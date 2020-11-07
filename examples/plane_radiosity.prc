(scene
	:name 'radiosity_test'
	:render_width 1000
	:render_height 1000
	:spectral_domain 520

	(integrator 
	  :type 'direct'
	  :max_ray_depth 1
	  :light_sample_count 1
	)
	(sampler 
	  :slot 'aa'
	  :type 'hammersley'
	  :sample_count 32
	)
	(filter 
	  :slot 'pixel'
	  :type 'mitchell'
	  :radius 0
	)
	; Outputs
	(output
		:name 'image'
		(channel :type 'color' :color 'xyz' )
		(channel :type 'var' )
	)
	; Camera
	(camera
		:name 'Camera'
		:type 'orthographic'

		:sensor_width 1
		:sensor_height 1
        :localDirection [0, 0,-1]
        :localUp [0, 1, 0]
        :localRight [1, 0, 0]
        :position [0, 0, 1]
	)

    ; Light Area
    (emission
        :name 'Area_em'
        :type 'diffuse'
        :radiance (illuminant "E")
    )
    (entity
        :name 'Area'
        :type 'plane'
        :centering true
        :width 1
        :height -1
        :emission 'Area_em'
        :position [0, 0, 2]
    )

    ; Pure diffuse object
    (material
        :name 'Diffuse'
        :type 'diffuse'
        :albedo 1
    )
    (entity
        :name 'Plane'
        :type 'plane'
        :centering true
        :material 'Diffuse'
    )
)

