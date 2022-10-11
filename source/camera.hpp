#pragma once

#include <iostream>
#include "primitives.hpp"
//#include "glm/glm.hpp"
//#include "glm/gtc/matrix_transform.hpp"
//#include "glm/gtc/type_ptr.hpp"

class Camera
{
private:
	glm::vec3 m_position;
	glm::vec3 m_front;
	glm::vec3 m_up;
	float pitch;
	float yaw;
	float m_speed;
	float sensitivity;

public:
    /**
     * @param position initial position of camera
     * @param direction initial camera direction
     * @param m_up camera up vector
     * @param curve dynamic curve on which camera moves
     */
	Camera(glm::vec3 position, glm::vec3 direction, glm::vec3 m_up);
	/**
	 * Checks colliders
	 */
	void forward(float deltaTime);
	void back(float deltaTime);
	void left(float deltaTime);
	void right(float deltaTime);
	void up(float deltaTime);
	void down(float deltaTime);
	/**
	 * Update camera direction based on mouse offsets
	 * @param xoffset mouse X offset from last frame
	 * @param yoffset mouse Y offset from last frame
	 */
	void updateFrontVec(float xoffset, float yoffset);
	/**
	 * Force camera to glide along predefined curve
	 */
	glm::vec3 getPos();
    glm::vec3 getFront();
	float getYaw();
	float getPitch();
	glm::mat4 getViewMatrix(bool LH = false);
};
