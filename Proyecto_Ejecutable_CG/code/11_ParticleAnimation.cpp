/*
* Laboratorio de Computación Gráfica e Interacción Humano-Computadora
* 11 - Animación de Partículas
*/

#include <iostream>
#include <stdlib.h>

// GLAD: Multi-Language GL/GLES/EGL/GLX/WGL Loader-Generator
// https://glad.dav1d.de/
#include <glad/glad.h>

// GLFW: https://www.glfw.org/
#include <GLFW/glfw3.h>

// GLM: OpenGL Math library
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Model loading classes
#include <shader_m.h>
#include <camera.h>
#include <model.h>
#include <material.h>
#include <light.h>
#include <cubemap.h>
#include <particles.h>
#include <irrKlang.h>

using namespace irrklang;

// Max number of bones
#define MAX_RIGGING_BONES 100

// Functions
bool Start();
bool Update();

// Definición de callbacks
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

// Gobals
GLFWwindow* window;

// Tamaño en pixeles de la ventana
const unsigned int SCR_WIDTH = 1024;
const unsigned int SCR_HEIGHT = 768;

// Definición de cámara (posición en XYZ)
Camera camera(glm::vec3(0.0f, 9.0f, 15.0f));
Camera camera3rd(glm::vec3(0.0f, 0.0f, 0.0f));

// Controladores para el movimiento del mouse
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// Variables para la velocidad de reproducción
// de la animación
float deltaTime = 0.0f;//Era igual a 0
float lastFrame = 0.0f;
float elapsedTime = 0.0f;

//Variables para el movimiento del personaje
float     rotateCharacter = 180.0f;
glm::vec3 position(0.0f, 0.0f, 0.0f);
glm::vec3 forwardView(0.0f, 0.0f, 00.0f);

// Shaders
Shader *ourShader;//Para el personaje
Shader *staticShader;//Para objetos estaticos
Shader *cubemapShader;
Shader *particlesShader;//Para las particulas
Shader *wavesShader;// Para el agua
Shader *proceduralShader;//Para la luna
Shader *phongShader;

// Carga la información del modelo
Model   *character;//Para el personaje
Model   *moon;//Para la luna
Model   *final;
Model   *animal;
Model   *anuncio1;
Model   *anuncio2;
Model   *anuncio3;
Model   *anuncio4;
Model   *anuncio5;
Model	*jeep;
Model	*llantas;

float tradius = 10.0f;
float theta = 0.0f;
float alpha = 0.0f;
//Mover carro
float carro = 0.0f;
bool mov_carro;
//Mover ratas
float rata1 = -285.0f;
float rata2 = -150.0f;
float rata3 = -620.0f;
float rata4 = -340.0f;

// Cubemap
CubeMap *mainCubeMap;

// Light gLight;
std::vector<Light> gLights;
Light    lightMoon;

// Pose inicial del modelo
glm::mat4 gBones[MAX_RIGGING_BONES];
glm::mat4 gBonesBar[MAX_RIGGING_BONES];


float	fps = 0.0f;//30.0
int		keys = 0;
int		animationCount = 0;
float proceduralTime = 0.0f;
float wavesTime = 0.0f;

// Materiales
Material material01;
Light light01;

// Audio
ISoundEngine *SoundEngine = createIrrKlangDevice();

// selección de cámara
bool    activeCamera = 1; // activamos la primera cámara

// Entrada a función principal
int main()
{
	if (!Start())
		return -1;

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		if (!Update())
			break;
	}

	glfwTerminate();
	return 0;

}

