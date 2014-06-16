cmake_minimum_required(VERSION 2.8)
project(gui3)

add_executable(gui3.exe gui3.cpp)

Find_Package( SDL REQUIRED )
Find_Package( OpenGL )
find_package(OpenCV 2.3.1 REQUIRED)
find_package( GLEW REQUIRED )
find_package( OpenAL REQUIRED )

if(NOT OpenCV_FOUND)
	message(FATAL_ERROR "OPENCV not found" )
endif(NOT OpenCV_FOUND)

if( NOT SDL_FOUND )
  message( FATAL_ERROR "SDL was not found ")
endif( NOT SDL_FOUND )

if( NOT OPENGL_FOUND )
  message( FATAL_ERROR "Opegl was not found ")
endif( NOT OPENGL_FOUND )

if( NOT OPENAL_FOUND )
  message( FATAL_ERROR "Opegl was not found ")
endif( NOT OPENAL_FOUND )




add_library( glew_static STATIC IMPORTED)
set_target_properties( glew_static PROPERTIES
  IMPORTED_LOCATION /home/mmfps/src/glew-1.9.0/lib/libGLEW.a)


include_directories(
  ${SDL_INCLUDE_DIR}
  ${OPENGL_INCLUDE_DIR}
  ${OpenCV_INCLUDE_DIRS}
  ${OPENAL_INCLUDE_DIR}
)


target_link_libraries(
  gui3.exe
  ${SDL_LIBRARY}
  ${OPENGL_LIBRARIES}
  ${OpenCV_LIBRARIES}
  ${OPENAL_LIBRARY}
  glew_static
)



install( TARGETS gui3.exe DESTINATION ${PROJECT_SOURCE_DIR} )
