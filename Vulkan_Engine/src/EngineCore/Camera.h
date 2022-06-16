#pragma once
#include "pch.h"
#include "VkTypes/InitializersUtility.h"

class Camera
{
public:
	Camera(float fov_degrees, VkExtent2D windowSize, float nearZ = 1.0f, float farZ = 100.0f) :
		fov_radians(glm::radians(fov_degrees)),
		nearZ(nearZ), farZ(farZ),
		pos(0.f), rot(glm::vec3(0.f, 0.f, 0.f))
	{
		calculateViewMatrix();
		updateWindowExtent(windowSize);
	}

	const glm::vec3& getPosition() const { return pos; }
	const glm::quat& getRotation() const { return rot; }

	void setPosition(const glm::vec3& position) { pos = position; calculateViewMatrix(); }
	void setRotation(const glm::quat& rotation) { rot = rotation; calculateViewMatrix(); }
	void setRotation(const glm::vec3& rotEuler) { rot = glm::quat(rotEuler); calculateViewMatrix(); }

	const VkViewport& getViewport() const { return viewport; }
	const VkRect2D& getScissorRect() const { return scissor; }

	void setNearFarZ(float near, float far)
	{
		nearZ = near;
		farZ = far;
	}

	void setFieldOfView(float fov_degrees)
	{
		auto newFoV = glm::radians(fov_degrees);
		if (newFoV == fov_radians)
			return;

		fov_radians = newFoV;
		calculateProjectionMatrix();
	}

	void updateWindowExtent(VkExtent2D newExtent)
	{
		if (newExtent.width == windowExtent.width &&
			newExtent.height == windowExtent.height)
			return;

		windowExtent = newExtent;

		aspectRatio = windowExtent.width / static_cast<float>(windowExtent.height);
		calculateProjectionMatrix();

		vkinit::Commands::initViewportAndScissor(viewport, scissor, newExtent);
	}

	const glm::mat4& getViewProjectionMatrix() const { return cachedViewProjectionMatrix; }

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

	glm::mat4 calculateViewMatrix()
	{
		const glm::mat4 rotationMatrix = glm::mat4_cast(rot);
		const glm::mat4 translationMatrix = glm::translate(glm::mat4(1.f), pos);

		cachedViewMatrix = translationMatrix * rotationMatrix;
		cachedViewProjectionMatrix = calculateViewProjectionMatrix();
		return cachedViewMatrix;
	}

	glm::mat4 calculateProjectionMatrix()
	{
		cachedProjectionMatrix = glm::perspective(fov_radians, aspectRatio, nearZ, farZ);
		cachedViewProjectionMatrix = calculateViewProjectionMatrix();
		return cachedProjectionMatrix;
	}

	glm::mat4 calculateViewProjectionMatrix() { return cachedProjectionMatrix * cachedViewMatrix; }
};
