SET(pwd ${CMAKE_CURRENT_LIST_DIR})

UNSET(src)

# Uncomment the following lines to explicitly set files to compile from
# CMakeLists.sources
#IF (EXISTS CMakeLists.sources)
#      FILE(STRINGS CMakeLists.sources src)
#ENDIF()
 
# By default, source files are added automatically
IF (NOT src)
    set(src ${src} "${pwd}/general.h" "${pwd}/RayTracer.cpp" "${pwd}/mathutil.cpp")
    AUX_SOURCE_DIRECTORY(${pwd}/fileio src)
    AUX_SOURCE_DIRECTORY(${pwd}/parser src)
    AUX_SOURCE_DIRECTORY(${pwd}/scene src)
    AUX_SOURCE_DIRECTORY(${pwd}/SceneObjects src)
    AUX_SOURCE_DIRECTORY(${pwd}/ui src)
    IF (WIN32)
        AUX_SOURCE_DIRECTORY(${pwd}/win32 src)
        ADD_EXECUTABLE(ray ${src})
    ENDIF (WIN32)
ENDIF(NOT src)

FIND_PACKAGE(OpenMP QUIET)
if(OPENMP_FOUND)
	message(STATUS "OPENMP FOUND")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

option(STOCHASTIC_SUPERSAMPLING "Enable or disable stochastic supersampling when
                                 using AA."
      OFF)

if(STOCHASTIC_SUPERSAMPLING)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSTOCHSSAA")
endif(STOCHASTIC_SUPERSAMPLING)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

option(PATH_TRACING "Enable or disable path tracing."
      ON)

if(PATH_TRACING)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DPATHTRACING")
endif(PATH_TRACING)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

option(FORCEAREA "Force point lights to act as area lights."
       ON)

if(FORCEAREA)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DFORCEAREA")
endif(FORCEAREA)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

add_executable(ray ${src} main.cpp)
add_executable(test-intersect ${src} test-intersect.cpp)

target_link_libraries(ray ${OPENGL_gl_LIBRARY})
SET(FLTK_SKIP_FLUID TRUE)
FIND_PACKAGE(FLTK REQUIRED)
SET_PROPERTY(TARGET ray APPEND PROPERTY INCLUDE_DIRECTORIES ${FLTK_INCLUDE_DIR})
target_link_libraries(ray ${FLTK_LIBRARIES})
FIND_PACKAGE(JPEG REQUIRED)
target_link_libraries(ray ${JPEG_LIBRARIES})
FIND_PACKAGE(PNG REQUIRED)
target_link_libraries(ray ${PNG_LIBRARIES})
FIND_PACKAGE(ZLIB REQUIRED)
target_link_libraries(ray ${ZLIB_LIBRARIES})
target_link_libraries(ray ${OPENGL_glu_LIBRARY})


target_link_libraries(test-intersect ${OPENGL_gl_LIBRARY})
target_link_libraries(test-intersect ${OPENGL_glu_LIBRARY})
target_link_libraries(test-intersect ${FLTK_LIBRARIES})
target_link_libraries(test-intersect ${PNG_LIBRARIES})
target_link_libraries(test-intersect ${ZLIB_LIBRARIES})
target_link_libraries(test-intersect ${FLTK_LIBRARIES})
