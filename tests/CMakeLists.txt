add_executable(sst-plugininfra-tests)
target_include_directories(sst-plugininfra-tests PRIVATE tests)
target_link_libraries(sst-plugininfra-tests PRIVATE ${PROJECT_NAME} ${PROJECT_NAME}::filesystem)
target_sources(sst-plugininfra-tests PRIVATE
        tests.cpp)

add_executable(sst-tixml-tests)
target_sources(sst-tixml-tests PRIVATE ${CMAKE_SOURCE_DIR}/libs/tinyxml/test/xmltest.cpp)
target_link_libraries(sst-tixml-tests PRIVATE tinyxml)

add_custom_target(sst-all-tests ALL)
add_dependencies(sst-all-tests sst-tixml-tests)
add_dependencies(sst-all-tests sst-plugininfra-tests)

add_custom_command(TARGET sst-all-tests
        POST_BUILD
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMAND ${CMAKE_COMMAND} -E echo "Copying tests to test-binary"
        COMMAND ${CMAKE_COMMAND} -E make_directory test-binary
        COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:sst-plugininfra-tests>" test-binary
        COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:sst-tixml-tests>" test-binary
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/libs/tinyxml/test/utf8test.xml test-binary
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/libs/tinyxml/test/utf8testverify.xml test-binary
        )