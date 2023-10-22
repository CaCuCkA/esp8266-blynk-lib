set(SOURCE_DIRS
        src
        src/internal
        src/stuff
        )

set(INCLUDE_DIRS
        include
        )

set(PRIVATE_INCLUDE_DIRS
        include/internal
        include/stuff
        )

idf_component_register(
        SRC_DIRS ${SOURCE_DIRS}
        INCLUDE_DIRS ${INCLUDE_DIRS}
        PRIV_INCLUDE_DIRS ${PRIVATE_INCLUDE_DIRS}
)

# Set compile definitions
target_compile_definitions(${COMPONENT_LIB} PRIVATE
        FREERTOS      # detecting system for blynk_freertos_port
        LOG_USE_COLOR # on color Logs
        LOG_WITH_TIME # on time in logging
        )
