# The sample code to compute entropy for a vector field
cmake_minimum_required (VERSION 2.8)
project (gesture) 

find_package(Leap)
find_package(CUDA)

include_directories(
	. 
	./LeapUtil
	${LEAP_INCLUDE_DIR}
	${CUDA_TOOLKIT_INCLUDE}
	#C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v6.0/include
	)

# specify the path to the libraries
link_directories(
)


# build the project as a library
add_executable(${PROJECT_NAME} WIN32
	Main.cpp
	DataMgr.h
	DataMgr.cpp
	StreamlineGL.h
	StreamlineGL.cpp
	GestureApp.h
	GestureApp.cpp
	LeapUtil/LeapScene.h
	LeapUtil/LeapScene.cpp
	LeapUtil/LeapUtil.h
	LeapUtil/LeapUtil.cpp
	LeapUtil/LeapUtilGL.h
	LeapUtil/LeapUtilGL.cpp
	)

ADD_SUBDIRECTORY (JuceLibraryCode)
	
target_link_libraries(
	${PROJECT_NAME} 
	${CMAKE_THREAD_LIBS_INIT}
	OSUFlow
	${LEAP_LIBRARY}
	myproject_juce
	)

