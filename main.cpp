
#define GLEW_STATIC

#include <iostream>
#include "glm/glm.hpp"//core glm functionality
#include "glm/gtc/matrix_transform.hpp"//glm extension for generating common transformation matrices
#include "glm/gtc/matrix_inverse.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "GLEW/glew.h"
#include "GLFW/glfw3.h"
#include <string>
#include "Shader.hpp"
#include "Camera.hpp"
#include<Windows.h>
#include<fstream>
using namespace std;
ifstream fin("coordonate.txt");//coordinates for the pre-estabilished track


#define TINYOBJLOADER_IMPLEMENTATION

#include "Model3D.hpp"
#include "Mesh.hpp"

int glWindowWidth = 2280;
int glWindowHeight = 1280;
//dimension of the camera
int retina_width, retina_height;

GLFWwindow* glWindow = NULL;
GLfloat lastX = glWindowWidth / 2.0;
GLfloat lastY = glWindowHeight / 2.0;

const GLuint SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;

//matrices
glm::mat4 model;
glm::mat4 model2;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;
glm::mat4 lightRotation;
glm::mat3 lightDirMatrix;

//light parameters
glm::vec3 lightDir;
glm::vec3 lightColor;
glm::vec3 newCoordinate;
glm::vec3 lightDir2;
glm::vec3 lightColor2;

//shader
GLuint modelLoc;
GLuint viewLoc;
GLuint projectionLoc;
GLuint normalMatrixLoc;
GLuint lightDirMatrixLoc;

//light direction and color for the sun
GLuint lightDirLoc;
GLuint lightColorLoc;
//light direction and color for the moon
GLuint lightDirLoc2;
GLuint lightColorLoc2;



gps::Camera myCamera(glm::vec3(0.0f, 1.0f, 2.5f),
				     glm::vec3(0.0f, 0.0f, 0.0f));

bool pressedKeys[1024];
GLfloat cameraSpeed = 0.1f;
GLfloat fog = 0.0f;
GLfloat angle;
GLfloat angle2;
GLfloat lightAngle;
GLfloat verticalAngle = 0.0f;
GLfloat horizontalAngle = 0.0f;
GLfloat scalare_ak47 = 1.0f;


gps::Model3D sun;
gps::Model3D moon;

gps::Shader myCustomShader;
gps::Shader lightShader;
gps::Shader depthMapShader;

gps::Model3D scena;

GLuint shadowMapFBO;
GLuint depthMapTexture;

int wireframe = 0, smooth = 0, polygon = 0;
bool moon_bool = 1;
const GLfloat near_plane = 0.1f, far_plane = 5.0f;

GLenum glCheckError_(const char *file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
		case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height)
{
	fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);

	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	myCustomShader.useShaderProgram();

	//set projection matrix
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	//send matrix data to shader
	GLint projLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
	
	lightShader.useShaderProgram();
	
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	//set Viewport transform
	glViewport(0, 0, retina_width, retina_height);
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			pressedKeys[key] = true;
		else if (action == GLFW_RELEASE)
			pressedKeys[key] = false;
	}
}

glm::mat4 computeLightSpaceTrMatrix() {
	//TODO - Return the light-space transformation matrix
	glm::mat4 lightView = glm::lookAt(glm::mat3(lightRotation) * lightDir, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 lightProjection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, near_plane, far_plane);
	glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;
	return lightSpaceTrMatrix;
}

bool firstMouse = true;
void mouseCallback(GLFWwindow* window, double xpos, double ypos)
{

	float pitch = 0;
	float yaw = 0;
	if (firstMouse == false) {
		pitch = (lastY - ypos) * 0.0025;
		yaw = (lastX - xpos) * 0.0025;
	}
	else {
		firstMouse = false;
	}

	lastX = xpos;
	lastY = ypos;

	myCamera.rotate(pitch, yaw);
}


