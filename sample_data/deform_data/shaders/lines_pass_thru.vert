#version 330
in vec4 vertex;
in vec3 tangent;
in vec4 vertexObjectColor;
in int translucent;

uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;

uniform vec4 line_color;
uniform int mode;

out vec4 VetexColor;


void main()
{
	vec3 lightDirection = vec3(0.0f, 1.0f, 0.0f);
	float diffuseIntensity;
	float specularItensity;

	vec3 light;

	vec3 normal;
	vec3 eye;
	
	vec3 reflection;

	light = normalize(lightDirection);

	normal = normalize(tangent);
	eye = normalize(vec3(0.0f, 1.0f, 0.0f));

	diffuseIntensity = clamp(max(dot(normal, light), 0.0), 0.0, 1.0);
	
	reflection = normalize(reflect(-light, normal));
	specularItensity = pow(clamp(max(dot(reflection, eye), 0.0), 0.0, 1.0), 2.0 );

	vec4 ks = vec4(0.25, 0.25, 0.25, 1.0);
	vec4 kd = vec4(0.25f, 0.25f, 0.25f, 1.0f);
	vec4 ka = vec4(0.7, 0.7, 0.7, 1.0);
	vec4 cc;
	const float opacity = 0.2;
	if(0 == mode || 2 == mode || 4 == mode)
	{
		cc = vertexObjectColor;
		VetexColor =  ka * cc
				+ kd*diffuseIntensity
				+ ks*(specularItensity);
	//	VetexColor.a = 0.3;
		if(2 == mode && 0 != translucent)
			VetexColor.a = 0;
		else if(4 == mode)
		{
			if(0 != translucent)
				VetexColor.a = opacity;
			else
				VetexColor.a = 0;
		}
	}
	else if(1 == mode || 3 == mode || 5 == mode)
	{
		VetexColor = vec4(line_color.x, line_color.y, line_color.z, 0.5);
		if(3 == mode && 0 != translucent)
			VetexColor.a = 0;
		else if(5 == mode)
		{
			if(0 != translucent)
				VetexColor.a = opacity;
			else
				VetexColor.a = 0;
		}
	}
	
	
//		VetexColor = vec4(0.01,0.01,0.01,0.3);
		

	
	//VetexColor.a = 1.0;
//	VetexColor = line_color;
	gl_Position = vertex;//projectionMatrix * modelViewMatrix * vertex;
	gl_Position = gl_Position / gl_Position.w;
}
