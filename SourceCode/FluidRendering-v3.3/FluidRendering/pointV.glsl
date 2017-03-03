#version 430

layout (location = 0) in vec3 Position;

uniform mat4 uWVPTransform;
uniform mat4 uWVTransform;

uniform float uPointRadius;  // point size in world space
uniform float uPointScale;	// scale to calculate size in pixels

out vec3 vsWorldPos;

void main()
{
	vec4 Pos = uWVPTransform * vec4(Position, 1.0);
	vec4 ViewPos = uWVTransform * vec4(Position, 1.0);
	gl_PointSize = 45;
//	gl_PointSize = -uPointScale * (uPointRadius / ViewPos.z);

	gl_Position = Pos;
	vsWorldPos = ViewPos.xyz;
}