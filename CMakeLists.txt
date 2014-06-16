cmake_minimum_required(VERSION 2.4.6)
include($ENV{ROS_ROOT}/core/rosbuild/rosbuild.cmake)

#set(ROS_COMPILE_FLAGS ${ROS_COMPILE_FLAGS} -std=c++0x)

rosbuild_init()
rosbuild_genmsg()
rosbuild_gensrv()
rosbuild_add_boost_directories()
 
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
set(EXTRA_LIBRARY ${PROJECT_SOURCE_DIR}/lib)
include_directories(${PROJECT_SOURCE_DIR}/include)



# Qt package
find_package(Qt4 COMPONENTS QtCore QtGui REQUIRED)
include(${QT_USE_FILE})
add_definitions(-DQT_NO_KEYWORDS)

#gtk package
find_package(GTK2 REQUIRED)
include_directories( ${GTK2_INCLUDE_DIRS} )

find_package(OpenCV 2.3.1 REQUIRED)
if(OpenCV_FOUND)
	message("======== ${OpenCV_VERSION} Found ========= ")
endif(OpenCV_FOUND)

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



find_package(Boost 1.40 COMPONENTS program_options system filesystem REQUIRED)
include_directories( ${Boost_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS} )


#add_library( glew_static STATIC IMPORTED)
#set_target_properties( glew_static PROPERTIES
#  IMPORTED_LOCATION /home/mmfps/src/glew-1.9.0/lib/libGLEW.a)

include_directories(
  ${SDL_INCLUDE_DIR}
  ${OPENGL_INCLUDE_DIR}
  ${OpenCV_INCLUDE_DIRS}
  ${OPENAL_INCLUDE_DIR}
)


#if(Boost_FOUND)
#	message("======== ${Boost_VERSION} Found ========= ")
#endif(Boost_FOUND)




# system executables
rosbuild_add_executable(systemNode SystemNode.cpp)
rosbuild_link_boost(systemNode system thread signals program_options filesystem regex)

# camera executables
#no ros executables required to run the camera node

# speech executables
#python scripts do not need compilation

# face executables
#rosbuild_add_executable(face node_face/face.cpp)
#rosbuild_link_boost(face system thread signals program_options filesystem regex)

# location executables
#rosbuild_add_executable(location node_location/location.cpp)

# object executables
rosbuild_add_executable(retail node_object/RetailRecognizer.cpp)
rosbuild_add_executable(fakeExemplar node_object/FakeExemplar.cpp)
rosbuild_add_executable(fakeExemplarfakeMatlab node_object/FakeExemplar_FakeMatlab.cpp)

# bnet executables
#rosbuild_add_executable(bnet node_bnet/bnet.cpp)

# responder executables
#rosbuild_add_executable(responder node_responder/src/responder.cpp)
#rosbuild_add_executable(gui node_object/gui.cpp)
#rosbuild_add_executable(responder node_responder/responder.cpp)

#rosbuild_add_executable(opencv_cam camera_node/src/opencv_cam.cpp )
#target_link_libraries(opencv_cam  ${OpenCV_LIBS})


# testing executables, disable them for release version
#rosbuild_add_executable(cameraTest test/cameraTest.cpp)
#rosbuild_add_executable(startClient test/startClient.cpp)
#rosbuild_add_executable(stopClient test/stopClient.cpp)

#rosbuild_add_executable(reconInfo node_info/ReconInfo.cpp)
#rosbuild_add_executable(reconReceive node_info/ReconReceive.cpp)

#retail result
rosbuild_add_executable(retailResult node_info/RetailResultParser.cpp)

#gui
#rosbuild_add_executable(Gui node_object/gui2.cpp)
rosbuild_add_executable(Gui node_gui/gui4.cpp )
#rosbuild_add_executable(Gui node_gui/gui3.cpp )
target_link_libraries(
 Gui
  ${SDL_LIBRARY}
  ${OPENAL_LIBRARY}
  ${OPENGL_LIBRARIES}
  ${OpenCV_LIBRARIES}
  ${EXTRA_LIBRARY}/libGLEW.a
#  ${GLEW_LIBRARY}
)

#face stuff
#rosbuild_add_executable(FaceReco node_face/ros_main.cpp)

rosbuild_add_executable(faceDetect node_face/face_detect_main.cpp)
target_link_libraries(faceDetect ${Boost_LIBRARIES} ${OpenCV_LIBRARIES} )

rosbuild_add_executable(faceRecognize node_face/face_recognize_main.cpp)
target_link_libraries(faceRecognize ${Boost_LIBRARIES} ${OpenCV_LIBRARIES} )

rosbuild_add_executable(faceTrain node_face/face_train_main.cpp)
target_link_libraries(faceTrain ${Boost_LIBRARIES} ${OpenCV_LIBRARIES} )

#location stuff
rosbuild_add_executable(location node_location/location_main.cpp)
target_link_libraries(location ${Boost_LIBRARIES} ${OpenCV_LIBRARIES} )

#bnet stuff
#rosbuild_add_executable(bayesNet node_dm_bnet/bnet_main.cpp)
rosbuild_add_executable(bayesNet node_bnet/main_new.cpp)
target_link_libraries(bayesNet ${Boost_LIBRARIES} ${EXTRA_LIBRARY}/libsmile.a)
#ADD_DEFINITIONS("-std=c++0x")

#logger
rosbuild_add_executable(logger node_logger/main.cpp)
target_link_libraries(logger ${Boost_LIBRARIES} )

#dummy tester
rosbuild_add_executable(dummyTester node_dummy_tester/dummy_test.cpp)

#cloud service
rosbuild_add_executable(cloudService node_backend_ros/backend_ros.cpp)

