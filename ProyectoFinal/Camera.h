#pragma once

// Std. Includes
#include <vector>

// GL Includes
#define GLEW_STATIC
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement
{
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

// Default camera values
const GLfloat YAW = -90.0f;
const GLfloat PITCH = 0.0f;
const GLfloat SPEED = 6.0f;
const GLfloat SENSITIVTY = 0.25f;
const GLfloat ZOOM = 45.0f;

// An abstract camera class that processes input and calculates the corresponding Eular Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:
	// Constructor with vectors
	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), GLfloat yaw = YAW, GLfloat pitch = PITCH) : front(glm::vec3(0.0f, 0.0f, -1.0f)), movementSpeed(SPEED), mouseSensitivity(SENSITIVTY), zoom(ZOOM)
	{
		this->position = position;
		this->worldUp = up;
		this->yaw = yaw;
		this->pitch = pitch;
		this->updateCameraVectors();
	}

	// Constructor with scalar values
	Camera(GLfloat posX, GLfloat posY, GLfloat posZ, GLfloat upX, GLfloat upY, GLfloat upZ, GLfloat yaw, GLfloat pitch) : front(glm::vec3(0.0f, 0.0f, -1.0f)), movementSpeed(SPEED), mouseSensitivity(SENSITIVTY), zoom(ZOOM)
	{
		this->position = glm::vec3(posX, posY, posZ);
		this->worldUp = glm::vec3(upX, upY, upZ);
		this->yaw = yaw;
		this->pitch = pitch;
		this->updateCameraVectors();
	}

	// Returns the view matrix calculated using Eular Angles and the LookAt Matrix
	glm::mat4 GetViewMatrix()
	{
		return glm::lookAt(this->position, this->position + this->front, this->up);
	}

	// Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
	void ProcessKeyboard(Camera_Movement direction, GLfloat deltaTime)
	{
		GLfloat velocity = this->movementSpeed * deltaTime;

		// se guarda la posición actual en una variable temporal
		glm::vec3 newPosition = this->position;

		// calculo de hacia dónde nos queremos mover
		if (direction == FORWARD)
			newPosition += this->front * velocity;
		if (direction == BACKWARD)
			newPosition -= this->front * velocity;
		if (direction == LEFT)
			newPosition -= this->right * velocity;
		if (direction == RIGHT)
			newPosition += this->right * velocity;

		// anclar la altura a la posición original, para no volar ni unirse al piso al chocar con algo
		newPosition.y = this->position.y;

		// verificar si la nueva posición es válida, no choque
		if (!CheckCollision(newPosition))
		{
			// si no choca actualizamos la cámara
			this->position = newPosition;
		}
	}

	// Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
	void ProcessMouseMovement(GLfloat xOffset, GLfloat yOffset, GLboolean constrainPitch = true)
	{
		xOffset *= this->mouseSensitivity;
		yOffset *= this->mouseSensitivity;

		this->yaw += xOffset;
		this->pitch += yOffset;

		// Make sure that when pitch is out of bounds, screen doesn't get flipped
		if (constrainPitch)
		{
			if (this->pitch > 89.0f)
			{
				this->pitch = 89.0f;
			}

			if (this->pitch < -89.0f)
			{
				this->pitch = -89.0f;
			}
		}

		// Update Front, Right and Up Vectors using the updated Eular angles
		this->updateCameraVectors();
	}

	// Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
	void ProcessMouseScroll(GLfloat yOffset)
	{

	}

	GLfloat GetZoom()
	{
		return this->zoom;
	}

	glm::vec3 GetPosition()
	{
		return this->position;
	}

	glm::vec3 GetFront()
	{
		return this->front;
	}