bool Start() {
	// Inicialización de GLFW

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Creación de la ventana con GLFW
	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "FBX Animation with OpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// Ocultar el cursor mientras se rota la escena
	// glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: Cargar todos los apuntadores
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return false;
	}

	// Activación de buffer de profundidad
	glEnable(GL_DEPTH_TEST);

	// Compilación y enlace de shaders
	ourShader = new Shader("bin/shaders/10_vertex_skinning-IT.vs", "bin/shaders/10_fragment_skinning-IT.fs");
	cubemapShader = new Shader("bin/shaders/10_vertex_cubemap.vs", "bin/shaders/10_fragment_cubemap.fs");
	particlesShader = new Shader("bin/shaders/13_particles.vs", "bin/shaders/13_particles.fs");
	staticShader = new Shader("bin/shaders/10_vertex_simple.vs", "bin/shaders/10_fragment_simple.fs");//Para objetos estaticos
	wavesShader = new Shader("bin/shaders/13_wavesAnimation.vs", "bin/shaders/13_wavesAnimation.fs");//Para el agua
	proceduralShader = new Shader("bin/shaders/12_ProceduralAnimation.vs", "bin/shaders/12_ProceduralAnimation.fs");
	phongShader = new Shader("bin/shaders/11_BasicPhongShader.vs", "bin/shaders/11_BasicPhongShader.fs");
	
	// Máximo número de huesos: 100
	ourShader->setBonesIDs(MAX_RIGGING_BONES);//agregado de la p6

	//Carga de modelos
	character = new Model("bin/models/animacion.fbx");
	animal = new Model("bin/models/source/Rat_MultiAnims_Textured.fbx");
	moon = new Model("bin/models/IllumModels/moon.fbx");
	final = new Model("bin/models/Escenario/Final2.fbx");
	anuncio1 = new Model("bin/models/anuncio_basura.fbx");
	anuncio2 = new Model("bin/models/anuncio_coches.fbx");
	anuncio3= new Model("bin/models/anuncio_enfermedades.fbx");
	anuncio4 = new Model("bin/models/anuncio_flora.fbx");
	anuncio5 = new Model("bin/models/anuncio_paneles.fbx");
	jeep = new Model("bin/models/Jeep.fbx");
	llantas = new Model("bin/models/Llantas.fbx");
	
	// Cubemap
	vector<std::string> faces
	{
		"bin/textures/cubemap/03/posx.jpg",
		"bin/textures/cubemap/03/negx.jpg",
		"bin/textures/cubemap/03/posy.jpg",
		"bin/textures/cubemap/03/negy.jpg",
		"bin/textures/cubemap/03/posz.jpg",
		"bin/textures/cubemap/03/negz.jpg"
	};
	mainCubeMap = new CubeMap();
	mainCubeMap->loadCubemap(faces);

	camera3rd.Position = position;
	camera3rd.Position.y += 1.7f;
	camera3rd.Position -= forwardView;
	camera3rd.Front = forwardView;

	SoundEngine->play2D("bin/sound/ciudad.mp3", true);

	// time, arrays
	character->SetPose(0.0f, gBones);

	fps = (float)character->getFramerate();
	keys = (int)character->getNumFrames();

	return true;
}

