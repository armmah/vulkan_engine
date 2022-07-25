#include "pch.h"
#include "VkTypes/InitializersUtility.h"
#include "Camera.h"

Camera::Camera(float fov_degrees, VkExtent2D windowSize, float nearZ, float farZ) :
	fov_radians(glm::radians(fov_degrees)),
	nearZ(nearZ), farZ(farZ),
	pos(0.f), rot(glm::vec3(0.f, 0.f, 0.f))
{
	calculateViewMatrix();
	updateWindowExtent(windowSize);
}

const glm::vec3& Camera::getPosition() const { return pos; }
const glm::quat& Camera::getRotation() const { return rot; }

void Camera::setPosition(const glm::vec3& position) { pos = position; calculateViewMatrix(); }
void Camera::setRotation(const glm::quat& rotation) { rot = rotation; calculateViewMatrix(); }
void Camera::setRotation(const glm::vec3& rotEuler) { rot = glm::quat(rotEuler); calculateViewMatrix(); }

const VkViewport& Camera::getViewport() const { return viewport; }
const VkRect2D& Camera::getScissorRect() const { return scissor; }

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

	vkinit::Commands::initViewportAndScissor(viewport, scissor, newExtent);
}

const glm::mat4& Camera::getViewProjectionMatrix() const { return cachedViewProjectionMatrix; }
const glm::mat4& Camera::getViewMatrix() const { return cachedViewMatrix; }
const glm::mat4& Camera::getPerspectiveMatrix() const { return cachedProjectionMatrix; }

glm::mat4 Camera::calculateViewMatrix()
{
	const glm::mat4 rotationMatrix = glm::mat4_cast(rot);
	const glm::mat4 translationMatrix = glm::translate(glm::mat4(1.f), pos);

	cachedViewMatrix = translationMatrix * rotationMatrix;
	cachedViewProjectionMatrix = calculateViewProjectionMatrix();
	return cachedViewMatrix;
}

glm::mat4 Camera::calculateProjectionMatrix()
{
	cachedProjectionMatrix = glm::perspective(fov_radians, aspectRatio, nearZ, farZ);
	cachedViewProjectionMatrix = calculateViewProjectionMatrix();
	return cachedProjectionMatrix;
}

glm::mat4 Camera::calculateViewProjectionMatrix() { return cachedProjectionMatrix * cachedViewMatrix; }
