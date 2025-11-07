
file(GLOB_RECURSE TRACKER_SOURCES CONFIGURE_DEPENDS *.c *.h)
list(REMOVE_ITEM TRACKER_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/main.c")

add_library(tracker_lib STATIC ${TRACKER_SOURCES})
target_include_directories(tracker_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${PROJECT_SOURCE_DIR}/external/libuv/include
)

find_package(Threads REQUIRED)


target_link_libraries(tracker_lib PUBLIC
    uv
    Threads::Threads
)



add_executable(tracker main.c)
target_link_libraries(tracker PRIVATE tracker_lib)