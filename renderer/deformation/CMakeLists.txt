# The sample code to compute entropy for a vector field
cmake_minimum_required (VERSION 2.8)
project (deformation) 

find_package(CUDA)
find_package(CGAL)
find_package(BOOST)
find_package(GLEW)

# DEL-BY-LEETEN 08/17/2011-BEGIN
	# # set paths to OSUFlow-related libraries
	# set(OSUFLOW_DIR 	"${CMAKE_SOURCE_DIR}" 		) # CACHE	PATH "The path to OSUFlow.")
	# set(RENDERER_DIR	"${OSUFLOW_DIR}/renderer" 	) # CACHE	PATH "The path to the renderer of OSUFlow.")
	# set(GCB_DIR 		"${RENDERER_DIR}/libgcb" 	) # CACHE	PATH "The path to gcb (a GLUT-related mini window managemenet library).")
	# if(NOT DEFINED WIN32 AND NOT DEFINED MAC_OSX_OMPI AND NOT DEFINED MAC_OSX_MPICH)
	#	add_definitions(
	#		-DLINUX
	#	)
	# endif()
# DEL-BY-LEETEN 08/17/2011-END

# specify the path to the headers
include_directories(
		. 
		# DEL-BY-LEETEN 08/17/2011		${OSUFLOW_DIR}/src
		#${RENDERER_DIR}
		${GCB_DIR}
		#${GLUT_INCLUDE_PATH}
	)
	
if ( CGAL_FOUND )
#	message(${CGAL_USE_FILE})
	include( ${CGAL_USE_FILE} )
else()
    message(STATUS "This program requires the CGAL library, and will not be compiled.")
endif()

# specify the path to the libraries
link_directories(
	# DEL-BY-LEETEN 08/17/2011		${OSUFLOW_DIR}/src
	${RENDERER_DIR}
	${GCB_DIR}
	${CUDA_TOOLKIT_INCLUDE}
	${CGAL_INCLUDE_DIR}
	${Boost_LIBRARY_DIRS}
)
# build the project as a library
cuda_add_executable(gcbDeformation 
	gcbDeformation.cpp
	LineRendererInOpenGLDeform.h
	LineRendererInOpenGLDeform.cpp
	StreamDeform.h
	StreamDeform.cpp
	shaderprogram.h
	shaderprogram.cpp
	PickPanel.h
	PickPanel.cpp
	PickWin.h
	PickWin.cpp
	bundle.h
	bundle.cpp
	cluster.h
	cluster.cpp
	TimeLineRendererInOpenGLDeform.h
	TimeLineRendererInOpenGLDeform.cpp
	gcb2.h
	gcb2.cpp
	DeformGPU.h
	DeformGPU.cu
	ColorPalette.h
	modes.h
	cutil_math.h
	Ellipse.h
	TransformMgr.h
	)

target_link_libraries(gcbDeformation 
		renderer # ADD-BY-LEETEN 10/22/2012
		OSUFlow
		# DEL-BY-LEETEN 10/22/2012: renderer
		gcb
		glew32
		${OPENGL_LIBRARY} 
		${GLU_LIBRARY} 
		${GLUT_LIBRARY}
		${CUDA_CUDART_LIBRARY}
		${Boost_SYSTEM_LIBRARY}
		${Boost_THREAD_LIBRARY}
)
