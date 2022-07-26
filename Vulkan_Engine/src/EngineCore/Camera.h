#pragma once
#include "pch.h"

const float YAW = 90.0f;
const float PITCH = 0.0f;
const float SPEED = 0.00025f;
const float SENSITIVITY = 50.0f;
const float ZOOM = 45.0f;

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
	const glm::mat4& getViewMatrix() const;
	const glm::mat4& getPerspectiveMatrix() const;

	void enqueueMouseMovement(int x, int y);
	void enqueueMouseScroll(int extent);
	void enqueueMovement(glm::vec3 direction);

	void processFrameEvents(float dt);

private:
	float fov_radians;
	float aspectRatio;
	float nearZ;
	float farZ;

	// Camera Attributes
	glm::vec3 pos;
	glm::quat rot;

	glm::vec3 movement;

	// Euler Angles
	float yaw;
	float pitch;

	// Camera options
	float movementSpeed;
	float mouseSensitivity;
	float zoom;

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
