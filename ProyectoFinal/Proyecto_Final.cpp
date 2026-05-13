/* ==========================================================================
   PROYECTO FINAL - CONJUNTO NORTE
   Equipo 6
   ==========================================================================
   Integrantes:
   - Echeverria Aguilar Luis Angel      (320236235)
   - Jacinto Robledo Valeria Berenice   (320057973)
   - Camarena Arevalo Yael Eduardo      (318279864)

   Fecha de entrega: 13/06/2026
   ========================================================================== */

#include <iostream>
#include <cmath>
#include <algorithm>
#include <vector>
#include <string>

   // GLEW
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// Other Libs
#include "stb_image.h"

// GLM Mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//Load Models
#include "SOIL2/SOIL2.h"

// Other includes
#include "Shader.h"
#include "Camera.h"
#include "Model.h"

// --- SISTEMA DE REPRODUCCIÓN DE KEYFRAMES (ORIENTADO A OBJETOS) ---
#include <fstream>
#include <sstream>

#define MAX_FRAMES 100 

typedef struct _frame {
	float visPosX, visPosY, visPosZ;
	float incX, incY, incZ;
	float rotVis, rotVisInc;
	float R_Arm, R_ArmInc;
	float L_Arm, L_ArmInc;
	float R_Leg, R_LegInc;
	float L_Leg, L_LegInc;
} FRAME;

struct Visitor {
	// Variables de posición y rotación únicas
	float posX = 0.0f, posY = 0.0f, posZ = 0.0f, rotVis = 0.0f;
	float R_Arm = 0.0f, L_Arm = 0.0f, R_Leg = 0.0f, L_Leg = 0.0f;

	// Variables del sistema de Keyframes
	FRAME KeyFrame[MAX_FRAMES];
	int FrameIndex = 0;
	bool play = false;
	int playIndex = 0;
	int i_curr_steps = 0;
	int i_max_steps = 80; // Pasos de interpolación

	void resetElements() {
		posX = KeyFrame[0].visPosX;
		posY = KeyFrame[0].visPosY;
		posZ = KeyFrame[0].visPosZ;
		rotVis = KeyFrame[0].rotVis;
		R_Arm = KeyFrame[0].R_Arm;
		L_Arm = KeyFrame[0].L_Arm;
		R_Leg = KeyFrame[0].R_Leg;
		L_Leg = KeyFrame[0].L_Leg;
	}

	void interpolation() {
		KeyFrame[playIndex].incX = (KeyFrame[playIndex + 1].visPosX - KeyFrame[playIndex].visPosX) / i_max_steps;
		KeyFrame[playIndex].incY = (KeyFrame[playIndex + 1].visPosY - KeyFrame[playIndex].visPosY) / i_max_steps;
		KeyFrame[playIndex].incZ = (KeyFrame[playIndex + 1].visPosZ - KeyFrame[playIndex].visPosZ) / i_max_steps;
		KeyFrame[playIndex].rotVisInc = (KeyFrame[playIndex + 1].rotVis - KeyFrame[playIndex].rotVis) / i_max_steps;
		KeyFrame[playIndex].R_ArmInc = (KeyFrame[playIndex + 1].R_Arm - KeyFrame[playIndex].R_Arm) / i_max_steps;
		KeyFrame[playIndex].L_ArmInc = (KeyFrame[playIndex + 1].L_Arm - KeyFrame[playIndex].L_Arm) / i_max_steps;
		KeyFrame[playIndex].R_LegInc = (KeyFrame[playIndex + 1].R_Leg - KeyFrame[playIndex].R_Leg) / i_max_steps;
		KeyFrame[playIndex].L_LegInc = (KeyFrame[playIndex + 1].L_Leg - KeyFrame[playIndex].L_Leg) / i_max_steps;
	}

	void loadAnimationFromFile(const char* filename) {
		std::ifstream file(filename);
		if (file.is_open()) {
			file >> FrameIndex;
			for (int i = 0; i < FrameIndex; i++) {
				file >> KeyFrame[i].visPosX >> KeyFrame[i].visPosY >> KeyFrame[i].visPosZ
					>> KeyFrame[i].rotVis >> KeyFrame[i].R_Arm >> KeyFrame[i].L_Arm
					>> KeyFrame[i].R_Leg >> KeyFrame[i].L_Leg;
			}
			file.close();
			printf("Animacion %s cargada, Total frames: %d\n", filename, FrameIndex);
			if (FrameIndex > 1) {
				resetElements();
				interpolation();
				play = true;
				playIndex = 0;
				i_curr_steps = 0;
			}
		}
		else {
			printf("No se encontro %s.\n", filename);
		}
	}

	void animate() {
		if (play) {
			if (i_curr_steps >= i_max_steps) {
				playIndex++;
				if (playIndex > FrameIndex - 2) {
					playIndex = 0;
					resetElements();
					interpolation();
					i_curr_steps = 0;
				}
				else {
					i_curr_steps = 0;
					interpolation();
				}
			}
			else {
				posX += KeyFrame[playIndex].incX;
				posY += KeyFrame[playIndex].incY;
				posZ += KeyFrame[playIndex].incZ;
				rotVis += KeyFrame[playIndex].rotVisInc;
				R_Arm += KeyFrame[playIndex].R_ArmInc;
				L_Arm += KeyFrame[playIndex].L_ArmInc;
				R_Leg += KeyFrame[playIndex].R_LegInc;
				L_Leg += KeyFrame[playIndex].L_LegInc;
				i_curr_steps++;
			}
		}
	}
};

// Instanciamos el arreglo para nuestros visitantes (ahora soporta hasta 6)
Visitor visitantes[6];
// --------------------------------------------------------------------

