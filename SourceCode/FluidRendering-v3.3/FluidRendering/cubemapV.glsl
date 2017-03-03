#version 430

layout (location = 0) in vec3 Position;

out vec3 TexCoords;

uniform mat4 uWVPTransformMat;

void main()
{
    vec4 Pos = uWVPTransformMat * vec4(Position, 1.0);
	gl_Position = Pos.xyww;
    TexCoords = Position;
}  