#version 450

layout(location = 0) in vec3 inPosition;

layout(set = 0, binding = 0) uniform ConstantsBlockUBO
{
	// ( t / 10, t, sin(t), dt )
	vec4 timeParams;
	vec4 screenParams;

	vec4 normalizedLightDirection;

	mat4 world_to_light;
	mat4 light_to_world;
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
    mat4 render_matrix = viewUBO.view_persp_matrix * pushConstants.model_matrix;
    gl_Position = render_matrix * vec4(inPosition.xyz, 1.0);
}
