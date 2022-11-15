#include "pch.h"
#include "Camera.h"

Camera::Camera(VkExtent2D windowSize, float nearZ, float farZ) :
	fov_radians(glm::radians(0.f)),
	nearZ(nearZ), farZ(farZ),
	pos(0.f),
	yaw(YAW), pitch(PITCH), movement(), zoom(ZOOM), movementSpeed(SPEED), mouseSensitivity(SENSITIVITY)
{
	updateWindowExtent(windowSize);
	processFrameEvents(0.f);
}

Camera::Camera(float fov_degrees, VkExtent2D windowSize, float nearZ, float farZ) :
	fov_radians(glm::radians(fov_degrees)),
	nearZ(nearZ), farZ(farZ),
	pos(0.f), //rot(glm::vec3(0.f, 0.f, 0.f)),
	yaw(YAW), pitch(PITCH), movement(), zoom(ZOOM), movementSpeed(SPEED), mouseSensitivity(SENSITIVITY)
{
	updateWindowExtent(windowSize);
	processFrameEvents(0.f);
}

const glm::vec3& Camera::getPosition() const { return pos; }
//const glm::quat& Camera::getRotation() const { return rot; }
float Camera::getYaw() const { return yaw; }
float Camera::getPitch() const { return pitch; }

void Camera::setPosition(const glm::vec3& position) { pos = position; }
//void Camera::setRotation(const glm::quat& rotation) { rot = rotation; calculateViewMatrix(); }
void Camera::setRotation(float y, float p) { yaw = y; pitch = p; }

void Camera::lookAt(glm::vec3 point)
{
	auto front = point - pos;
	auto right = glm::vec3(1.f, 0.f, 0.f);

	auto up = glm::normalize(glm::cross(right, front));
	cachedViewMatrix = glm::lookAt(pos, point, up);

	cachedViewProjectionMatrix = calculateViewProjectionMatrix();
}

void Camera::setNearFarZ(float near, float far)
{
	nearZ = near;
	farZ = far;
}

void Camera::setFieldOfView(float fov_degrees)
{
	auto newFoV = glm::radians(fov_degrees);
	if (newFoV == fov_radians)
		return;

	fov_radians = newFoV;
	calculateProjectionMatrix();
}

void Camera::updateWindowExtent(VkExtent2D newExtent)
{
	if (newExtent.width == windowExtent.width &&
		newExtent.height == windowExtent.height)
		return;

	windowExtent = newExtent;

	aspectRatio = windowExtent.width / static_cast<float>(windowExtent.height);
	calculateProjectionMatrix();
}

const glm::mat4& Camera::getViewProjectionMatrix() const { return cachedViewProjectionMatrix; }
const glm::mat4& Camera::getViewMatrix() const { return cachedViewMatrix; }
const glm::mat4& Camera::getPerspectiveMatrix() const { return cachedProjectionMatrix; }

void Camera::enqueueMouseMovement(int x, int y)
{
	yaw += (x / static_cast<float>(windowExtent.width)) * mouseSensitivity;
	pitch += (y / static_cast<float>(windowExtent.height)) * mouseSensitivity;
	pitch = std::clamp(pitch, -89.f, 89.f);
}

void Camera::enqueueMouseScroll(int extent) { zoom = std::clamp((float)extent, 1.f, 45.f); }
void Camera::enqueueMovement(glm::vec3 direction) 
{
	movement += direction;
}

glm::vec3 getFrontVector(float pitchRad, float yawRad)
{
	return glm::normalize(
		glm::vec3(
			cos(yawRad) * cos(pitchRad),
			sin(pitchRad),
			sin(yawRad) * cos(pitchRad)
		)
	);
}

glm::mat4 _centerAround(glm::vec3 center, float pitch, float yaw, float distance)
{
	auto front = getFrontVector(glm::radians(pitch), glm::radians(yaw));
	auto pos = center + front * distance;
	return glm::lookAt(pos, center, glm::vec3(0.f, 1.f, 0.f));
}

void Camera::centerAround(float pitch, float yaw, float distance)
{
	cachedViewMatrix = _centerAround(glm::vec3(0.f), pitch, yaw, distance);
	cachedViewProjectionMatrix = calculateViewProjectionMatrix();
}

void Camera::processFrameEvents(float dt)
{
	// calculate the new Front vector
	auto front = getFrontVector(glm::radians(pitch), glm::radians(yaw));

	// also re-calculate the Right and Up vector
	// normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
	auto right = glm::normalize(glm::cross(front, glm::vec3(0.f, 1.f, 0.f)));
	auto up = glm::normalize(glm::cross(right, front));

	// Redo with sanitized inputs
	if (movement.x != 0.0)
		pos += right * -(static_cast<float>(std::signbit(movement.x)) * 2.f - 1.f) * movementSpeed * dt;
	if (movement.y != 0.0)
		pos += up * (static_cast<float>(std::signbit(movement.y)) * 2.f - 1.f) * movementSpeed * dt;
	if (movement.z != 0.0)
		pos += front * (static_cast<float>(std::signbit(movement.z)) * 2.f - 1.f) * movementSpeed * dt;

	movement = glm::vec3();

	cachedViewMatrix = glm::lookAt(pos, pos + front, up);
	cachedViewProjectionMatrix = calculateViewProjectionMatrix();
}

/*
glm::mat4 Camera::calculateViewMatrix()
{
	const glm::mat4 rotationMatrix = glm::mat4_cast(rot);
	const glm::mat4 translationMatrix = glm::translate(glm::mat4(1.f), pos);

	cachedViewMatrix = translationMatrix * rotationMatrix;
	cachedViewProjectionMatrix = calculateViewProjectionMatrix();
	return cachedViewMatrix;
}
*/
glm::mat4 Camera::calculateProjectionMatrix()
{
	if (fov_radians == 0.f)
	{
		auto w = static_cast<float>(windowExtent.width);
		auto h = static_cast<float>(windowExtent.height);
		cachedProjectionMatrix = glm::ortho(w * -0.5f, w * 0.5f, h * -0.5f, h * 0.5f, nearZ, farZ);
	}
	else
	{
		cachedProjectionMatrix = glm::perspective(fov_radians, aspectRatio, nearZ, farZ);
	}
	cachedProjectionMatrix[1][1] *= -1;

	cachedViewProjectionMatrix = calculateViewProjectionMatrix();
	return cachedProjectionMatrix;
}

glm::mat4 Camera::calculateViewProjectionMatrix() { return cachedProjectionMatrix * cachedViewMatrix; }