// --- SISTEMA DE REPRODUCCIÓN DE KEYFRAMES (VISITANTE) ---
//#include <fstream>
//#include <sstream>
//
//float visPosX = 0.0f, visPosY = 0.0f, visPosZ = 0.0f; // Posición en el lobby
//float rotVis = 0.0f; // Rotación del cuerpo entero
//
//float R_Arm = 0.0f; // Brazo Derecho
//float L_Arm = 0.0f; // Brazo Izquierdo
//float R_Leg = 0.0f; // Pierna Derecha
//float L_Leg = 0.0f; // Pierna Izquierda
//
//// --- VARIABLES PARA EL VISITANTE MASCULINO ---
//float visMasPosX = 0.0f, visMasPosY = 0.0f, visMasPosZ = 0.0f;
//float rotVisMas = 0.0f;
//
//float R_Arm_Mas = 0.0f;
//float L_Arm_Mas = 0.0f;
//float R_Leg_Mas = 0.0f;
//float L_Leg_Mas = 0.0f; 
//
//#define MAX_FRAMES 100 
//int i_max_steps = 80;
//int i_curr_steps = 0;
//
//typedef struct _frame {
//	float visPosX, visPosY, visPosZ;
//	float incX, incY, incZ;
//	float rotVis, rotVisInc;
//	float R_Arm, R_ArmInc;
//	float L_Arm, L_ArmInc;
//	float R_Leg, R_LegInc;
//	float L_Leg, L_LegInc;
//} FRAME;
//
//FRAME KeyFrame[MAX_FRAMES];
//int FrameIndex = 0;
//bool play = false;
//int playIndex = 0;
//
//// --- SISTEMA DE REPRODUCCIÓN (VISITANTE MASCULINO) ---
//FRAME KeyFrameMas[MAX_FRAMES];
//int FrameIndexMas = 0;
//bool playMas = false;
//int playIndexMas = 0;
//int i_curr_steps_mas = 0;
//
//void resetElements(void) {
//	visPosX = KeyFrame[0].visPosX;
//	visPosY = KeyFrame[0].visPosY;
//	visPosZ = KeyFrame[0].visPosZ;
//	rotVis = KeyFrame[0].rotVis;
//	R_Arm = KeyFrame[0].R_Arm;
//	L_Arm = KeyFrame[0].L_Arm;
//	R_Leg = KeyFrame[0].R_Leg;
//	L_Leg = KeyFrame[0].L_Leg;
//}
//
//void interpolation(void) {
//	KeyFrame[playIndex].incX = (KeyFrame[playIndex + 1].visPosX - KeyFrame[playIndex].visPosX) / i_max_steps;
//	KeyFrame[playIndex].incY = (KeyFrame[playIndex + 1].visPosY - KeyFrame[playIndex].visPosY) / i_max_steps;
//	KeyFrame[playIndex].incZ = (KeyFrame[playIndex + 1].visPosZ - KeyFrame[playIndex].visPosZ) / i_max_steps;
//	KeyFrame[playIndex].rotVisInc = (KeyFrame[playIndex + 1].rotVis - KeyFrame[playIndex].rotVis) / i_max_steps;
//	KeyFrame[playIndex].R_ArmInc = (KeyFrame[playIndex + 1].R_Arm - KeyFrame[playIndex].R_Arm) / i_max_steps;
//	KeyFrame[playIndex].L_ArmInc = (KeyFrame[playIndex + 1].L_Arm - KeyFrame[playIndex].L_Arm) / i_max_steps;
//	KeyFrame[playIndex].R_LegInc = (KeyFrame[playIndex + 1].R_Leg - KeyFrame[playIndex].R_Leg) / i_max_steps;
//	KeyFrame[playIndex].L_LegInc = (KeyFrame[playIndex + 1].L_Leg - KeyFrame[playIndex].L_Leg) / i_max_steps;
//}
//
//void loadAnimationFromFile(void) {
//	std::ifstream file("animacion_visitante.txt");
//	if (file.is_open()) {
//		file >> FrameIndex;
//		for (int i = 0; i < FrameIndex; i++) {
//			file >> KeyFrame[i].visPosX >> KeyFrame[i].visPosY >> KeyFrame[i].visPosZ
//				>> KeyFrame[i].rotVis >> KeyFrame[i].R_Arm >> KeyFrame[i].L_Arm
//				>> KeyFrame[i].R_Leg >> KeyFrame[i].L_Leg;
//		}
//		file.close();
//		printf("Animacion preguardada cargada, Total frames: %d\n", FrameIndex);
//		if (FrameIndex > 1) {
//			resetElements();
//			interpolation();
//			play = true; // Se pone en true para que arranque solita
//			playIndex = 0;
//			i_curr_steps = 0;
//		}
//	}
//	else {
//		printf("No se encontro animacion_visitante.txt.\n");
//	}
//}
//
//void Animation() {
//	if (play) {
//		if (i_curr_steps >= i_max_steps) {
//			playIndex++;
//			if (playIndex > FrameIndex - 2) {
//				playIndex = 0;
//				resetElements();
//				interpolation();
//				i_curr_steps = 0;
//			}
//			else {
//				i_curr_steps = 0;
//				interpolation();
//			}
//		}
//		else {
//			visPosX += KeyFrame[playIndex].incX;
//			visPosY += KeyFrame[playIndex].incY;
//			visPosZ += KeyFrame[playIndex].incZ;
//			rotVis += KeyFrame[playIndex].rotVisInc;
//			R_Arm += KeyFrame[playIndex].R_ArmInc;
//			L_Arm += KeyFrame[playIndex].L_ArmInc;
//			R_Leg += KeyFrame[playIndex].R_LegInc;
//			L_Leg += KeyFrame[playIndex].L_LegInc;
//			i_curr_steps++;
//		}
//	}
//}
//
//void resetElementsMas(void) {
//	visMasPosX = KeyFrameMas[0].visPosX;
//	visMasPosY = KeyFrameMas[0].visPosY;
//	visMasPosZ = KeyFrameMas[0].visPosZ;
//	rotVisMas = KeyFrameMas[0].rotVis;
//	R_Arm_Mas = KeyFrameMas[0].R_Arm;
//	L_Arm_Mas = KeyFrameMas[0].L_Arm;
//	R_Leg_Mas = KeyFrameMas[0].R_Leg;
//	L_Leg_Mas = KeyFrameMas[0].L_Leg;
//}
//
//void interpolationMas(void) {
//	KeyFrameMas[playIndexMas].incX = (KeyFrameMas[playIndexMas + 1].visPosX - KeyFrameMas[playIndexMas].visPosX) / i_max_steps;
//	KeyFrameMas[playIndexMas].incY = (KeyFrameMas[playIndexMas + 1].visPosY - KeyFrameMas[playIndexMas].visPosY) / i_max_steps;
//	KeyFrameMas[playIndexMas].incZ = (KeyFrameMas[playIndexMas + 1].visPosZ - KeyFrameMas[playIndexMas].visPosZ) / i_max_steps;
//	KeyFrameMas[playIndexMas].rotVisInc = (KeyFrameMas[playIndexMas + 1].rotVis - KeyFrameMas[playIndexMas].rotVis) / i_max_steps;
//	KeyFrameMas[playIndexMas].R_ArmInc = (KeyFrameMas[playIndexMas + 1].R_Arm - KeyFrameMas[playIndexMas].R_Arm) / i_max_steps;
//	KeyFrameMas[playIndexMas].L_ArmInc = (KeyFrameMas[playIndexMas + 1].L_Arm - KeyFrameMas[playIndexMas].L_Arm) / i_max_steps;
//	KeyFrameMas[playIndexMas].R_LegInc = (KeyFrameMas[playIndexMas + 1].R_Leg - KeyFrameMas[playIndexMas].R_Leg) / i_max_steps;
//	KeyFrameMas[playIndexMas].L_LegInc = (KeyFrameMas[playIndexMas + 1].L_Leg - KeyFrameMas[playIndexMas].L_Leg) / i_max_steps;
//}
//
//void loadAnimationFromFileMas(void) {
//	std::ifstream file("animacion_masculino.txt");
//	if (file.is_open()) {
//		file >> FrameIndexMas;
//		for (int i = 0; i < FrameIndexMas; i++) {
//			file >> KeyFrameMas[i].visPosX >> KeyFrameMas[i].visPosY >> KeyFrameMas[i].visPosZ
//				>> KeyFrameMas[i].rotVis >> KeyFrameMas[i].R_Arm >> KeyFrameMas[i].L_Arm
//				>> KeyFrameMas[i].R_Leg >> KeyFrameMas[i].L_Leg;
//		}
//		file.close();
//		printf("¡Animacion MASCULINA cargada! Total frames: %d\n", FrameIndexMas);
//		if (FrameIndexMas > 1) {
//			resetElementsMas();
//			interpolationMas();
//			playMas = true;
//			playIndexMas = 0;
//			i_curr_steps_mas = 0;
//		}
//	}
//	else {
//		printf("No se encontro animacion_masculino.txt.\n");
//	}
//}
//
//void AnimationMas() {
//	if (playMas) {
//		if (i_curr_steps_mas >= i_max_steps) {
//			playIndexMas++;
//			if (playIndexMas > FrameIndexMas - 2) {
//				playIndexMas = 0;
//				resetElementsMas();
//				interpolationMas();
//				i_curr_steps_mas = 0;
//			}
//			else {
//				i_curr_steps_mas = 0;
//				interpolationMas();
//			}
//		}
//		else {
//			visMasPosX += KeyFrameMas[playIndexMas].incX;
//			visMasPosY += KeyFrameMas[playIndexMas].incY;
//			visMasPosZ += KeyFrameMas[playIndexMas].incZ;
//			rotVisMas += KeyFrameMas[playIndexMas].rotVisInc;
//			R_Arm_Mas += KeyFrameMas[playIndexMas].R_ArmInc;
//			L_Arm_Mas += KeyFrameMas[playIndexMas].L_ArmInc;
//			R_Leg_Mas += KeyFrameMas[playIndexMas].R_LegInc;
//			L_Leg_Mas += KeyFrameMas[playIndexMas].L_LegInc;
//			i_curr_steps_mas++;
//		}
//	}
//}

