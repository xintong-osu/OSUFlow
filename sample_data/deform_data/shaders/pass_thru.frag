#version 330

in vec4 VetexColor;
out vec4 FragColor;


void main( )
{
	if(0 == VetexColor.a)
		discard;
    FragColor = VetexColor;
}
