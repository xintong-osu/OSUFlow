INCLUDE_DIRECTORIES ( . )

add_library(myproject_juce 
	./modules/juce_core/juce_core.cpp
	./modules/juce_data_structures/juce_data_structures.cpp
	./modules/juce_events/juce_events.cpp
	./modules/juce_graphics/juce_graphics.cpp
	./modules/juce_gui_basics/juce_gui_basics.cpp
	./modules/juce_gui_extra/juce_gui_extra.cpp
	./modules/juce_opengl/juce_opengl.cpp
	)

TARGET_LINK_LIBRARIES ( myproject_juce )
	
# ADD-BY-LEETEN 09/01/2011-BEGIN
set_target_properties(${PROJECT_NAME} PROPERTIES 
	DEBUG_OUTPUT_NAME "${PROJECT_NAME}d"
	RELEASE_OUTPUT_NAME "${PROJECT_NAME}"
	)