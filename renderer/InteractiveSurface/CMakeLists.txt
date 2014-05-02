# The sample code to compute entropy for a vector field
cmake_minimum_required (VERSION 2.8)
project (gcbInteractiveSurface) 

# specify the path to the headers
include_directories(
		. 
		${GCB_DIR}
		#${GLUT_INCLUDE_PATH}
	)

# specify the path to the libraries
link_directories(
	# DEL-BY-LEETEN 08/17/2011		${OSUFLOW_DIR}/src
	${RENDERER_DIR}
	${GCB_DIR}
)

# build the project as a library
add_executable(	${PROJECT_NAME} 
				${PROJECT_NAME}.cpp 
				SurfaceRenderer.h
				SurfaceRenderer.cpp 
				SurfaceRendererBasic.h
				SurfaceRendererBasic.cpp
				)

target_link_libraries(${PROJECT_NAME} 
		renderer # ADD-BY-LEETEN 10/22/2012
		OSUFlow
		# DEL-BY-LEETEN 10/22/2012: renderer
		gcb
)

# ADD-BY-JIMMY 5/10/2013-BEGIN
target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARY} ${GLU_LIBRARY} ${GLUT_LIBRARY})

