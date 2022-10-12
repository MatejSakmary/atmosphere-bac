#include "camera.hpp"
// #define GLM_FORCE_DEPTH_ZERO_TO_ONE
// #define GLM_FORCE_LEFT_HANDED

Camera::Camera(glm::vec3 position, glm::vec3 direction, glm::vec3 up) {
	this->m_position = position;
	this->m_front = direction;
	this->m_up = up;
	this->m_speed = 50.0f;
	this->yaw = -90.0f;
	this->pitch = 0.0f;
	this->sensitivity = 0.08f;
}

glm::mat4 Camera::getViewMatrix(bool LH) {
	glm::vec3 direction;

    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.z = sin(glm::radians(pitch));
    direction.y = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

	m_front = glm::normalize(direction);

	if(LH)
	{
		return glm::lookAtLH(m_position, m_position + m_front, m_up);
	}else{
		return glm::lookAt(m_position, m_position + m_front, m_up);
	}
}
    
void Camera::forward(float deltaTime) {
	this->m_position += m_front * m_speed * deltaTime;
}

void Camera::back(float deltaTime) {
	this->m_position -= m_front * m_speed * deltaTime;
}

void Camera::left(float deltaTime) {
	this->m_position -= glm::normalize(glm::cross(m_front, m_up)) * m_speed * deltaTime;
}

void Camera::right(float deltaTime) {
	this->m_position += glm::normalize(glm::cross(m_front, m_up)) * m_speed * deltaTime;
}

void Camera::updateFrontVec(float xoffset, float yoffset) {
	yaw += sensitivity * xoffset;
	pitch += sensitivity * yoffset;
	// make sure the camera is not flipping, lock the top and bottom view angle so that
	// we can look up to the sky or to the ground but not further
	if (pitch > 89.0f) pitch = 89.0f;
	if (pitch < -89.0f) pitch = -89.0f;
}

void Camera::up(float deltaTime)
{
	this->m_position += m_up * m_speed * deltaTime;
}

void Camera::down(float deltaTime)
{
	this->m_position -= m_up * m_speed * deltaTime;
}

glm::vec3 Camera::getPos() {
	return m_position;
}
float  Camera::getPitch() {
	return pitch;
}
float Camera::getYaw() {
	return yaw;
}

glm::vec3 Camera::getFront() {
    return this->m_front;
}