bool Update() {
	// Cálculo del framerate
	float currentFrame = (float)glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;

	//MOVER CARRO
	if (mov_carro) {
		if (carro > -1200.0) {
			carro -= 1.0;
		}
		else {
			carro = 0.0;
			mov_carro = false;
		}
	}

	//MOVER RATAS
	if (rata1 < -150.0) {
		rata1 += 0.7;
	}
	else {
		rata1 = -285.0;
	}
	if (rata2 > -275.0) {
		rata2 -= 0.7;
	}
	else {
		rata2 = -150.0;
	}
	if (rata3 < -340.0) {
		rata3 += 0.7;
	}
	else {
		rata3 = -620.0;
	}
	if (rata4 > -630.0) {
		rata4 -= 0.6;
	}
	else {
		rata4 = -340.0;
	}

	elapsedTime += deltaTime;
	if (elapsedTime > 1.0f / fps) {
		animationCount++;
		if (animationCount > keys - 1) {
			animationCount = 0;
		}
		// Configuración de la pose en el instante t
		character->SetPose((float)animationCount, gBones);
		elapsedTime = 0.0f;
	}

	// Procesa la entrada del teclado o mouse
	processInput(window);

	// Renderizado R - G - B - A
	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	 
	// Cubemap (fondo)
	{
		glm::mat4 projection;
		glm::mat4 view;

		if (activeCamera) {
			projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
			view = camera.GetViewMatrix();
		}
		else {
			projection = glm::perspective(glm::radians(camera3rd.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
			view = camera3rd.GetViewMatrix();
		}
		
		mainCubeMap->drawCubeMap(*cubemapShader, projection, view);
	}
	//Para la luna
	{
		// Activamos el shader de Phong
		proceduralShader->use();

		// Activamos para objetos transparentes
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Aplicamos transformaciones de proyección y cámara (si las hubiera)
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
		glm::mat4 view = camera.GetViewMatrix();
		proceduralShader->setMat4("projection", projection);
		proceduralShader->setMat4("view", view);

		// Aplicamos transformaciones del modelo
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 40.0f, -250.0f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(15.0f, 15.0f, 15.0f));
		proceduralShader->setMat4("model", model);

		proceduralShader->setFloat("time", proceduralTime);
		proceduralShader->setFloat("radius", 100.0f);
		proceduralShader->setFloat("height", 45.0f);

		moon->Draw(*proceduralShader);
		proceduralTime += 0.0005;//0.01;

	}
	glUseProgram(0);

	
	//RATA1
	{
		// Activación del shader del personaje
		ourShader->use();

		// Aplicamos transformaciones de proyección y cámara (si las hubiera)
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
		glm::mat4 view = camera.GetViewMatrix();
		ourShader->setMat4("projection", projection);
		ourShader->setMat4("view", view);

		// Aplicamos transformaciones del modelo
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(68.0f, 4.6f, rata1)); //X (ancho camellon -> + derecha, - izquierda), Y (+ arriba, -abajo), (Z largo del camellon -> +atras, -adelante)  
		model = glm::translate(model, position); // translate it down so it's at the center of the scene
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.01f, 0.01f, 0.01f));	// it's a bit too big for our scene, so scale it down

		ourShader->setMat4("model", model);
		ourShader->setMat4("gBones", MAX_RIGGING_BONES, gBones);

		// Dibujamos el modelo
		character->Draw(*ourShader);
	}
	glUseProgram(0);
	//RATA2
	{
		// Activación del shader del personaje
		ourShader->use();

		// Aplicamos transformaciones de proyección y cámara (si las hubiera)
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
		glm::mat4 view = camera.GetViewMatrix();
		ourShader->setMat4("projection", projection);
		ourShader->setMat4("view", view);

		// Aplicamos transformaciones del modelo
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(56.0f, 4.6f, rata2)); //X (ancho camellon -> + derecha, - izquierda), Y (+ arriba, -abajo), (Z largo del camellon -> +atras, -adelante)  
		model = glm::translate(model, position); // translate it down so it's at the center of the scene
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.01f, 0.01f, 0.01f));	// it's a bit too big for our scene, so scale it down

		ourShader->setMat4("model", model);
		ourShader->setMat4("gBones", MAX_RIGGING_BONES, gBones);

		// Dibujamos el modelo
		character->Draw(*ourShader);
	}
	glUseProgram(0);
	//RATA3
	{
		// Activación del shader del personaje
		ourShader->use();

		// Aplicamos transformaciones de proyección y cámara (si las hubiera)
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
		glm::mat4 view = camera.GetViewMatrix();
		ourShader->setMat4("projection", projection);
		ourShader->setMat4("view", view);

		// Aplicamos transformaciones del modelo
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(67.0f, 4.6f, rata3)); //X (ancho camellon -> + derecha, - izquierda), Y (+ arriba, -abajo), (Z largo del camellon -> +atras, -adelante)  
		model = glm::translate(model, position); // translate it down so it's at the center of the scene
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.01f, 0.01f, 0.01f));	// it's a bit too big for our scene, so scale it down

		ourShader->setMat4("model", model);
		ourShader->setMat4("gBones", MAX_RIGGING_BONES, gBones);

		// Dibujamos el modelo
		character->Draw(*ourShader);
	}
	glUseProgram(0);
	//RATA4
	{
		// Activación del shader del personaje
		ourShader->use();

		// Aplicamos transformaciones de proyección y cámara (si las hubiera)
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
		glm::mat4 view = camera.GetViewMatrix();
		ourShader->setMat4("projection", projection);
		ourShader->setMat4("view", view);

		// Aplicamos transformaciones del modelo
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(63.0f, 4.6f, rata4)); //X (ancho camellon -> + derecha, - izquierda), Y (+ arriba, -abajo), (Z largo del camellon -> +atras, -adelante)  
		model = glm::translate(model, position); // translate it down so it's at the center of the scene
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.01f, 0.01f, 0.01f));	// it's a bit too big for our scene, so scale it down

		ourShader->setMat4("model", model);
		ourShader->setMat4("gBones", MAX_RIGGING_BONES, gBones);

		// Dibujamos el modelo
		character->Draw(*ourShader);
	}
	glUseProgram(0);


	{
		//	""DATOS DEL ESCENARIO"
		// Activamos el shader del plano
		phongShader->use();

		// Activamos para objetos transparentes
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Aplicamos transformaciones de proyección y cámara (si las hubiera)
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
		glm::mat4 view = camera.GetViewMatrix();
		phongShader->setMat4("projection", projection);
		phongShader->setMat4("view", view);

		// Aplicamos transformaciones del modelo
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
		//model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// primera->profundidad, segunda->altura(C), tercera->anchura
		phongShader->setMat4("model", model);

		// Configuramos propiedades de fuentes de luz
		phongShader->setVec4("LightColor", light01.Color);
		phongShader->setVec4("LightPower", light01.Power);
		phongShader->setInt("alphaIndex", light01.alphaIndex);
		phongShader->setFloat("distance", light01.distance);
		phongShader->setVec3("lightPosition", light01.Position);
		phongShader->setVec3("lightDirection", light01.Direction);
		phongShader->setVec3("eye", camera.Position);

		// Aplicamos propiedades materiales
		phongShader->setVec4("MaterialAmbientColor", material01.ambient);
		phongShader->setVec4("MaterialDiffuseColor", material01.diffuse);
		phongShader->setVec4("MaterialSpecularColor", material01.specular);
		phongShader->setFloat("transparency", material01.transparency);

		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);

		final->Draw(*phongShader);
	}
	glUseProgram(0);
	
	{
		//	""DATOS DEL ANUNCIO1"
		// Activamos el shader del plano
		phongShader->use();

		// Activamos para objetos transparentes
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Aplicamos transformaciones de proyección y cámara (si las hubiera)
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
		glm::mat4 view = camera.GetViewMatrix();
		phongShader->setMat4("projection", projection);
		phongShader->setMat4("view", view);

		// Aplicamos transformaciones del modelo
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(95.891f, 9.068f, -64.638f)); // translate it down so it's at the center of the scene
		//model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(5.0f, 5.0f, 5.0f));	// primera->profundidad, segunda->altura(C), tercera->anchura
		phongShader->setMat4("model", model);

		// Configuramos propiedades de fuentes de luz
		phongShader->setVec4("LightColor", light01.Color);
		phongShader->setVec4("LightPower", light01.Power);
		phongShader->setInt("alphaIndex", light01.alphaIndex);
		phongShader->setFloat("distance", light01.distance);
		phongShader->setVec3("lightPosition", light01.Position);
		phongShader->setVec3("lightDirection", light01.Direction);
		phongShader->setVec3("eye", camera.Position);

		// Aplicamos propiedades materiales
		phongShader->setVec4("MaterialAmbientColor", material01.ambient);
		phongShader->setVec4("MaterialDiffuseColor", material01.diffuse);
		phongShader->setVec4("MaterialSpecularColor", material01.specular);
		phongShader->setFloat("transparency", material01.transparency);

		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);

		anuncio1->Draw(*phongShader);
	}
	glUseProgram(0);

	{
		//	""DATOS DEL ANUNCIO2"
		// Activamos el shader del plano
		phongShader->use();

		// Activamos para objetos transparentes
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Aplicamos transformaciones de proyección y cámara (si las hubiera)
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
		glm::mat4 view = camera.GetViewMatrix();
		phongShader->setMat4("projection", projection);
		phongShader->setMat4("view", view);

		// Aplicamos transformaciones del modelo
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-25.298f, 9.528f, -20.371f)); // translate it down so it's at the center of the scene
		//model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(5.0f, 5.0f, 5.0f));	// primera->profundidad, segunda->altura(C), tercera->anchura
		phongShader->setMat4("model", model);

		// Configuramos propiedades de fuentes de luz
		phongShader->setVec4("LightColor", light01.Color);
		phongShader->setVec4("LightPower", light01.Power);
		phongShader->setInt("alphaIndex", light01.alphaIndex);
		phongShader->setFloat("distance", light01.distance);
		phongShader->setVec3("lightPosition", light01.Position);
		phongShader->setVec3("lightDirection", light01.Direction);
		phongShader->setVec3("eye", camera.Position);

		// Aplicamos propiedades materiales
		phongShader->setVec4("MaterialAmbientColor", material01.ambient);
		phongShader->setVec4("MaterialDiffuseColor", material01.diffuse);
		phongShader->setVec4("MaterialSpecularColor", material01.specular);
		phongShader->setFloat("transparency", material01.transparency);

		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);

		anuncio2->Draw(*phongShader);
	}
	glUseProgram(0);

	{
		//	""DATOS DEL ANUNCIO3"
		// Activamos el shader del plano
		phongShader->use();

		// Activamos para objetos transparentes
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Aplicamos transformaciones de proyección y cámara (si las hubiera)
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
		glm::mat4 view = camera.GetViewMatrix();
		phongShader->setMat4("projection", projection);
		phongShader->setMat4("view", view);

		// Aplicamos transformaciones del modelo
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-420.349f, 10.902f, -66.314f)); // translate it down so it's at the center of the scene
		//model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(5.0f, 5.0f, 5.0f));	// primera->profundidad, segunda->altura(C), tercera->anchura
		phongShader->setMat4("model", model);

		// Configuramos propiedades de fuentes de luz
		phongShader->setVec4("LightColor", light01.Color);
		phongShader->setVec4("LightPower", light01.Power);
		phongShader->setInt("alphaIndex", light01.alphaIndex);
		phongShader->setFloat("distance", light01.distance);
		phongShader->setVec3("lightPosition", light01.Position);
		phongShader->setVec3("lightDirection", light01.Direction);
		phongShader->setVec3("eye", camera.Position);

		// Aplicamos propiedades materiales
		phongShader->setVec4("MaterialAmbientColor", material01.ambient);
		phongShader->setVec4("MaterialDiffuseColor", material01.diffuse);
		phongShader->setVec4("MaterialSpecularColor", material01.specular);
		phongShader->setFloat("transparency", material01.transparency);

		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);

		anuncio3->Draw(*phongShader);
	}
	glUseProgram(0);

	{
		//	""DATOS DEL ANUNCIO4"
		// Activamos el shader del plano
		phongShader->use();

		// Activamos para objetos transparentes
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Aplicamos transformaciones de proyección y cámara (si las hubiera)
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
		glm::mat4 view = camera.GetViewMatrix();
		phongShader->setMat4("projection", projection);
		phongShader->setMat4("view", view);

		// Aplicamos transformaciones del modelo
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-180.246f, 11.975f, -7.14f)); // translate it down so it's at the center of the scene
		//model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(5.0f, 5.0f, 5.0f));	// primera->profundidad, segunda->altura(C), tercera->anchura
		phongShader->setMat4("model", model);

		// Configuramos propiedades de fuentes de luz
		phongShader->setVec4("LightColor", light01.Color);
		phongShader->setVec4("LightPower", light01.Power);
		phongShader->setInt("alphaIndex", light01.alphaIndex);
		phongShader->setFloat("distance", light01.distance);
		phongShader->setVec3("lightPosition", light01.Position);
		phongShader->setVec3("lightDirection", light01.Direction);
		phongShader->setVec3("eye", camera.Position);

		// Aplicamos propiedades materiales
		phongShader->setVec4("MaterialAmbientColor", material01.ambient);
		phongShader->setVec4("MaterialDiffuseColor", material01.diffuse);
		phongShader->setVec4("MaterialSpecularColor", material01.specular);
		phongShader->setFloat("transparency", material01.transparency);

		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);

		anuncio4->Draw(*phongShader);
	}
	glUseProgram(0);
	{
		//	""DATOS DEL ANUNCIO5"
		// Activamos el shader del plano
		phongShader->use();

		// Activamos para objetos transparentes
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Aplicamos transformaciones de proyección y cámara (si las hubiera)
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
		glm::mat4 view = camera.GetViewMatrix();
		phongShader->setMat4("projection", projection);
		phongShader->setMat4("view", view);

		// Aplicamos transformaciones del modelo
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-589.126f, 10.454f, 59.111f)); // translate it down so it's at the center of the scene
		//model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(5.0f, 5.0f, 5.0f));	// primera->profundidad, segunda->altura(C), tercera->anchura
		phongShader->setMat4("model", model);

		// Configuramos propiedades de fuentes de luz
		phongShader->setVec4("LightColor", light01.Color);
		phongShader->setVec4("LightPower", light01.Power);
		phongShader->setInt("alphaIndex", light01.alphaIndex);
		phongShader->setFloat("distance", light01.distance);
		phongShader->setVec3("lightPosition", light01.Position);
		phongShader->setVec3("lightDirection", light01.Direction);
		phongShader->setVec3("eye", camera.Position);

		// Aplicamos propiedades materiales
		phongShader->setVec4("MaterialAmbientColor", material01.ambient);
		phongShader->setVec4("MaterialDiffuseColor", material01.diffuse);
		phongShader->setVec4("MaterialSpecularColor", material01.specular);
		phongShader->setFloat("transparency", material01.transparency);

		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);

		anuncio5->Draw(*phongShader);
	}
	glUseProgram(0);


	{
		//	""DATOS DEL JEEP"
		// Activamos el shader del plano
		phongShader->use();

		// Activamos para objetos transparentes
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Aplicamos transformaciones de proyección y cámara (si las hubiera)
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
		glm::mat4 view = camera.GetViewMatrix();
		phongShader->setMat4("projection", projection);
		phongShader->setMat4("view", view);

		// Aplicamos transformaciones del modelo
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, carro)); // translate it down so it's at the center of the scene
		//model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// primera->profundidad, segunda->altura(C), tercera->anchura
		phongShader->setMat4("model", model);

		// Configuramos propiedades de fuentes de luz
		phongShader->setVec4("LightColor", light01.Color);
		phongShader->setVec4("LightPower", light01.Power);
		phongShader->setInt("alphaIndex", light01.alphaIndex);
		phongShader->setFloat("distance", light01.distance);
		phongShader->setVec3("lightPosition", light01.Position);
		phongShader->setVec3("lightDirection", light01.Direction);
		phongShader->setVec3("eye", camera.Position);

		// Aplicamos propiedades materiales
		phongShader->setVec4("MaterialAmbientColor", material01.ambient);
		phongShader->setVec4("MaterialDiffuseColor", material01.diffuse);
		phongShader->setVec4("MaterialSpecularColor", material01.specular);
		phongShader->setFloat("transparency", material01.transparency);

		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);

		jeep->Draw(*phongShader);
		llantas->Draw(*phongShader);
	}
	glUseProgram(0);


	// glfw: swap buffers 
	glfwSwapBuffers(window);
	glfwPollEvents();

	return true;
}