// Function prototypes
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow* window, double xPos, double yPos);
void DoMovement();

// Window dimensions
const GLuint WIDTH = 800, HEIGHT = 600;
int SCREEN_WIDTH, SCREEN_HEIGHT;

// Camera
Camera  camera(glm::vec3(-6.0f, 4.0f, -15.0f));
GLfloat lastX = WIDTH / 2.0;
GLfloat lastY = HEIGHT / 2.0;
bool keys[1024];
bool firstMouse = true;

// Light attributes
glm::vec3 lightPos(0.0f, 0.0f, 0.0f);
bool active;

// --- Sistema de Iluminacion Dinamica Dia / Noche ---
// dayMode = true  -> Vista de Dia
// dayMode = false -> Vista Nocturna
// dayFactor interpola suavemente entre 0.0 (noche) y 1.0 (dia)
bool dayMode = true;
float dayFactor = 1.0f;
const float DAY_NIGHT_TRANSITION_SPEED = 2.0f; // segundos para completar la transicion

// Positions of the point lights
glm::vec3 pointLightPositions[] = {
	glm::vec3(0.0f, 5.0f, 0.0f), // Subí un poco la luz para iluminar mejor
	glm::vec3(0.0f, 0.0f, 0.0f),
	glm::vec3(0.0f, 0.0f, 0.0f),
	glm::vec3(0.0f, 0.0f, 0.0f)
};

// Positions of the roof lights (Ceiling lights)
// Coordenadas tomadas del centro exacto de cada malla en Lamparas.obj
glm::vec3 roofLightPositions[] = {
	glm::vec3(5.1064f,   8.3336f, -17.0502f),  // P1
	glm::vec3(-5.5031f,  8.3336f, -17.0502f),  // P2
	glm::vec3(-16.1718f, 8.3336f, -17.0502f),  // P3
	glm::vec3(5.1064f,   8.3336f, -28.0097f),  // P4
	glm::vec3(-5.5031f,  8.3336f, -28.0097f),  // P5
	glm::vec3(-16.1718f, 8.3336f, -28.0097f),  // P6
	glm::vec3(5.1064f,   8.3336f, -39.3118f),  // P7 (lampara larga: 4.85 x 0.91)
	glm::vec3(-5.5031f,  8.3336f, -39.3118f),  // P8
	glm::vec3(-16.1718f, 8.3336f, -39.3118f),  // P9
	glm::vec3(-25.0226f, 8.3336f, -17.3416f),  // P10
	glm::vec3(-27.7818f, 8.3336f, -17.3416f),  // P11
	glm::vec3(-31.0772f, 8.3336f, -17.3416f),  // P12
	glm::vec3(-34.7410f, 8.3336f, -17.3416f),  // P13
	glm::vec3(-38.1671f, 8.3336f, -17.3416f),  // P14
	glm::vec3(-38.1671f, 8.3336f, -20.2091f),  // P15
	glm::vec3(-38.1671f, 8.3336f, -23.1389f),  // P16
	glm::vec3(-38.1671f, 8.3336f, -26.0543f),  // P17
	glm::vec3(-38.1671f, 8.3336f, -29.1360f),  // P18
	glm::vec3(-38.1671f, 8.3336f, -39.7622f),  // P19
	glm::vec3(-34.7410f, 8.3336f, -39.7622f),  // P20
	glm::vec3(-31.0772f, 8.3336f, -39.7622f),  // P21
	glm::vec3(-27.7818f, 8.3336f, -39.7622f),  // P22
	glm::vec3(-25.0226f, 8.3336f, -39.7622f),  // P23
	glm::vec3(-38.1671f, 8.3336f, -32.1696f),  // P24
	glm::vec3(-38.1671f, 8.3336f, -35.9375f)   // P25
};

float vertices[] = {
	 -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	   -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	   -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

	   -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
	   -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
	   -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

	   -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
	   -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	   -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	   -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	   -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
	   -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

	   -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
	   -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
	   -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

	   -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
	   -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
	   -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
};

glm::vec3 Light1 = glm::vec3(0);

// Deltatime
GLfloat deltaTime = 0.0f;	// Time between current frame and last frame
GLfloat lastFrame = 0.0f;  	// Time of last frame

// --- Variables para la animación del Roll-up ---
float animProgress = 0.0f;
bool bannerOpen = false;
const float ALTURA_LONA = 0.3f;

// --- Skybox ---
GLuint skyboxVAO, skyboxVBO;
GLuint cubemapDay, cubemapNight;
Shader* skyboxShaderPtr;

GLfloat skyboxVertices[] = {
	// Positions
	-1.0f,  1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	-1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f
};