void processMovement()
{

	if (pressedKeys[GLFW_KEY_Q]) {
		angle += 0.2f;
		if (angle > 360.0f)
			angle -= 360.0f;
	}

	if (pressedKeys[GLFW_KEY_E]) {
		angle -= 0.2f;
		if (angle < 0.0f)
			angle += 360.0f;
	}
	if (pressedKeys[GLFW_KEY_LEFT_SHIFT]) {
		cameraSpeed = 0.2;
	}
	if(! pressedKeys[GLFW_KEY_LEFT_SHIFT]) {
		cameraSpeed = 0.1;
	}

	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
	}
	//rotate light dir 1
	if (pressedKeys[GLFW_KEY_J]) {

		lightAngle += 0.3f;
		if (lightAngle > 360.0f)
			lightAngle -= 360.0f;
		glm::vec3 lightDirTr = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightDir, 1.0f));
		myCustomShader.useShaderProgram();
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDirTr));
	}

	if (pressedKeys[GLFW_KEY_L]) {
		lightAngle -= 0.3f; 
		if (lightAngle < 0.0f)
			lightAngle += 360.0f;
		glm::vec3 lightDirTr = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightDir, 1.0f));
		myCustomShader.useShaderProgram();
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDirTr));
	}


	if (pressedKeys[GLFW_KEY_K]) {
		Sleep(300);
		moon_bool = !moon_bool;

	}
	if (pressedKeys[GLFW_KEY_I])
	{
		Sleep(300);
		wireframe = !wireframe;
		if (wireframe)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	if (pressedKeys[GLFW_KEY_O])
	{	
		Sleep(300);
		smooth = !smooth;
		if (smooth)
			glPolygonMode(GL_FRONT_AND_BACK, GL_SMOOTH);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	if (pressedKeys[GLFW_KEY_P])
	{
		Sleep(300);
		polygon = !polygon;
		if (polygon)
			glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	if (pressedKeys[GLFW_KEY_F])
	{
		Sleep(300);
		if (fog == 0)
		{
			myCustomShader.useShaderProgram();
			fog = 1.0f;
			glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "fog"), fog);

		}
		else
		{
			myCustomShader.useShaderProgram();
			fog = 0.0f;
			glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "fog"), fog);

		}
		
	}
	if (pressedKeys[GLFW_KEY_LEFT_CONTROL]) {
		glm::mat4 matrix = myCamera.getViewMatrix();
		glm::mat3 rotationMat(matrix);
		glm::vec3 matrix2(matrix[3]);
		glm::vec3 rotationVec = -matrix2 * rotationMat;

		if (rotationVec != newCoordinate)
		{
			newCoordinate = rotationVec;

			cout << myCamera.getCameraDirection().x << " " << myCamera.getCameraDirection().y << " " << myCamera.getCameraDirection().z << " ";
			cout << myCamera.getCameraPosition().x << " " << myCamera.getCameraPosition().y << " " << myCamera.getCameraPosition().z << std::endl;

		}
	}
	if (pressedKeys[GLFW_KEY_Y]) {

		float x, y, z, directionX, directionY, directionZ;
		if (fin.eof()==0) {
			fin >> x >> y >> z >> directionX >> directionY >> directionZ;
			myCamera.move(glm::vec3(directionX, directionY, directionZ), glm::vec3(x, y, z), 0.2);
		}
	}

}

bool initOpenGLWindow()
{
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

	glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "Afganistan 1998", NULL, NULL);
	if (!glWindow) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}

	glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
	glfwMakeContextCurrent(glWindow);

	// start GLEW extension handler
	glewExperimental = GL_TRUE;
	glewInit();

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	glfwSetKeyCallback(glWindow, keyboardCallback);
	glfwSetCursorPosCallback(glWindow, mouseCallback);

	glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	return true;
}

void initOpenGLState()
{
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	glViewport(0, 0, retina_width, retina_height);
    glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
}

