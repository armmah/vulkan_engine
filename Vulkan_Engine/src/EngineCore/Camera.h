#pragma once
#include "pch.h"

static constexpr float YAW = 90.0f;
static constexpr float PITCH = 0.0f;
static constexpr float SPEED = 0.125f;
static constexpr float SENSITIVITY = 150.0f;
static constexpr float ZOOM = 45.0f;

class Camera
{
public:
	Camera(float fov_degrees, VkExtent2D windowSize, float nearZ = 1.0f, float farZ = 100.0f);
	Camera(VkExtent2D windowSize, float nearZ = 1.0f, float farZ = 100.0f);

	const glm::vec3& getPosition() const;
	// const glm::quat& getRotation() const;
	float getYaw() const;
	float getPitch() const;
	float getSpeedMultiplier() const { return movementSpeed / SPEED; }
	void setSpeedMultiplier(float multiplier) { movementSpeed = SPEED * multiplier; }
	
	void setPosition(const glm::vec3& position);
	void setRotation(float yaw, float pitch);
	void lookAt(glm::vec3 point);
	void centerAround(float pitch, float yaw, float distance);

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
	// glm::quat rot;

	glm::vec3 movement;

	// Euler Angles
	float yaw;
	float pitch;

	// Camera options
	float movementSpeed;
	float mouseSensitivity;
	float zoom;

	VkExtent2D windowExtent;

	glm::mat4 cachedViewMatrix;
	glm::mat4 cachedProjectionMatrix;
	glm::mat4 cachedViewProjectionMatrix;

	// glm::mat4 calculateViewMatrix();
	glm::mat4 calculateProjectionMatrix();
	glm::mat4 calculateViewProjectionMatrix();
};
