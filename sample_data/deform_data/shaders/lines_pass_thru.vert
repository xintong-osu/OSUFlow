#version 330
in vec4 vertex;
in vec3 tangent;
in vec4 vertexObjectColor;

uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;

uniform vec4 line_color;
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

	vec4 ks = vec4(0.3, 0.3, 0.3, 1.0);
	vec4 kd = vec4(0.3f, 0.3f, 0.3f, 1.0f);
	vec4 ka = vec4(0.5, 0.5, 0.5, 1.0);
	vec4 cc;
	if(line_color.a > 0.6 )
		cc = vertexObjectColor;
	else 
		cc = vec4(line_color.x, line_color.y, line_color.z, 1.0);

	VetexColor =  ka * cc
				+ kd*diffuseIntensity
				+ ks*(specularItensity);
//	VetexColor = line_color;
	gl_Position = vertex;//projectionMatrix * modelViewMatrix * vertex;
	gl_Position = gl_Position / gl_Position.w;
}