void initFBOs()
{
	//generate FBO ID
	glGenFramebuffers(1, &shadowMapFBO);

	//create depth texture for FBO
	glGenTextures(1, &depthMapTexture);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
	SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	//attach texture to FBO
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void initModels()
{
	scena = gps::Model3D("objects/scena/tot.obj", "objects/scena/");
	sun = gps::Model3D("objects/scena/sun1.obj", "objects/scena/");
	moon = gps::Model3D("objects/scena/moon1.obj", "objects/scena/");
}

void initShaders()
{
	myCustomShader.loadShader("shaders/shaderStart.vert", "shaders/shaderStart.frag");
	depthMapShader.loadShader("shaders/simpleDepthMap.vert", "shaders/simpleDepthMap.frag");
}

void initUniforms()
{
	myCustomShader.useShaderProgram();
	model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
	modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");

	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");
	// send view matrix to shader
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view)); 

	viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");
	
	//set the light direction (direction towards the light)
	lightDir = glm::vec3(49.79f,82.21f, 150.55f);
	lightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDir");
	lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(angle2), glm::vec3(0.0f, 1.0f, 0.0f));
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
	lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor");
	
	// send light color to shader
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

	//set the light direction2 (direction towards the light)
	lightDir2 = glm::vec3(73.187f, 72.84f, -100.28f);
	lightDirLoc2 = glGetUniformLocation(myCustomShader.shaderProgram, "lightDir2");
	// send light dir2 to shader
	glUniform3fv(lightDirLoc2, 1, glm::value_ptr(lightDir2));

	//set light color2
	lightColor2 = glm::vec3(1.0f, 0.0f, 0.0f); //red light
	lightColorLoc2 = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor2");
	// send light color2 to shader
	glUniform3fv(lightColorLoc2, 1, glm::value_ptr(lightColor2));

	normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");
	
	lightDirMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDirMatrix");

	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));	


	lightShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
}
void renderMoon(gps::Shader shader)
{
	shader.useShaderProgram();
		model2 = glm::mat4(1.0);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model2));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		moon.Draw(shader);
	
}
void renderSun(gps::Shader shader) {
	shader.useShaderProgram();
	lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(angle2), glm::vec3(0.0f, 1.0f, 0.0f));
	lightDirLoc = glGetUniformLocation(shader.shaderProgram, "lightDir");
	// send light dir to shader
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));


	model2 = glm::mat4(1.0f);
	model2 = glm::rotate(model2, glm::radians(angle2), glm::vec3(0, 1, 0));//se invarte in jurul centrul scenei
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model2));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	sun.Draw(shader);
}
void renderScene()
{
	angle2 += 0.1f;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	processMovement();	

	//render the scene to the depth buffer (first pass)

	depthMapShader.useShaderProgram();

	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"),
		1,
		GL_FALSE,
		glm::value_ptr(computeLightSpaceTrMatrix()));
		
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	
	scena.Draw(depthMapShader);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//render the scene (second pass)

	myCustomShader.useShaderProgram();

	//send lightSpace matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"),
		1,
		GL_FALSE,
		glm::value_ptr(computeLightSpaceTrMatrix()));

	//send view matrix to shader
	view = myCamera.getViewMatrix();
	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "view"),
		1,
		GL_FALSE,
		glm::value_ptr(view));	

	//compute light direction transformation matrix
	lightDirMatrix = glm::mat3(glm::inverseTranspose(view));
	//send lightDir matrix data to shader
	glUniformMatrix3fv(lightDirMatrixLoc, 1, GL_FALSE, glm::value_ptr(lightDirMatrix));

	glViewport(0, 0, retina_width, retina_height);
	myCustomShader.useShaderProgram();

	//bind the depth map
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 3);
	
	//create model matrix for nanosuit
	model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	//compute normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	scena.Draw(myCustomShader);
		
	//create model matrix for ground
	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	//send model matrix data to shader
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	//draw a white cube around the light

	lightShader.useShaderProgram();

	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

	model = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::translate(model, 1.0f * lightDir);
	model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	renderSun(myCustomShader);
	
	if (moon_bool == 1) {
		lightColor2 = glm::vec3(1.0f, 0.0f, 0.0f); //red light
		lightColorLoc2 = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor2");
		// send light color to shader
		glUniform3fv(lightColorLoc2, 1, glm::value_ptr(lightColor2));
		renderMoon(myCustomShader);
	}
	else {
		lightColor2 = glm::vec3(0.0f, 0.0f, 0.0f);
		lightColorLoc2 = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor2");
		// send light color to shader
		glUniform3fv(lightColorLoc2, 1, glm::value_ptr(lightColor2));
	}
}

int main(int argc, const char * argv[]) {

	initOpenGLWindow();
	initOpenGLState();
	initFBOs();
	initModels();
	initShaders();
	initUniforms();	
	glCheckError();
	while (!glfwWindowShouldClose(glWindow)) {
		renderScene();

		glfwPollEvents();
		glfwSwapBuffers(glWindow);
	}

	//close GL context and any other GLFW resources
	glfwTerminate();

	return 0;
}
