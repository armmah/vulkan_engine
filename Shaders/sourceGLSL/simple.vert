#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inColor;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out vec3 viewDirection;

//push constants block
layout( push_constant ) uniform constants
{
	mat4 model_matrix;
	mat4 view_matrix;
	mat4 persp_matrix;
} PushConstants;

void main()
{
    mat4 render_matrix = PushConstants.persp_matrix * PushConstants.view_matrix * PushConstants.model_matrix;
    gl_Position = render_matrix * vec4(inPosition.xyz, 1.0);

    viewDirection = normalize(vec3(0.0, -0.75, -0.25));

    fragColor = inColor;

    fragTexCoord = uv;

    mat3 normalMatrix = mat3(transpose(inverse(PushConstants.model_matrix)));
    fragNormal = normalMatrix * inNormal;
}
