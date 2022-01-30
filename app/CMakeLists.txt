#
# Dependencies
#
find_package(OpenGL REQUIRED COMPONENTS OpenGL )
find_package(glfw3 CONFIG REQUIRED)
find_package(Enet REQUIRED)

#
# Config
#
include(PlatformConfig.cmake)

#
# Sources
#
set(SOURCES
    src/bitFont.c
    src/bitFont.h
    src/effects.c
    src/effects.h
    src/item.c
    src/item.h
    src/level.c
    src/level.h
    src/main_server.c
    src/main_server.h
    src/main.c
    src/main.h
    src/myMath.c
    src/myMath.h
    src/myopengl.h
    src/net_client.c
    src/net_client.h
    src/netPackets.h
    src/player.c
    src/player.h
    src/powerup.c
    src/powerup.h
    src/projectile.c
    src/projectile.h
    src/util.h
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX src-app FILES ${SOURCES})


#
# Target
#
add_executable(netfight ${SOURCES})
target_link_libraries(netfight
    PRIVATE
        glfw
        Enet::enet
        OpenGL::GL
        OpenGL::GLU
)

# Add build dir to find config.h
target_include_directories(netfight
    PRIVATE
        "${CMAKE_CURRENT_BINARY_DIR}"
)

# Add a build step to copy the data dir
add_custom_command(
    TARGET netfight POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/data
            ${CMAKE_CURRENT_BINARY_DIR}/data)