// --- Función para Cargar Cubemap ---
GLuint LoadCubemap(std::vector<std::string> faces)
{
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;

	for (GLuint i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Error cargando skybox: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}

// --- Función de Interpolación Elástica (Efecto de Rebote) ---
float easeOutBack(float x) {
	const float c1 = 1.70158f;
	const float c3 = c1 + 1.0f;

	if (x == 0.0f) return 0.0f;
	if (x >= 1.0f) return 1.0f;

	// Sube, pasa un poquitito el límite simulando el tirón humano, y baja a su lugar
	return 1.0f + c3 * pow(x - 1.0f, 3.0f) + c1 * pow(x - 1.0f, 2.0f);
}



int main()
{
	// Init GLFW
	glfwInit();

	// Create a GLFWwindow object that we can use for GLFW's functions
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Conjunto Norte - Equipo 6", nullptr, nullptr);

	if (nullptr == window)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return EXIT_FAILURE;
	}

	glfwMakeContextCurrent(window);
	glfwGetFramebufferSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);

	// Set the required callback functions
	glfwSetKeyCallback(window, KeyCallback);
	glfwSetCursorPosCallback(window, MouseCallback);

	// GLFW Options (Descomenta esto para ocultar el cursor si vas a navegar el modelo)
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
	glewExperimental = GL_TRUE;

	// Initialize GLEW to setup the OpenGL Function pointers
	if (GLEW_OK != glewInit())
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		return EXIT_FAILURE;
	}

	// Define the viewport dimensions
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	Shader lightingShader("Shader/lighting.vs", "Shader/lighting.frag");
	Shader lampShader("Shader/lamp.vs", "Shader/lamp.frag");
	Shader flagShader("Shader/flag_wave.vs", "Shader/lighting.frag");

	// --- Configuración e Inicialización de Skybox ---
	Shader skyboxShader("Shader/SkyBox.vs", "Shader/SkyBox.frag");
	skyboxShaderPtr = &skyboxShader;

	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);

	std::vector<std::string> facesDay = {
		"SkyBox/rightdia.jpg",
		"SkyBox/leftdia.jpg",
		"SkyBox/topdia.jpg",
		"SkyBox/bottomdia.jpg",
		"SkyBox/backdia.jpg",
		"SkyBox/frontdia.jpg"
	};

	std::vector<std::string> facesNight = {
		"SkyBox/rightnoche.jpg",
		"SkyBox/leftnoche.jpg",
		"SkyBox/topnoche.jpg",
		"SkyBox/bottomdnoche.jpg",
		"SkyBox/backnoche.jpg",
		"SkyBox/frontnoche.jpg"
	};

	cubemapDay = LoadCubemap(facesDay);
	cubemapNight = LoadCubemap(facesNight);
	// -----------------------------------------------

	// CARGA DE MODELOS
	Model conjuntoNorte((char*)"Models/conjuntoNorte/conjunto_norte.obj");
	Model stand1((char*)"Models/stands/stand1_2x1.obj");
	Model stand2((char*)"Models/stands/stand1_4x2.obj");
	Model stand3((char*)"Models/stands/stand2_4x2.obj");
	Model tubo((char*)"Models/banner/tubo_banner.obj");
	Model lamparas((char*)"Models/lamparas/Lamparas.obj");
	Model banner((char*)"Models/banner/banner.obj");
	Model banderaFlag((char*)"Models/banderas/bandera_.obj");
	Model banderaUnam((char*)"Models/banderas/banderaunamaerodesign.obj");
	Model banderaCisco((char*)"Models/banderas/banderacisco.obj");
	Model tomacorriente((char*)"Models/senalizacion de corriente/Tomacorriente.obj");

	// --- CARGA DEL MODELO FEMENINO ---
	Model cabezaFemenino((char*)"Models/visitanteFemenino/cabezaFemenino.obj");
	Model brazoDerFemenino((char*)"Models/visitanteFemenino/brazoDerFemenino.obj");
	Model brazoIzqFemenino((char*)"Models/visitanteFemenino/brazoIzqFemenino.obj");
	Model piernaDerFemenino((char*)"Models/visitanteFemenino/piernaDerFemenino.obj");
	Model torsoFemenino((char*)"Models/visitanteFemenino/torsoFemenino.obj");
	Model piernaIzqFemenino((char*)"Models/visitanteFemenino/piernaIzqFemenino.obj");

	// --- CARGA DEL MODELO MASCULINO ---
	Model cabezaMasculino((char*)"Models/visitanteMasculino/cabezaMasculino.obj");
	Model brazoDerMasculino((char*)"Models/visitanteMasculino/brazoDerMasculino.obj");
	Model brazoIzqMasculino((char*)"Models/visitanteMasculino/brazoIzqMasculino.obj");
	Model piernaDerMasculino((char*)"Models/visitanteMasculino/piernaDerMasculino.obj");
	Model torsoMasculino((char*)"Models/visitanteMasculino/torsoMasculino.obj");
	Model piernaIzqMasculino((char*)"Models/visitanteMasculino/piernaIzqMasculino.obj");


	// Setup VAO and VBO for the light cube
	GLuint VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	// Normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// Set texture units
	lightingShader.Use();
	glUniform1f(glGetUniformLocation(lightingShader.Program, "emissive"), 0.0f);
	glUniform1i(glGetUniformLocation(lightingShader.Program, "Material.difuse"), 0);
	glUniform1i(glGetUniformLocation(lightingShader.Program, "Material.specular"), 1);

	flagShader.Use();
	glUniform1f(glGetUniformLocation(flagShader.Program, "emissive"), 0.0f);
	glUniform1i(glGetUniformLocation(flagShader.Program, "material.diffuse"), 0);
	glUniform1i(glGetUniformLocation(flagShader.Program, "material.specular"), 1);

	glm::mat4 projection = glm::perspective(camera.GetZoom(), (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT, 0.1f, 100.0f);

	// Cargar la animación del visitante al inicio
	/*loadAnimationFromFile();
	loadAnimationFromFileMas();*/

	// Cargar las animaciones de los visitantes
	visitantes[0].loadAnimationFromFile("animacion_visitante.txt"); // Mujer
	visitantes[1].loadAnimationFromFile("animacion_masculino.txt"); // Hombre

	// Game loop
	while (!glfwWindowShouldClose(window))
	{
		// Calculate deltatime of current frame
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glfwPollEvents();
		DoMovement();

		/*Animation();
		AnimationMas();*/

		// Actualizar el "cerebro" de los 2 visitantes actuales
		for (int i = 0; i < 2; i++) {
			visitantes[i].animate();
		}

		// --- Sistema Dinamico Dia / Noche : interpolacion suave ---
		float targetFactor = dayMode ? 1.0f : 0.0f;
		if (dayFactor < targetFactor)
			dayFactor = std::min(targetFactor, dayFactor + deltaTime / DAY_NIGHT_TRANSITION_SPEED);
		else if (dayFactor > targetFactor)
			dayFactor = std::max(targetFactor, dayFactor - deltaTime / DAY_NIGHT_TRANSITION_SPEED);

		// Color del cielo : azul claro de dia, azul muy oscuro de noche (Opcional, el Skybox ya cubre el fondo)
		glm::vec3 daySky(0.53f, 0.81f, 0.92f);
		glm::vec3 nightSky(0.02f, 0.02f, 0.08f);
		glm::vec3 sky = glm::mix(nightSky, daySky, dayFactor);
		glClearColor(sky.r, sky.g, sky.b, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// OpenGL options
		glEnable(GL_DEPTH_TEST);

		lightingShader.Use();
		glUniform1i(glGetUniformLocation(lightingShader.Program, "diffuse"), 0);

		GLint viewPosLoc = glGetUniformLocation(lightingShader.Program, "viewPos");
		glUniform3f(viewPosLoc, camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);

		// --- Luz direccional : Sol calido fuerte de dia, Luna fria tenue de noche ---
		// (Simula la luz que entra por los ventanales del conjunto norte)
		glm::vec3 dayDirAmb(0.66f, 0.66f, 0.61f);   // interior de dia mas iluminado sin quemar
		glm::vec3 dayDirDif(1.03f, 0.98f, 0.85f); // tono calido del sol
		glm::vec3 dayDirSpc(0.49f, 0.49f, 0.43f);
		glm::vec3 nightDirAmb(0.04f, 0.04f, 0.08f);
		glm::vec3 nightDirDif(0.10f, 0.12f, 0.22f); // tono frio de luna
		glm::vec3 nightDirSpc(0.05f, 0.05f, 0.10f);

		glm::vec3 dirAmb = glm::mix(nightDirAmb, dayDirAmb, dayFactor);
		glm::vec3 dirDif = glm::mix(nightDirDif, dayDirDif, dayFactor);
		glm::vec3 dirSpc = glm::mix(nightDirSpc, dayDirSpc, dayFactor);

		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.direction"), -0.2f, -1.0f, -0.3f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.ambient"), dirAmb.r, dirAmb.g, dirAmb.b);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.diffuse"), dirDif.r, dirDif.g, dirDif.b);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.specular"), dirSpc.r, dirSpc.g, dirSpc.b);

		// --- Luz puntual : focos interiores apagados de dia, encendidos calidos de noche ---
		// Si el modo "evento" (tecla ESPACIO) esta activo, la animacion strobe tiene prioridad.
		glm::vec3 ptAmb, ptDif, ptSpc;
		if (active)
		{
			// Modo evento : efecto strobe original del equipo
			glm::vec3 lightColor;
			lightColor.x = abs(sin(glfwGetTime() * Light1.x));
			lightColor.y = abs(sin(glfwGetTime() * Light1.y));
			lightColor.z = sin(glfwGetTime() * Light1.z);
			ptAmb = lightColor;
			ptDif = lightColor;
			ptSpc = glm::vec3(1.0f, 0.2f, 0.2f);
		}
		else
		{
			// Iluminacion interior : foco amarillo calido encendido en la noche
			glm::vec3 dayPtAmb(0.0f, 0.0f, 0.0f);
			glm::vec3 dayPtDif(0.0f, 0.0f, 0.0f);
			glm::vec3 dayPtSpc(0.0f, 0.0f, 0.0f);
			glm::vec3 nightPtAmb(0.20f, 0.15f, 0.05f);
			glm::vec3 nightPtDif(1.00f, 0.85f, 0.40f);
			glm::vec3 nightPtSpc(1.00f, 0.90f, 0.50f);
			ptAmb = glm::mix(nightPtAmb, dayPtAmb, dayFactor);
			ptDif = glm::mix(nightPtDif, dayPtDif, dayFactor);
			ptSpc = glm::mix(nightPtSpc, dayPtSpc, dayFactor);
		}

		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].position"), pointLightPositions[0].x, pointLightPositions[0].y, pointLightPositions[0].z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].ambient"), ptAmb.r, ptAmb.g, ptAmb.b);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].diffuse"), ptDif.r, ptDif.g, ptDif.b);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].specular"), ptSpc.r, ptSpc.g, ptSpc.b);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].constant"), 1.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].linear"), 0.045f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].quadratic"), 0.075f);

		// SpotLight
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.position"), camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.direction"), camera.GetFront().x, camera.GetFront().y, camera.GetFront().z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.ambient"), 0.2f, 0.2f, 0.8f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.diffuse"), 0.2f, 0.2f, 0.8f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.specular"), 0.0f, 0.0f, 0.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.constant"), 1.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.linear"), 0.3f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.quadratic"), 0.7f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.cutOff"), glm::cos(glm::radians(12.0f)));
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.outerCutOff"), glm::cos(glm::radians(18.0f)));

		// --- Luces de techo (25 focos) : Solo encendidas de noche ---
		glm::vec3 dayRoofDif(0.0f, 0.0f, 0.0f);
		glm::vec3 nightRoofDif(2.30f, 2.10f, 1.75f);   // cercano al original, solo un poco menos
		glm::vec3 roofDif = glm::mix(nightRoofDif, dayRoofDif, dayFactor);

		glm::vec3 dayRoofSpc(0.0f, 0.0f, 0.0f);
		glm::vec3 nightRoofSpc(0.55f, 0.49f, 0.40f);   // specular separado para no blanquear
		glm::vec3 roofSpc = glm::mix(nightRoofSpc, dayRoofSpc, dayFactor);

		glm::vec3 dayRoofAmb(0.0f, 0.0f, 0.0f);
		glm::vec3 nightRoofAmb(0.085f, 0.076f, 0.063f);
		glm::vec3 roofAmb = glm::mix(nightRoofAmb, dayRoofAmb, dayFactor);

		for (int i = 0; i < 25; i++) {
			std::string base = "roofLights[" + std::to_string(i) + "].";
			glUniform3f(glGetUniformLocation(lightingShader.Program, (base + "position").c_str()), roofLightPositions[i].x, roofLightPositions[i].y, roofLightPositions[i].z);
			glUniform3f(glGetUniformLocation(lightingShader.Program, (base + "direction").c_str()), 0.0f, -1.0f, 0.0f);

			glUniform3f(glGetUniformLocation(lightingShader.Program, (base + "ambient").c_str()), roofAmb.r, roofAmb.g, roofAmb.b);
			glUniform3f(glGetUniformLocation(lightingShader.Program, (base + "diffuse").c_str()), roofDif.r, roofDif.g, roofDif.b);
			glUniform3f(glGetUniformLocation(lightingShader.Program, (base + "specular").c_str()), roofSpc.r, roofSpc.g, roofSpc.b);
			glUniform1f(glGetUniformLocation(lightingShader.Program, (base + "constant").c_str()), 1.0f);

			glUniform1f(glGetUniformLocation(lightingShader.Program, (base + "linear").c_str()), 0.007f);
			glUniform1f(glGetUniformLocation(lightingShader.Program, (base + "quadratic").c_str()), 0.0002f);

			glUniform1f(glGetUniformLocation(lightingShader.Program, (base + "cutOff").c_str()), glm::cos(glm::radians(43.0f)));
			glUniform1f(glGetUniformLocation(lightingShader.Program, (base + "outerCutOff").c_str()), glm::cos(glm::radians(63.0f)));
		}

		// Set material properties
		glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 5.0f);

		// Create camera transformations
		glm::mat4 view = camera.GetViewMatrix();

		// Get the uniform locations
		GLint modelLoc = glGetUniformLocation(lightingShader.Program, "model");
		GLint viewLoc = glGetUniformLocation(lightingShader.Program, "view");
		GLint projLoc = glGetUniformLocation(lightingShader.Program, "projection");

		// Pass the matrices to the shader
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

		// --- DIBUJADO DE LA FACULTAD ---
		glm::mat4 model(1); // Aquí se declara por PRIMERA vez
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0);
		conjuntoNorte.Draw(lightingShader);
		// -------------------------------


		// --- DIBUJADO DE LOS STANDS ---
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0);
		stand1.Draw(lightingShader);


		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0);
		stand2.Draw(lightingShader);


		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0);
		stand3.Draw(lightingShader);

		// --- DIBUJADO DE LAS LAMPARAS ---
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0);

		float lampEmissive = (1.0f - dayFactor) * 1.8f;

		if (active) lampEmissive = 1.8f;

		glUniform1f(glGetUniformLocation(lightingShader.Program, "emissive"), lampEmissive);
		lamparas.Draw(lightingShader);

		glUniform1f(glGetUniformLocation(lightingShader.Program, "emissive"), 0.0f);
		// -------------------------------

		// --- DIBUJADO DE LA SEÑALIZACIÓN DE LA CORRIENTE ---
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0);
		tomacorriente.Draw(lightingShader);
		// -------------------------------

		// --- DIBUJADO DEL VISITANTE ANIMADO POR KEYFRAMES (MUJER - Index 0) ---
		glm::mat4 baseModel = glm::mat4(1.0f);

		// 1. Aplicamos el desplazamiento del recorrido
		baseModel = glm::translate(baseModel, glm::vec3(visitantes[0].posX, visitantes[0].posY, visitantes[0].posZ));

		// 2. Rotar sobre su propio centro
		glm::vec3 pivotCentro(-4.2f, 0.0f, -11.5f);
		baseModel = glm::translate(baseModel, pivotCentro);
		baseModel = glm::rotate(baseModel, glm::radians(visitantes[0].rotVis), glm::vec3(0.0f, 1.0f, 0.0f));
		baseModel = glm::translate(baseModel, -pivotCentro);

		glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0);

		// Cabeza y Torso
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(baseModel));
		cabezaFemenino.Draw(lightingShader);
		torsoFemenino.Draw(lightingShader);

		// Brazo Derecho
		model = baseModel;
		glm::vec3 pivotBrazoDer(-3.8167f, 3.5118f, -11.4444f);
		model = glm::translate(model, pivotBrazoDer);
		model = glm::rotate(model, glm::radians(visitantes[0].R_Arm), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::translate(model, -pivotBrazoDer);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		brazoDerFemenino.Draw(lightingShader);

		// Brazo Izquierdo
		model = baseModel;
		glm::vec3 pivotBrazoIzq(-4.5887f, 3.5097f, -11.4444f);
		model = glm::translate(model, pivotBrazoIzq);
		model = glm::rotate(model, glm::radians(visitantes[0].L_Arm), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::translate(model, -pivotBrazoIzq);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		brazoIzqFemenino.Draw(lightingShader);

		// Pierna Derecha
		model = baseModel;
		glm::vec3 pivotPiernaDer(-3.8924f, 2.6617f, -11.6100f);
		model = glm::translate(model, pivotPiernaDer);
		model = glm::rotate(model, glm::radians(visitantes[0].R_Leg), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::translate(model, -pivotPiernaDer);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		piernaDerFemenino.Draw(lightingShader);

		// Pierna Izquierda
		model = baseModel;
		glm::vec3 pivotPiernaIzq(-4.5216f, 2.6617f, -11.6100f);
		model = glm::translate(model, pivotPiernaIzq);
		model = glm::rotate(model, glm::radians(visitantes[0].L_Leg), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::translate(model, -pivotPiernaIzq);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		piernaIzqFemenino.Draw(lightingShader);
		// ----------------------------------------------------

		// --- DIBUJADO DEL VISITANTE MASCULINO (HOMBRE - Index 1) ---
		glm::mat4 baseModelMas = glm::mat4(1.0f);

		// 1. Posición ajustable
		baseModelMas = glm::translate(baseModelMas, glm::vec3(visitantes[1].posX, visitantes[1].posY, visitantes[1].posZ));

		// 2. Pivote central
		glm::vec3 pivotCentroMas(-7.8431f, 0.0f, -11.5270f);
		baseModelMas = glm::translate(baseModelMas, pivotCentroMas);
		baseModelMas = glm::rotate(baseModelMas, glm::radians(visitantes[1].rotVis), glm::vec3(0.0f, 1.0f, 0.0f));
		baseModelMas = glm::translate(baseModelMas, -pivotCentroMas);

		glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0);

		// Cabeza y Torso
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(baseModelMas));
		cabezaMasculino.Draw(lightingShader);
		torsoMasculino.Draw(lightingShader);

		// Brazo Derecho
		model = baseModelMas;
		glm::vec3 pivotBDMas(-7.4376f, 3.6748f, -11.4620f);
		model = glm::translate(model, pivotBDMas);
		model = glm::rotate(model, glm::radians(visitantes[1].R_Arm), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::translate(model, -pivotBDMas);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		brazoDerMasculino.Draw(lightingShader);

		// Brazo Izquierdo
		model = baseModelMas;
		glm::vec3 pivotBIMas(-8.2487f, 3.6743f, -11.4460f);
		model = glm::translate(model, pivotBIMas);
		model = glm::rotate(model, glm::radians(visitantes[1].L_Arm), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::translate(model, -pivotBIMas);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		brazoIzqMasculino.Draw(lightingShader);

		// Pierna Derecha
		model = baseModelMas;
		glm::vec3 pivotPDMas(-7.5539f, 2.8185f, -11.5980f);
		model = glm::translate(model, pivotPDMas);
		model = glm::rotate(model, glm::radians(visitantes[1].R_Leg), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::translate(model, -pivotPDMas);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		piernaDerMasculino.Draw(lightingShader);

		// Pierna Izquierda
		model = baseModelMas;
		glm::vec3 pivotPIMas(-8.1341f, 2.8189f, -11.5870f);
		model = glm::translate(model, pivotPIMas);
		model = glm::rotate(model, glm::radians(visitantes[1].L_Leg), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::translate(model, -pivotPIMas);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		piernaIzqMasculino.Draw(lightingShader);
		// ----------------------------------------

		//// --- DIBUJADO DEL VISITANTE ANIMADO POR KEYFRAMES ---
		//glm::mat4 baseModel = glm::mat4(1.0f);

		//// 1. Aplicamos el desplazamiento del recorrido (Caminata del Keyframe)
		//baseModel = glm::translate(baseModel, glm::vec3(visPosX, visPosY, visPosZ));

		//// 2. Rotar sobre su propio centro
		//glm::vec3 pivotCentro(-4.2f, 0.0f, -11.5f); // Centro de gravedad de tu modelo
		//baseModel = glm::translate(baseModel, pivotCentro);
		//baseModel = glm::rotate(baseModel, glm::radians(rotVis), glm::vec3(0.0f, 1.0f, 0.0f));
		//baseModel = glm::translate(baseModel, -pivotCentro);

		//glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0);

		//// Cabeza y Torso
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(baseModel));
		//cabezaFemenino.Draw(lightingShader);
		//torsoFemenino.Draw(lightingShader);

		//// Brazo Derecho
		//model = baseModel;
		//glm::vec3 pivotBrazoDer(-3.8167f, 3.5118f, -11.4444f);
		//model = glm::translate(model, pivotBrazoDer);
		//model = glm::rotate(model, glm::radians(R_Arm), glm::vec3(1.0f, 0.0f, 0.0f));
		//model = glm::translate(model, -pivotBrazoDer);
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//brazoDerFemenino.Draw(lightingShader);

		//// Brazo Izquierdo
		//model = baseModel;
		//glm::vec3 pivotBrazoIzq(-4.5887f, 3.5097f, -11.4444f);
		//model = glm::translate(model, pivotBrazoIzq);
		//model = glm::rotate(model, glm::radians(L_Arm), glm::vec3(1.0f, 0.0f, 0.0f));
		//model = glm::translate(model, -pivotBrazoIzq);
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//brazoIzqFemenino.Draw(lightingShader);

		//// Pierna Derecha
		//model = baseModel;
		//glm::vec3 pivotPiernaDer(-3.8924f, 2.6617f, -11.6100f);
		//model = glm::translate(model, pivotPiernaDer);
		//model = glm::rotate(model, glm::radians(R_Leg), glm::vec3(1.0f, 0.0f, 0.0f));
		//model = glm::translate(model, -pivotPiernaDer);
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//piernaDerFemenino.Draw(lightingShader);

		//// Pierna Izquierda
		//model = baseModel;
		//glm::vec3 pivotPiernaIzq(-4.5216f, 2.6617f, -11.6100f);
		//model = glm::translate(model, pivotPiernaIzq);
		//model = glm::rotate(model, glm::radians(L_Leg), glm::vec3(1.0f, 0.0f, 0.0f));
		//model = glm::translate(model, -pivotPiernaIzq);
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//piernaIzqFemenino.Draw(lightingShader);
		//// ----------------------------------------------------

		//// --- DIBUJADO DEL VISITANTE MASCULINO ---
		//glm::mat4 baseModelMas = glm::mat4(1.0f);

		//// 1. Posición ajustable desde las variables globales
		//baseModelMas = glm::translate(baseModelMas, glm::vec3(visMasPosX, visMasPosY, visMasPosZ));

		//// 2. Pivote central del hombre (para evitar teletransportación)
		//glm::vec3 pivotCentroMas(-7.8431f, 0.0f, -11.5270f);
		//baseModelMas = glm::translate(baseModelMas, pivotCentroMas);
		//baseModelMas = glm::rotate(baseModelMas, glm::radians(rotVisMas), glm::vec3(0.0f, 1.0f, 0.0f));
		//baseModelMas = glm::translate(baseModelMas, -pivotCentroMas);

		//glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0);

		//// Cabeza y Torso
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(baseModelMas));
		//cabezaMasculino.Draw(lightingShader);
		//torsoMasculino.Draw(lightingShader); 

		//// Brazo Derecho
		//model = baseModelMas;
		//glm::vec3 pivotBDMas(-7.4376f, 3.6748f, -11.4620f);
		//model = glm::translate(model, pivotBDMas);
		//model = glm::rotate(model, glm::radians(R_Arm_Mas), glm::vec3(1.0f, 0.0f, 0.0f));
		//model = glm::translate(model, -pivotBDMas);
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//brazoDerMasculino.Draw(lightingShader);

		//// Brazo Izquierdo
		//model = baseModelMas;
		//glm::vec3 pivotBIMas(-8.2487f, 3.6743f, -11.4460f);
		//model = glm::translate(model, pivotBIMas);
		//model = glm::rotate(model, glm::radians(L_Arm_Mas), glm::vec3(1.0f, 0.0f, 0.0f));
		//model = glm::translate(model, -pivotBIMas);
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//brazoIzqMasculino.Draw(lightingShader);

		//// Pierna Derecha
		//model = baseModelMas;
		//glm::vec3 pivotPDMas(-7.5539f, 2.8185f, -11.5980f);
		//model = glm::translate(model, pivotPDMas);
		//model = glm::rotate(model, glm::radians(R_Leg_Mas), glm::vec3(1.0f, 0.0f, 0.0f));
		//model = glm::translate(model, -pivotPDMas);
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//piernaDerMasculino.Draw(lightingShader);

		//// Pierna Izquierda
		//model = baseModelMas;
		//glm::vec3 pivotPIMas(-8.1341f, 2.8189f, -11.5870f);
		//model = glm::translate(model, pivotPIMas);
		//model = glm::rotate(model, glm::radians(L_Leg_Mas), glm::vec3(1.0f, 0.0f, 0.0f));
		//model = glm::translate(model, -pivotPIMas);
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model)); 
		//piernaIzqMasculino.Draw(lightingShader); 
		//// ----------------------------------------

		//// --- LÓGICA DE ANIMACIÓN DEL ROLL-UP ---

		////DIBUJAR LA BASE 
		//model = glm::mat4(1);
		//model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//tubo.Draw(lightingShader);

		//// LÓGICA Y DIBUJO DE LA LONA
		////abierto / cerrado
		//if (bannerOpen) {
		//	if (animProgress < 1.0f) {
		//		animProgress += deltaTime * 1.0f; // Velocidad de caída
		//		if (animProgress > 1.0f) animProgress = 1.0f;
		//	}
		//}
		//else {
		//	if (animProgress > 0.0f) {
		//		animProgress -= deltaTime * 1.5f; // Sube un poco más rápido de lo que cae
		//		if (animProgress < 0.0f) animProgress = 0.0f;
		//	}
		//}

		//if (animProgress > 0.0f) {
		//	float Sy = easeOutBack(animProgress);
		//	if (Sy < 0.001f) Sy = 0.001f;

		//	float Y_TUBO = 4.88f;
		//	float Ty = ALTURA_LONA * (1.0f - Sy);

		//	model = glm::mat4(1);
		//	model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.01f));

		//	// Transformación anclada arriba
		//	model = glm::translate(model, glm::vec3(0.0f, Y_TUBO, 0.0f));
		//	model = glm::scale(model, glm::vec3(1.0f, Sy, 1.0f));
		//	model = glm::translate(model, glm::vec3(0.0f, -Y_TUBO, 0.0f));

		//	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//	banner.Draw(lightingShader);
		//}
		//// ----------------------------------------

		// ====================================================================
		// BANDERA PUBLICITARIA - Deformacion Procedimental de Vertices 
		// ====================================================================
		flagShader.Use();
		{
			GLuint fp = flagShader.Program;

			// --- Iluminacion (reutiliza valores ya calculados en este frame) ---
			glUniform3f(glGetUniformLocation(fp, "viewPos"),
				camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);

			glUniform3f(glGetUniformLocation(fp, "dirLight.direction"), -0.2f, -1.0f, -0.3f);
			glUniform3f(glGetUniformLocation(fp, "dirLight.ambient"), dirAmb.r, dirAmb.g, dirAmb.b);
			glUniform3f(glGetUniformLocation(fp, "dirLight.diffuse"), dirDif.r, dirDif.g, dirDif.b);
			glUniform3f(glGetUniformLocation(fp, "dirLight.specular"), dirSpc.r, dirSpc.g, dirSpc.b);

			glUniform3f(glGetUniformLocation(fp, "pointLights[0].position"),
				pointLightPositions[0].x, pointLightPositions[0].y, pointLightPositions[0].z);
			glUniform3f(glGetUniformLocation(fp, "pointLights[0].ambient"), ptAmb.r, ptAmb.g, ptAmb.b);
			glUniform3f(glGetUniformLocation(fp, "pointLights[0].diffuse"), ptDif.r, ptDif.g, ptDif.b);
			glUniform3f(glGetUniformLocation(fp, "pointLights[0].specular"), ptSpc.r, ptSpc.g, ptSpc.b);
			glUniform1f(glGetUniformLocation(fp, "pointLights[0].constant"), 1.0f);
			glUniform1f(glGetUniformLocation(fp, "pointLights[0].linear"), 0.045f);
			glUniform1f(glGetUniformLocation(fp, "pointLights[0].quadratic"), 0.075f);

			glUniform3f(glGetUniformLocation(fp, "spotLight.position"),
				camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);
			glUniform3f(glGetUniformLocation(fp, "spotLight.direction"),
				camera.GetFront().x, camera.GetFront().y, camera.GetFront().z);
			glUniform3f(glGetUniformLocation(fp, "spotLight.ambient"), 0.2f, 0.2f, 0.8f);
			glUniform3f(glGetUniformLocation(fp, "spotLight.diffuse"), 0.2f, 0.2f, 0.8f);
			glUniform3f(glGetUniformLocation(fp, "spotLight.specular"), 0.0f, 0.0f, 0.0f);
			glUniform1f(glGetUniformLocation(fp, "spotLight.constant"), 1.0f);
			glUniform1f(glGetUniformLocation(fp, "spotLight.linear"), 0.3f);
			glUniform1f(glGetUniformLocation(fp, "spotLight.quadratic"), 0.7f);
			glUniform1f(glGetUniformLocation(fp, "spotLight.cutOff"), glm::cos(glm::radians(12.0f)));
			glUniform1f(glGetUniformLocation(fp, "spotLight.outerCutOff"), glm::cos(glm::radians(18.0f)));

			for (int i = 0; i < 25; i++) {
				std::string b = "roofLights[" + std::to_string(i) + "].";
				glUniform3f(glGetUniformLocation(fp, (b + "position").c_str()),
					roofLightPositions[i].x, roofLightPositions[i].y, roofLightPositions[i].z);
				glUniform3f(glGetUniformLocation(fp, (b + "direction").c_str()), 0.0f, -1.0f, 0.0f);
				glUniform3f(glGetUniformLocation(fp, (b + "ambient").c_str()), 0.1f, 0.1f, 0.1f);
				glUniform3f(glGetUniformLocation(fp, (b + "diffuse").c_str()), roofDif.r, roofDif.g, roofDif.b);
				glUniform3f(glGetUniformLocation(fp, (b + "specular").c_str()), roofDif.r, roofDif.g, roofDif.b);
				glUniform1f(glGetUniformLocation(fp, (b + "constant").c_str()), 1.0f);
				glUniform1f(glGetUniformLocation(fp, (b + "linear").c_str()), 0.007f);
				glUniform1f(glGetUniformLocation(fp, (b + "quadratic").c_str()), 0.0002f);
				glUniform1f(glGetUniformLocation(fp, (b + "cutOff").c_str()), glm::cos(glm::radians(45.0f)));
				glUniform1f(glGetUniformLocation(fp, (b + "outerCutOff").c_str()), glm::cos(glm::radians(65.0f)));
			}

			glUniform1f(glGetUniformLocation(fp, "material.shininess"), 32.0f);

			// --- Matrices de transformacion ---
			glUniformMatrix4fv(glGetUniformLocation(fp, "view"), 1, GL_FALSE, glm::value_ptr(view));
			glUniformMatrix4fv(glGetUniformLocation(fp, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
			glm::mat4 flagModel(1);
			glUniformMatrix4fv(glGetUniformLocation(fp, "model"), 1, GL_FALSE, glm::value_ptr(flagModel));
			glUniform1i(glGetUniformLocation(fp, "transparency"), 0);

			// --- Parametros de onda ---
			// Z(x,t) = A * sin(k*x - omega*t) * weight(x)
			glUniform1f(glGetUniformLocation(fp, "time"), currentFrame);
			glUniform1f(glGetUniformLocation(fp, "waveAmp"), 0.18f);
			glUniform1f(glGetUniformLocation(fp, "waveK"), 9.4248f);
			glUniform1f(glGetUniformLocation(fp, "waveOmega"), 2.5f);
			glUniform1f(glGetUniformLocation(fp, "flagXMin"), -0.986661f);
			glUniform1f(glGetUniformLocation(fp, "flagWidth"), 1.0f);
			glUniform1f(glGetUniformLocation(fp, "flagZ"), -40.908188f);

			banderaFlag.Draw(flagShader);
			// ---- Segunda bandera (UNAM Aero Design) ----
			{
				GLuint fp = flagShader.Program;

				glm::mat4 flagModel2(1);
				glUniformMatrix4fv(glGetUniformLocation(fp, "model"), 1, GL_FALSE, glm::value_ptr(flagModel2));

				glUniform1f(glGetUniformLocation(fp, "time"), currentFrame);
				glUniform1f(glGetUniformLocation(fp, "waveAmp"), 0.18f);
				glUniform1f(glGetUniformLocation(fp, "waveK"), 9.4248f);
				glUniform1f(glGetUniformLocation(fp, "waveOmega"), 2.5f);
				glUniform1f(glGetUniformLocation(fp, "flagXMin"), -14.168542f);
				glUniform1f(glGetUniformLocation(fp, "flagWidth"), 1.0f);
				glUniform1f(glGetUniformLocation(fp, "flagZ"), -40.908188f);

				banderaUnam.Draw(flagShader);
			}
			{
			// ---- Tercera bandera (Cisco) ----
			GLuint fp = flagShader.Program;

			glm::mat4 flagModel2(1);
			glUniformMatrix4fv(glGetUniformLocation(fp, "model"), 1, GL_FALSE, glm::value_ptr(flagModel2));

			glUniform1f(glGetUniformLocation(fp, "time"), currentFrame);
			glUniform1f(glGetUniformLocation(fp, "waveAmp"), 0.18f);
			glUniform1f(glGetUniformLocation(fp, "waveK"), 9.4248f);
			glUniform1f(glGetUniformLocation(fp, "waveOmega"), 2.5f);
			glUniform1f(glGetUniformLocation(fp, "flagXMin"), -11.088256f);
			glUniform1f(glGetUniformLocation(fp, "flagWidth"), -0.997640f);
			glUniform1f(glGetUniformLocation(fp, "flagZ"), -15.008704f);

			banderaCisco.Draw(flagShader);
			}
		}
		// ====================================================================


		// Draw the lamp object
		lampShader.Use();
		modelLoc = glGetUniformLocation(lampShader.Program, "model");
		viewLoc = glGetUniformLocation(lampShader.Program, "view");
		projLoc = glGetUniformLocation(lampShader.Program, "projection");

		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

		model = glm::mat4(1);
		model = glm::translate(model, pointLightPositions[0]);
		model = glm::scale(model, glm::vec3(0.2f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);

		// --- DIBUJADO DEL SKYBOX ---
		glDepthFunc(GL_LEQUAL); // Cambia la función de profundidad para asegurar que se dibuje en el fondo

		skyboxShader.Use();

		// Remover la traslación de la matriz de vista para que el skybox siga a la cámara
		glm::mat4 viewSky = glm::mat4(glm::mat3(camera.GetViewMatrix()));
		glUniformMatrix4fv(glGetUniformLocation(skyboxShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(viewSky));
		glUniformMatrix4fv(glGetUniformLocation(skyboxShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		// 1. Enviar el factor de mezcla al shader (sincronizado con la iluminación)
		glUniform1f(glGetUniformLocation(skyboxShader.Program, "blendFactor"), dayFactor);

		// 2. Bindear el cubemap de DÍA a la unidad de textura 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapDay);
		glUniform1i(glGetUniformLocation(skyboxShader.Program, "skyboxDay"), 0);

		// 3. Bindear el cubemap de NOCHE a la unidad de textura 1
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapNight);
		glUniform1i(glGetUniformLocation(skyboxShader.Program, "skyboxNight"), 1);

		// 4. Dibujar el cubo
		glBindVertexArray(skyboxVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);

		glDepthFunc(GL_LESS); // Restaura la función de profundidad original
		// ---------------------------

		// Swap the screen buffers
		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}

// Moves/alters the camera positions based on user input
void DoMovement()
{
	// Camera controls
	if (keys[GLFW_KEY_W] || keys[GLFW_KEY_UP])
		camera.ProcessKeyboard(FORWARD, deltaTime);

	if (keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN])
		camera.ProcessKeyboard(BACKWARD, deltaTime);

	if (keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT])
		camera.ProcessKeyboard(LEFT, deltaTime);

	if (keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT])
		camera.ProcessKeyboard(RIGHT, deltaTime);

	if (keys[GLFW_KEY_T])
		pointLightPositions[0].x += 0.01f;
	if (keys[GLFW_KEY_G])
		pointLightPositions[0].x -= 0.01f;
	if (keys[GLFW_KEY_Y])
		pointLightPositions[0].y += 0.01f;
	if (keys[GLFW_KEY_H])
		pointLightPositions[0].y -= 0.01f;
	if (keys[GLFW_KEY_U])
		pointLightPositions[0].z -= 0.1f;
	if (keys[GLFW_KEY_J])
		pointLightPositions[0].z += 0.01f;
}

// Is called whenever a key is pressed/released via GLFW
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	// Tecla R para alternar (abrir/cerrar) el cartel
	if (key == GLFW_KEY_R && action == GLFW_PRESS)
	{
		bannerOpen = !bannerOpen; // Invierte el estado actual
	}

	if (GLFW_KEY_ESCAPE == key && GLFW_PRESS == action)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	// Toggle Vista de Dia / Vista Nocturna
	if (GLFW_KEY_L == key && GLFW_PRESS == action)
	{
		dayMode = !dayMode;
		std::cout << "[Iluminacion] Modo: " << (dayMode ? "Vista de Dia" : "Vista Nocturna") << std::endl;
	}

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
		{
			keys[key] = true;
		}
		else if (action == GLFW_RELEASE)
		{
			keys[key] = false;
		}
	}

	if (keys[GLFW_KEY_SPACE])
	{
		active = !active;
		if (active)
		{
			Light1 = glm::vec3(1.0f, 1.0f, 0.0f);
		}
		else
		{
			Light1 = glm::vec3(0);
		}
	}
}

void MouseCallback(GLFWwindow* window, double xPos, double yPos)
{
	if (firstMouse)
	{
		lastX = xPos;
		lastY = yPos;
		firstMouse = false;
	}

	GLfloat xOffset = xPos - lastX;
	GLfloat yOffset = lastY - yPos;  // Reversed since y-coordinates go from bottom to left

	lastX = xPos;
	lastY = yPos;

	camera.ProcessMouseMovement(xOffset, yOffset);
}