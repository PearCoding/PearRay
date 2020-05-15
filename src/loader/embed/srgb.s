.section .rodata
    .global srgb_coeff_data
    .type   srgb_coeff_data, @object
    .balign  64
srgb_coeff_data:
    .incbin "embed/srgb.coeff"
srgb_coeff_end:
    .global srgb_coeff_size
    .type   srgb_coeff_size, @object
    .balign  64
srgb_coeff_size:
    .int    srgb_coeff_end - srgb_coeff_data
    .balign 64
.text