private:
	// Camera Attributes
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 worldUp;

	// Eular Angles
	GLfloat yaw;
	GLfloat pitch;

	// Camera options
	GLfloat movementSpeed;
	GLfloat mouseSensitivity;
	GLfloat zoom;

	// Calculates the front vector from the Camera's (updated) Eular Angles
	void updateCameraVectors()
	{
		// Calculate the new Front vector
		glm::vec3 front;
		front.x = cos(glm::radians(this->yaw)) * cos(glm::radians(this->pitch));
		front.y = sin(glm::radians(this->pitch));
		front.z = sin(glm::radians(this->yaw)) * cos(glm::radians(this->pitch));
		this->front = glm::normalize(front);
		// Also re-calculate the Right and Up vector
		this->right = glm::normalize(glm::cross(this->front, this->worldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		this->up = glm::normalize(glm::cross(this->right, this->front));
	}

	// --- FUNCIÓN DE COLISIONES AABB ---
	bool CheckCollision(glm::vec3 targetPos)
	{
		// buffer simula el "ancho" del cuerpo de la cámara para no pegarse mucho al modelo
		float buffer = 0.7f;
		//--- COLUMNA 1 ---
		// Centro geométrico tomado de Blender
		float centroX_col1 = -10.875f;
		float centroZ_col1 = -11.749f;
		float radio = 0.25f;
		float col1_minX = centroX_col1 - radio;
		float col1_maxX = centroX_col1 + radio;
		float col1_minZ = centroZ_col1 - radio;
		float col1_maxZ = centroZ_col1 + radio;

		if (targetPos.x > (col1_minX - buffer) && targetPos.x < (col1_maxX + buffer) &&
			targetPos.z >(col1_minZ - buffer) && targetPos.z < (col1_maxZ + buffer))
		{
			return true; 
		}

		//--- COLUMNA 2 ---
		float centroX_col2 = -0.774f;
		float centroZ_col2 = -11.749f;
		float col2_minX = centroX_col2 - radio;
		float col2_maxX = centroX_col2 + radio;
		float col2_minZ = centroZ_col2 - radio;
		float col2_maxZ = centroZ_col2 + radio;
		if (targetPos.x > (col2_minX - buffer) && targetPos.x < (col2_maxX + buffer) &&
			targetPos.z >(col2_minZ - buffer) && targetPos.z < (col2_maxZ + buffer))
		{
			return true;
		}


		//--- COLUMNA 3 ---
		float centroX_col3 = -10.875f;
		float centroZ_col3 = -21.749f;
		float col3_minX = centroX_col3 - radio;
		float col3_maxX = centroX_col3 + radio;
		float col3_minZ = centroZ_col3 - radio;
		float col3_maxZ = centroZ_col3 + radio;

		if (targetPos.x > (col3_minX - buffer) && targetPos.x < (col3_maxX + buffer) &&
			targetPos.z >(col3_minZ - buffer) && targetPos.z < (col3_maxZ + buffer))
		{
			return true;
		}

		//--- COLUMNA 4 ---
		float centroX_col4 = -0.774f;
		float centroZ_col4 = -21.749f;
		float col4_minX = centroX_col4 - radio;
		float col4_maxX = centroX_col4 + radio;
		float col4_minZ = centroZ_col4 - radio;
		float col4_maxZ = centroZ_col4 + radio;
		if (targetPos.x > (col4_minX - buffer) && targetPos.x < (col4_maxX + buffer) &&
			targetPos.z >(col4_minZ - buffer) && targetPos.z < (col4_maxZ + buffer))
		{
			return true;
		}


		//--- COLUMNA 5 ---
		float centroX_col5 = -10.875f;
		float centroZ_col5 = -33.749f;
		float col5_minX = centroX_col5 - radio;
		float col5_maxX = centroX_col5 + radio;
		float col5_minZ = centroZ_col5 - radio;
		float col5_maxZ = centroZ_col5 + radio;
		if (targetPos.x > (col5_minX - buffer) && targetPos.x < (col5_maxX + buffer) &&
			targetPos.z >(col5_minZ - buffer) && targetPos.z < (col5_maxZ + buffer))
		{
			return true;
		}


		//--- COLUMNA 6 ---
		float centroX_col6 = -0.774f;
		float centroZ_col6 = -33.749f;
		float col6_minX = centroX_col6 - radio;
		float col6_maxX = centroX_col6 + radio;
		float col6_minZ = centroZ_col6 - radio;
		float col6_maxZ = centroZ_col6 + radio;

		if (targetPos.x > (col6_minX - buffer) && targetPos.x < (col6_maxX + buffer) &&
			targetPos.z >(col6_minZ - buffer) && targetPos.z < (col6_maxZ + buffer))
		{
			return true;
		}


		//--- COLUMNA 7 ---
		float centroX_col7 = -10.875f;
		float centroZ_col7 = -43.749f;
		float col7_minX = centroX_col7 - radio;
		float col7_maxX = centroX_col7 + radio;
		float col7_minZ = centroZ_col7 - radio;
		float col7_maxZ = centroZ_col7 + radio;
		if (targetPos.x > (col7_minX - buffer) && targetPos.x < (col7_maxX + buffer) &&
			targetPos.z >(col7_minZ - buffer) && targetPos.z < (col7_maxZ + buffer))
		{
			return true;
		}

		//--- COLUMNA 8 ---
		float centroX_col8 = -0.774f;
		float centroZ_col8 = -43.749f;
		float col8_minX = centroX_col8 - radio;
		float col8_maxX = centroX_col8 + radio;
		float col8_minZ = centroZ_col8 - radio;
		float col8_maxZ = centroZ_col8 + radio;

		if (targetPos.x > (col8_minX - buffer) && targetPos.x < (col8_maxX + buffer) &&
			targetPos.z >(col8_minZ - buffer) && targetPos.z < (col8_maxZ + buffer))
		{
			return true;
		}

		//--- COLUMNA 9 ---
		float centroX_col9 = -22.875f;
		float centroZ_col9 = -33.749f;
		float col9_minX = centroX_col9 - radio;
		float col9_maxX = centroX_col9 + radio;
		float col9_minZ = centroZ_col9 - radio;
		float col9_maxZ = centroZ_col9 + radio;

		if (targetPos.x > (col9_minX - buffer) && targetPos.x < (col9_maxX + buffer) &&
			targetPos.z >(col9_minZ - buffer) && targetPos.z < (col9_maxZ + buffer))
		{
			return true;
		}

		//--- PAREDES GENERAL ---
		float piso_minX = -44.375f;
		float piso_maxX = 11.625f;
		float piso_minZ = -45.749f; 
		float piso_maxZ = -9.749f;   

		if (targetPos.x < (piso_minX + buffer) || targetPos.x >(piso_maxX - buffer) ||
			targetPos.z < (piso_minZ + buffer) || targetPos.z >(piso_maxZ - buffer))
		{
			return true; 
		}
		

		// --- ZONA DE ESCALERAS ---

		float esc_minX = -36.0f;
		float esc_maxX = -22.04f;
		float esc_minZ = -34.0f; 
		float esc_maxZ = -18.431f;

		if (targetPos.x > (esc_minX - buffer) && targetPos.x < (esc_maxX + buffer) &&
			targetPos.z >(esc_minZ - buffer) && targetPos.z < (esc_maxZ + buffer))
		{
			return true; 
		}

		// -- ZONA DE STANDS --
		
		// Stand 1: Ford 4x2
		float stand1_minX = -9.86f;
		float stand1_maxX = -1.69f;
		float stand1_minZ = -44.23f; 
		float stand1_maxZ = -40.16f;

		if (targetPos.x > (stand1_minX - buffer) && targetPos.x < (stand1_maxX + buffer) &&
			targetPos.z >(stand1_minZ - buffer) && targetPos.z < (stand1_maxZ + buffer)) {
			return true;
		}

		// Stand 2: Cisco / Intel 4x2
		float stand2_minX = -20.65f;
		float stand2_maxX = -12.48f;
		float stand2_minZ = -15.32f;
		float stand2_maxZ = -11.25f;

		if (targetPos.x > (stand2_minX - buffer) && targetPos.x < (stand2_maxX + buffer) &&
			targetPos.z >(stand2_minZ - buffer) && targetPos.z < (stand2_maxZ + buffer)) {
			return true;
		}

		// Stand 3: UNAM Aero Design
		float stand3_minX = -18.93f;
		float stand3_maxX = -14.76f;
		float stand3_minZ = -42.82f; 
		float stand3_maxZ = -40.75f;

		if (targetPos.x > (stand3_minX - buffer) && targetPos.x < (stand3_maxX + buffer) &&
			targetPos.z >(stand3_minZ - buffer) && targetPos.z < (stand3_maxZ + buffer)) {
			return true;
		}

		// -- ZONA DEL BANNER --
		float cartel_minX = -0.6414f - 1.5f;
		float cartel_maxX = -0.6414f + 1.5f;
		float cartel_minZ = -3.3628f - 1.5f;
		float cartel_maxZ = -3.3628f + 1.5f;

		if (targetPos.x > (cartel_minX - buffer) && targetPos.x < (cartel_maxX + buffer) &&
			targetPos.z >(cartel_minZ - buffer) && targetPos.z < (cartel_maxZ + buffer)) {
			return true;
		}

		return false; // El camino está libre
	}
};