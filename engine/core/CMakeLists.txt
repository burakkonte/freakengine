add_library(freak_core STATIC)

target_sources(freak_core PRIVATE
    src/Log.cpp
    src/Timer.cpp
)

target_include_directories(freak_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Tracy is the only external dependency of Core
if(FREAK_ENABLE_TRACY)
    target_link_libraries(freak_core PUBLIC TracyClient)
endif()

target_compile_features(freak_core PUBLIC cxx_std_23)
