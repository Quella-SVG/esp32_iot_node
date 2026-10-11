idf_component_register(
    SRCS "onenet.c" "token.c"
    INCLUDE_DIRS "include"
    PRIV_INCLUDE_DIRS "."
    REQUIRES mqtt mbedtls json
)
