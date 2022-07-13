#pragma once
#include "pch.h"

class Camera
{
public:
	Camera(float fov_degrees, VkExtent2D windowSize, float nearZ = 1.0f, float farZ = 100.0f);

	const glm::vec3& getPosition() const;
	const glm::quat& getRotation() const;

	void setPosition(const glm::vec3& position);
	void setRotation(const glm::quat& rotation);
	void setRotation(const glm::vec3& rotEuler);

	const VkViewport& getViewport() const;
	const VkRect2D& getScissorRect() const;

	void setNearFarZ(float near, float far);
	void setFieldOfView(float fov_degrees);
	void updateWindowExtent(VkExtent2D newExtent);
	const glm::mat4& getViewProjectionMatrix() const;

private:
	float fov_radians;
	float aspectRatio;
	float nearZ;
	float farZ;

	glm::vec3 pos;
	glm::quat rot;

	VkExtent2D windowExtent;
	VkViewport viewport;
	VkRect2D scissor;

	glm::mat4 cachedViewMatrix;
	glm::mat4 cachedProjectionMatrix;
	glm::mat4 cachedViewProjectionMatrix;

	glm::mat4 calculateViewMatrix();
	glm::mat4 calculateProjectionMatrix();
	glm::mat4 calculateViewProjectionMatrix();
};
