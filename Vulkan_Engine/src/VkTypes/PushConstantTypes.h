#pragma once

struct TransformPushConstant
{
	glm::mat4 model_matrix;
	glm::mat4 view_matrix;
	glm::mat4 persp_matrix;
};