// Procesamos entradas del teclado
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
		
	// camera movement
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {

		position = position + 0.05F * forwardView;
		camera3rd.Front = forwardView;
		camera3rd.ProcessKeyboard(FORWARD, deltaTime);
		camera3rd.Position = position;
		camera3rd.Position.y += 1.7f;
		camera3rd.Position -= forwardView;

	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		position = position - 0.05F * forwardView;
		camera3rd.Front = forwardView;
		camera3rd.ProcessKeyboard(BACKWARD, deltaTime);
		camera3rd.Position = position;
		camera3rd.Position.y += 1.7f;
		camera3rd.Position -= forwardView;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
		rotateCharacter += 0.5f;

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::rotate(model, glm::radians(rotateCharacter), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::vec4 viewVector = model * glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
		forwardView = glm::vec3(viewVector);
		forwardView = glm::normalize(forwardView);

		camera3rd.Front = forwardView;
		camera3rd.Position = position;
		camera3rd.Position.y += 1.7f;
		camera3rd.Position -= forwardView;
	}
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		rotateCharacter -= 0.5f;

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::rotate(model, glm::radians(rotateCharacter), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::vec4 viewVector = model * glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
		forwardView = glm::vec3(viewVector);
		forwardView = glm::normalize(forwardView);

		camera3rd.Front = forwardView;
		camera3rd.Position = position;
		camera3rd.Position.y += 1.7f;
		camera3rd.Position -= forwardView;
	}

	if (glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS)
		activeCamera = 0;
	if (glfwGetKey(window, GLFW_KEY_F2) == GLFW_PRESS)
		activeCamera = 1;

	if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
		light01.Position.x += 10.0;
	if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
		light01.Position.x -= 10.0;
	if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
		light01.Position.y += 10.0;
	if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS)
		light01.Position.y -= 10.0;
	if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
		light01.Position.z += 10.0;
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
		light01.Position.z -= 10.0;

	if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
		mov_carro = !mov_carro;
		//printf("\n mov_carro: %d", mov_carro);
	}
}

// glfw: Actualizamos el puerto de vista si hay cambios del tamaño
// de la ventana
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

// glfw: Callback del movimiento y eventos del mouse
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = (float)xpos;
		lastY = (float)ypos;
		firstMouse = false;
	}

	float xoffset = (float)xpos - lastX;
	float yoffset = lastY - (float)ypos; 

	lastX = (float)xpos;
	lastY = (float)ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: Complemento para el movimiento y eventos del mouse
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll((float)yoffset);
}
