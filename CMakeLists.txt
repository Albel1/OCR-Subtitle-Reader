cmake_minimum_required(VERSION 3.0.0)
if (POLICY CMP0025)
    cmake_policy(SET CMP0025 NEW)
endif ()

set (CMAKE_CXX_STANDARD 20)

#set (GCC_COVERAGE_COMPILE_FLAGS "-framework Python")
#add_definitions({${GCC_COVERAGE_COMPILE_FLAGS}})

project(SubReader VERSION 0.1.0)

include(CTest)
enable_testing()

find_package(CURL REQUIRED)

find_package(nlohmann_json 3.10.5 REQUIRED)

get_cmake_property(_variableNames VARIABLES)
list (SORT _variableNames)
foreach (_variableName ${_variableNames})
    message(STATUS "${_variableName}=${${_variableName}}")
endforeach()



find_package( OpenCV REQUIRED )

find_package( PkgConfig REQUIRED)

pkg_search_module( TESSERACT REQUIRED tesseract )

pkg_search_module( LEPTONICA REQUIRED lept )

include_directories( ${TESSERACT_INCLUDE_DIRS} )

include_directories( ${LEPTONICA_INCLUDE_DIRS} )

include_directories( ${OpenCV_INCLUDE_DIRS} )

link_directories( ${TESSERACT_LIBRARY_DIRS} )

link_directories( ${LEPTONICA_LIBRARY_DIRS} )


add_executable(SubReader src/main.cpp src/Selection.cpp src/Properties.cpp src/SelectionCanvas.cpp src/base64.cpp)


target_link_libraries(SubReader nlohmann_json::nlohmann_json)
target_link_libraries( SubReader ${OpenCV_LIBS} )
target_link_libraries( SubReader ${TESSERACT_LIBRARIES} )
target_link_libraries( SubReader ${LEPTONICA_LIBRARIES} )


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})



include(CPack)