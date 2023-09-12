
add_library(tinyxml STATIC
        src/tinystr.cpp
        src/tinyxml.cpp
        src/tinyxmlerror.cpp
        src/tinyxmlparser.cpp
        )
target_include_directories(tinyxml PUBLIC include)
target_include_directories(tinyxml PRIVATE include/tinyxml)
target_compile_definitions(tinyxml PUBLIC TIXML_USE_STL)
target_link_libraries(tinyxml PUBLIC sst-plugininfra::filesystem)
set_property(TARGET tinyxml PROPERTY C_VISIBILITY_PRESET hidden)
set_property(TARGET tinyxml PROPERTY VISIBILITY_INLINES_HIDDEN ON)