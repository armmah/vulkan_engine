#version 450

layout (location = 0) out vec2 outUV;

layout(set = 0, binding = 0) uniform ConstantsBlockUBO
{
	// ( t / 10, t, sin(t), dt )
	vec4 timeParams;
	vec4 screenParams;

	vec4 normalizedLightDirection;
} constUBO;

layout(set = 1, binding = 0) uniform ViewBlockUBO
{
	mat4 view_matrix;
	mat4 persp_matrix;
	mat4 view_persp_matrix;

	vec4 cameraPosition;
} viewUBO;

layout( push_constant ) uniform constants
{
	mat4 model_matrix;
} pushConstants;

void main() 
{
	outUV = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
	gl_Position = vec4(outUV * 2.0f - 1.0f, 0.0f, 1.0f);
}
