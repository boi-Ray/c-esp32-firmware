idf_component_register(SRCS "main.c" "my_memcpy.c"
                       INCLUDE_DIRS ".")
target_compile_options(${COMPONENT_LIB} PRIVATE "-fno-builtin")