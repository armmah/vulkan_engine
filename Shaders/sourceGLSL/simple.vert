#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inColor;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out vec3 viewDirection;

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
    mat4 render_matrix = viewUBO.view_persp_matrix * pushConstants.model_matrix;
    gl_Position = render_matrix * vec4(inPosition.xyz, 1.0);

    viewDirection = normalize(vec3(0.0, -0.75, -0.25));

    fragColor = inColor;

    fragTexCoord = uv;
    fragTexCoord.y = 1.0 - fragTexCoord.y;

    mat3 normalMatrix = mat3(transpose(inverse(pushConstants.model_matrix)));
    fragNormal = normalMatrix * inNormal;
}
