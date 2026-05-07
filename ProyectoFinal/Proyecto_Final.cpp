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

// Function prototypes
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow* window, double xPos, double yPos);
void DoMovement();

// Window dimensions
const GLuint WIDTH = 800, HEIGHT = 600;
int SCREEN_WIDTH, SCREEN_HEIGHT;

// Camera
Camera  camera(glm::vec3(-5.0f, 3.3f, 5.0f));
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
bool animActive = false;
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
	Model banner((char*)"Models/banner/banner.obj");


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
	glUniform1i(glGetUniformLocation(lightingShader.Program, "Material.difuse"), 0);
	glUniform1i(glGetUniformLocation(lightingShader.Program, "Material.specular"), 1);

	glm::mat4 projection = glm::perspective(camera.GetZoom(), (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT, 0.1f, 100.0f);

	// Game loop
	while (!glfwWindowShouldClose(window))
	{
		// Calculate deltatime of current frame
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glfwPollEvents();
		DoMovement();

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
		glm::vec3 dayDirAmb(0.55f, 0.55f, 0.50f);
		glm::vec3 dayDirDif(0.95f, 0.90f, 0.78f); // tono calido del sol
		glm::vec3 dayDirSpc(0.45f, 0.45f, 0.40f);
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
		// -------------------------------


		// --- LÓGICA DE ANIMACIÓN DEL ROLL-UP ---

		//DIBUJAR LA BASE 
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		tubo.Draw(lightingShader);

		// LÓGICA Y DIBUJO DE LA LONA

		// Solo se calcula y dibuja la lona si la animación ya inició
		if (animActive) {

			if (animProgress < 1.0f) {
				animProgress += deltaTime * 1.0f;
				if (animProgress > 1.0f) animProgress = 1.0f;
			}

			float Sy = easeOutBack(animProgress);

			if (Sy < 0.001f) Sy = 0.001f;

			float Y_TUBO = 4.88f;
			float Ty = ALTURA_LONA * (1.0f - Sy);

			model = glm::mat4(1);
			model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.01f));

			// Transformación anclada arriba
			model = glm::translate(model, glm::vec3(0.0f, Y_TUBO, 0.0f));
			model = glm::scale(model, glm::vec3(1.0f, Sy, 1.0f));
			model = glm::translate(model, glm::vec3(0.0f, -Y_TUBO, 0.0f));

			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			banner.Draw(lightingShader);
		}
		// ----------------------------------------


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
	// Tecla R para reiniciar y lanzar la animación del cartel
	if (keys[GLFW_KEY_R])
	{
		animActive = true;
		animProgress = 0.0f; // Reinicia el contador para verla desde el principio
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