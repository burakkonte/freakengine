add_library(freak_physics STATIC)

target_sources(freak_physics PRIVATE
    src/PhysicsWorld.cpp
    src/CollisionWorld.cpp
)

target_include_directories(freak_physics PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(freak_physics
    PUBLIC
        freak_core
        glm::glm
)
