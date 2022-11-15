#pragma once

struct ConstantsUBO
{
	// ( t / 10, t, sin(t), dt )
	glm::vec4 timeParams;
	glm::vec4 screenParams;

	glm::vec4 bias_ambient;

	glm::mat4 world_to_light;
	glm::mat4 light_to_world;
};

struct ViewUBO
{
	glm::mat4 view_matrix;
	glm::mat4 persp_matrix;

	glm::mat4 view_persp_matrix;

	glm::vec4 cameraPosition;
};

struct TransformPushConstant
{
	glm::mat4 model_matrix;
};
