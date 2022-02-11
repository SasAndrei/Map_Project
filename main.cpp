#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "glm/glm.hpp" //core glm functionality
#include "glm/gtc/matrix_transform.hpp" //glm extension for generating common transformation matrices
#include "glm/gtc/matrix_inverse.hpp" //glm extension for computing inverse matrices
#include "glm/gtc/type_ptr.hpp" //glm extension for accessing the internal data structure of glm types

#include "Window.h"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"
#include "SkyBox.hpp"

#include <iostream>

// window
gps::Window myWindow;

// matrices
glm::mat4 model;
glm::mat4 carModel;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;

// light parameters
glm::vec3 lightDir;
glm::vec3 colorDir;

glm::vec3 fog;

glm::vec3 diffuseDir;
glm::vec3 specularDir;

glm::vec3 positionSpot;
glm::vec3 directionSpot;
GLfloat cutOffSpot;
GLfloat outerCutOffSpot;

GLfloat constantSpot;
GLfloat linearSpot;
GLfloat quadraticSpot;

glm::vec3 ambientSpot;
glm::vec3 diffuseSpot;
glm::vec3 specularSpot;

// shader uniform locations
GLint modelLoc;
GLint carLoc;
GLint viewLoc;
GLint projectionLoc;
GLint normalMatrixLoc;

GLint lightDirLoc;
GLint colorDirLoc;

GLint fogLoc;
GLint pointLoc;

GLint diffuseDirLoc;
GLint specularDirLoc;

GLint positionSpotLoc;
GLint directionSpotLoc;
GLint cutOffSpotLoc;
GLint outerCutOffSpotLoc;

GLint constantSpotLoc;
GLint linearSpotLoc;
GLint quadraticSpotLoc;

GLint ambientSpotLoc;
GLint diffuseSpotLoc;
GLint specularSpotLoc;

// camera
gps::Camera myCamera(
    glm::vec3(0.0f, 100.0f, 3.0f),
    glm::vec3(0.0f, 0.0f, -10.0f),
    glm::vec3(0.0f, 50.0f, 0.0f));

GLfloat cameraSpeed = 10.0f;

gps::SkyBox mySkyBox;
gps::Shader skyboxShader;

GLboolean pressedKeys[1024];

bool firstMouse = true;
float lastX = 800.0f / 2.0;
float lastY = 600.0 / 2.0;
float fov = 23.0f;
float offset = 0.0f;
int direction = 1;
float rotation = 0.0f;

bool animation = false;
int animationPoint = 0;
int count = 0;

const unsigned int SHADOW_WIDTH = 2048;
const unsigned int SHADOW_HEIGHT = 2048;
bool shadow = false;

// models
gps::Model3D car;
gps::Model3D map;
gps::Model3D screenQuad;
GLfloat angle;

// shaders
gps::Shader myBasicShader;
gps::Shader carShader;
gps::Shader depthMapShader;
gps::Shader screenQuadShader;

GLuint shadowMapFBO;
GLuint depthMapTexture;

GLenum glCheckError_(const char *file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR) {
		std::string error;
		switch (errorCode) {
            case GL_INVALID_ENUM:
                error = "INVALID_ENUM";
                break;
            case GL_INVALID_VALUE:
                error = "INVALID_VALUE";
                break;
            case GL_INVALID_OPERATION:
                error = "INVALID_OPERATION";
                break;
            case GL_STACK_OVERFLOW:
                error = "STACK_OVERFLOW";
                break;
            case GL_STACK_UNDERFLOW:
                error = "STACK_UNDERFLOW";
                break;
            case GL_OUT_OF_MEMORY:
                error = "OUT_OF_MEMORY";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                error = "INVALID_FRAMEBUFFER_OPERATION";
                break;
        }
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

	if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            pressedKeys[key] = true;
        } else if (action == GLFW_RELEASE) {
            pressedKeys[key] = false;
        }
    }
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    if (animation == false) {
        if (pressedKeys[GLFW_KEY_R])
            glfwSetInputMode(myWindow.getWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        else
            glfwSetInputMode(myWindow.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        if (firstMouse || pressedKeys[GLFW_KEY_R])
        {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }

        float xoffset = xpos - lastX;
        float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
        lastX = xpos;
        lastY = ypos;

        float sensitivity = 0.1f; // change this value to your liking
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        myCamera.rotate(yoffset, xoffset);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }
}

void processMovement() {
    if (animation == false) {
        if (pressedKeys[GLFW_KEY_W]) {
            myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
            //update view matrix
            view = myCamera.getViewMatrix();
            myBasicShader.useShaderProgram();
            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
            // compute normal matrix for teapot
            normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
            glUniformMatrix4fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
            directionSpot = myCamera.getPosition();
            directionSpotLoc = glGetUniformLocation(myBasicShader.shaderProgram, "pointLight");
            glUniform3fv(directionSpotLoc, 1, glm::value_ptr(directionSpot));
        }

        if (pressedKeys[GLFW_KEY_S]) {
            myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
            //update view matrix
            view = myCamera.getViewMatrix();
            myBasicShader.useShaderProgram();
            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
            // compute normal matrix for teapot
            normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
            glUniformMatrix4fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
            directionSpot = myCamera.getPosition();
            directionSpotLoc = glGetUniformLocation(myBasicShader.shaderProgram, "pointLight");
            glUniform3fv(directionSpotLoc, 1, glm::value_ptr(directionSpot));
        }

        if (pressedKeys[GLFW_KEY_A]) {
            myCamera.move(gps::MOVE_LEFT, cameraSpeed);
            //update view matrix
            view = myCamera.getViewMatrix();
            myBasicShader.useShaderProgram();
            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
            // compute normal matrix for teapot
            normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
            glUniformMatrix4fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
            directionSpot = myCamera.getPosition();
            directionSpotLoc = glGetUniformLocation(myBasicShader.shaderProgram, "pointLight");
            glUniform3fv(directionSpotLoc, 1, glm::value_ptr(directionSpot));
        }

        if (pressedKeys[GLFW_KEY_D]) {
            myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
            //update view matrix
            view = myCamera.getViewMatrix();
            myBasicShader.useShaderProgram();
            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
            // compute normal matrix for teapot
            normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
            glUniformMatrix4fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
            directionSpot = myCamera.getPosition();
            directionSpotLoc = glGetUniformLocation(myBasicShader.shaderProgram, "pointLight");
            glUniform3fv(directionSpotLoc, 1, glm::value_ptr(directionSpot));
        }

        if (pressedKeys[GLFW_KEY_Q]) {
            angle -= 1.0f;
            // update model matrix for teapot
            model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
            // update normal matrix for teapot
            normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
            myBasicShader.useShaderProgram();
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
        }

        if (pressedKeys[GLFW_KEY_E]) {
            angle += 1.0f;
            // update model matrix for teapot
            model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
            // update normal matrix for teapot
            normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
            myBasicShader.useShaderProgram();
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
        }

        if (pressedKeys[GLFW_KEY_UP]) {
            myCamera.zoom(gps::MOVE_FORWARD, cameraSpeed);
            //update view matrix
            view = myCamera.getViewMatrix();
            myBasicShader.useShaderProgram();
            projection = glm::perspective(glm::radians(myCamera.getFov()),
                (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
                0.1f, 200.0f);
            // send projection matrix to shader
            glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
        }

        if (pressedKeys[GLFW_KEY_DOWN]) {
            myCamera.zoom(gps::MOVE_BACKWARD, cameraSpeed);
            //update view matrix
            view = myCamera.getViewMatrix();
            myBasicShader.useShaderProgram();
            projection = glm::perspective(glm::radians(myCamera.getFov()),
                (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
                0.1f, 200.0f);
            // send projection matrix to shader
            glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
        }

        if (pressedKeys[GLFW_KEY_1]) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }

        if (pressedKeys[GLFW_KEY_2]) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
        }

        if (pressedKeys[GLFW_KEY_3]) {
            myBasicShader.useShaderProgram();
            colorDir = glm::vec3(0.05f, 0.05f, 0.05f); //white light
            // send light color to shader
            glUniform3fv(colorDirLoc, 1, glm::value_ptr(colorDir));
            
            std::vector<const GLchar*> faces;
            faces.push_back("skybox/nightsky_rt.tga");
            faces.push_back("skybox/nightsky_lf.tga");
            faces.push_back("skybox/nightsky_up.tga");
            faces.push_back("skybox/nightsky_dn.tga");
            faces.push_back("skybox/nightsky_bk.tga");
            faces.push_back("skybox/nightsky_ft.tga");

            mySkyBox.Load(faces);
        }

        if (pressedKeys[GLFW_KEY_4]) {
            myBasicShader.useShaderProgram();
            colorDir = glm::vec3(0.5f, 0.5f, 0.5f); //white light
            // send light color to shader
            glUniform3fv(colorDirLoc, 1, glm::value_ptr(colorDir));

            std::vector<const GLchar*> faces;
            faces.push_back("skybox/right.tga");
            faces.push_back("skybox/left.tga");
            faces.push_back("skybox/top.tga");
            faces.push_back("skybox/bottom.tga");
            faces.push_back("skybox/back.tga");
            faces.push_back("skybox/front.tga");

            mySkyBox.Load(faces);
        }

        if (pressedKeys[GLFW_KEY_6]) {
            fog = glm::vec3(1.0f, 0.0f, 0.0f);
            myBasicShader.useShaderProgram();
            glUniform3fv(fogLoc, 1, glm::value_ptr(fog));
        }

        if (pressedKeys[GLFW_KEY_7]) {
            fog = glm::vec3(1.0f, 1.0f, 0.0f);
            myBasicShader.useShaderProgram();
            glUniform3fv(fogLoc, 1, glm::value_ptr(fog));
        }

        if (pressedKeys[GLFW_KEY_8]) {
            shadow = false;
        }

        if (pressedKeys[GLFW_KEY_9]) {
            myBasicShader.useShaderProgram();
            glUniform3fv(pointLoc, 1, glm::value_ptr(glm::vec3(1.0f, 0.0f, 0.0f)));
        }

        if (pressedKeys[GLFW_KEY_0]) {
            myBasicShader.useShaderProgram();
            glUniform3fv(pointLoc, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 0.0f)));
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
    }

    if (pressedKeys[GLFW_KEY_5]) {
        animation = true;
        animationPoint = 0;
    }
}

void initOpenGLWindow() {
    myWindow.Create(1024, 768, "OpenGL Project Core");
}

void setWindowCallbacks() {
	glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
    glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
    glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
}

void initOpenGLState() {
	glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
	glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
    glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
}

void initFBO() {
    //TODO - Create the FBO, the depth texture and attach the depth texture to the FBO
    glGenFramebuffers(1, &shadowMapFBO);
    //create depth texture for FBO
    glGenTextures(1, &depthMapTexture);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
        SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    //attach texture to FBO
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture,
        0);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void initModels() {
    map.LoadModel("models/Map/NewMap.obj");
    car.LoadModel("models/Car/Challenger.obj");
}

void initShaders() {
	myBasicShader.loadShader("shaders/simple.vert", "shaders/simple.frag");
    depthMapShader.loadShader("shaders/depthMap.vert", "shaders/depthMap.frag");
}

void initSkyBox() {
    std::vector<const GLchar*> faces;
    faces.push_back("skybox/right.tga");
    faces.push_back("skybox/left.tga");
    faces.push_back("skybox/top.tga");
    faces.push_back("skybox/bottom.tga");
    faces.push_back("skybox/back.tga");
    faces.push_back("skybox/front.tga");
    
    mySkyBox.Load(faces);
}

void initUniforms() {

    myBasicShader.useShaderProgram();

    // create model matrix for teapot
    model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
	modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");

	// get view matrix for current camera
	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
	// send view matrix to shader
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    // compute normal matrix for teapot
    normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");

	// create projection matrix
	projection = glm::perspective(glm::radians(myCamera.getFov()),
                               (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
                               0.1f, 5000.0f);
	projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
	// send projection matrix to shader
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));	


	//set the light direction (direction towards the light)
	lightDir = glm::vec3(-0.2f, 4.0f, -0.3f);
	lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
	// send light dir to shader
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

	//set light color
    colorDir = glm::vec3(0.5f, 0.5f, 0.5f); //white light
    colorDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");
	// send light color to shader
	glUniform3fv(colorDirLoc, 1, glm::value_ptr(colorDir));

    directionSpot = myCamera.getPosition();
    directionSpotLoc = glGetUniformLocation(myBasicShader.shaderProgram, "pointLight");
    glUniform3fv(directionSpotLoc, 1, glm::value_ptr(directionSpot));

    fog = glm::vec3(1.0f, 1.0f, 0.0f);
    fogLoc = glGetUniformLocation(myBasicShader.shaderProgram, "fog");
    // send light color to shader
    glUniform3fv(fogLoc, 1, glm::value_ptr(fog));

    pointLoc = glGetUniformLocation(myBasicShader.shaderProgram, "point");
    // send light color to shader
    glUniform3fv(pointLoc, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 0.0f)));
}

void renderMap(gps::Shader shader, bool depth) {
    // select active shader program
    shader.useShaderProgram();

    //send teapot model matrix data to shader

    //send teapot normal matrix data to shader
    
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    // do not send the normal matrix if we are rendering in the depth map
    if (!depth) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        normalMatrixLoc = glGetUniformLocation(shader.shaderProgram, "normalMatrix");
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }

    // draw teapot
    map.Draw(shader);
}

void renderCar(gps::Shader shader, bool depth) {
    // select active shader program
    shader.useShaderProgram();

    //send teapot model matrix data to shader
    carModel = model;
    offset += 2.0f * direction;
    if ((offset > 2500.0f && rotation >= 180) || (offset < -2300.0f && rotation <= 0))
        direction *= -1;
    else
        if (offset > 2500.0f || offset < -2300.0f)
            rotation += 1.0f;
    if (offset > 2500.0f)
        offset = 2500.0f;
    if (offset < -2300.0f)
        offset = -2300.0f;
    if (rotation == 360.0f)
        rotation = 0.0f;
    carModel = glm::translate(carModel, glm::vec3(offset, -200.0f, 50.0f));
    carModel = glm::rotate(carModel, glm::radians(rotation), glm::vec3(0.0f, 1.0f, 0.0f));
    if (rotation == 180)
        carModel = glm::translate(carModel, glm::vec3(0.0f, 0.0f, 150.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(carModel));


    //send teapot normal matrix data to shader
    if (!depth) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * carModel));
        normalMatrixLoc = glGetUniformLocation(shader.shaderProgram, "normalMatrix");
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }

    // draw teapot
    car.Draw(shader);
}

glm::mat4 computeLightSpaceTrMatrix() {
    //TODO - Return the light-space transformation matrix
    glm::mat4 lightView = glm::lookAt(lightDir, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    const GLfloat near_plane = 0.1f, far_plane = 5.0f;
    glm::mat4 lightProjection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, near_plane, far_plane);

    glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;
    return lightSpaceTrMatrix;
}

void cameraAnimation() {
    if (animationPoint == 0) {
        myCamera.setCamera(glm::vec3(-873.189514, 361.486603, 242.407684), glm::vec3(-872.200745, 361.538806, 242.386978));
        animationPoint++;
        count = 0;
    }
    else
    if (animationPoint == 1) {
        glm::vec3 p = glm::vec3(1236.275269, 248.056732, 248.817230) - myCamera.getPosition();
        p.x = p.x / 100;
        p.y = p.y / 100;
        p.z = p.z / 100;
        glm::vec3 t = glm::vec3(1237.258545, 247.974496, 248.824097) - myCamera.getTarget();
        t.x = t.x / 100;
        t.y = t.y / 100;
        t.z = t.z / 100;
        if (myCamera.getPosition() == glm::vec3(1236.275269, 248.056732, 248.817230) && myCamera.getTarget() == glm::vec3(1237.258545, 247.974496, 248.824097)) {
            animationPoint++;
            count = 0;
        }
        else
        {
            myCamera.setCamera(myCamera.getPosition() + p, myCamera.getTarget() + t);
        }
        count++;
        if (count == 300)
            myCamera.setCamera(glm::vec3(1236.275269, 248.056732, 248.817230), glm::vec3(1237.258545, 247.974496, 248.824097));
    }
    else
    if (animationPoint == 2) {
        glm::vec3 p = glm::vec3(1887.446167, 196.989548, 227.696182) - myCamera.getPosition();
        p.x = p.x / 100;
        p.y = p.y / 100;
        p.z = p.z / 100;
        glm::vec3 t = glm::vec3(1887.487915, 197.004129, 228.691650) - myCamera.getTarget();
        t.x = t.x / 100;
        t.y = t.y / 100;
        t.z = t.z / 100;
        if (myCamera.getPosition() == glm::vec3(1887.446167, 196.989548, 227.696182) && myCamera.getTarget() == glm::vec3(1887.487915, 197.004129, 228.691650)){
            animationPoint++;
            count = 0;
        }
        else
        {
            myCamera.setCamera(myCamera.getPosition() + p, myCamera.getTarget() + t);
        }
        count++;
        if (count == 300)
            myCamera.setCamera(glm::vec3(1887.446167, 196.989548, 227.696182), glm::vec3(1887.487915, 197.004129, 228.691650));
    }
    else
    if (animationPoint == 3) {
        glm::vec3 p = glm::vec3(1897.893555, 166.479889, 1553.789551) - myCamera.getPosition();
        p.x = p.x / 100;
        p.y = p.y / 100;
        p.z = p.z / 100;
        glm::vec3 t = glm::vec3(1897.905640, 166.544177, 1554.780273) - myCamera.getTarget();
        t.x = t.x / 100;
        t.y = t.y / 100;
        t.z = t.z / 100;
        if (myCamera.getPosition() == glm::vec3(1897.893555, 166.479889, 1553.789551) && myCamera.getTarget() == glm::vec3(1897.905640, 166.544177, 1554.780273)) {
            animationPoint++;
            count = 0;
        }
        else
        {
            myCamera.setCamera(myCamera.getPosition() + p, myCamera.getTarget() + t);
        }
        count++;
        if (count == 300)
            myCamera.setCamera(glm::vec3(1897.893555, 166.479889, 1553.789551), glm::vec3(1897.905640, 166.544177, 1554.780273));
    }
    else
    if (animationPoint == 4) {
        glm::vec3 p = glm::vec3(2027.540039, 151.412918, 1974.046631) - myCamera.getPosition();
        p.x = p.x / 100;
        p.y = p.y / 100;
        p.z = p.z / 100;
        glm::vec3 t = glm::vec3(2026.542969, 151.546649, 1974.084961) - myCamera.getTarget();
        t.x = t.x / 100;
        t.y = t.y / 100;
        t.z = t.z / 100;
        if (myCamera.getPosition() == glm::vec3(2027.540039, 151.412918, 1974.046631) && myCamera.getTarget() == glm::vec3(2026.542969, 151.546649, 1974.084961)) {
            animationPoint++;
            count = 0;
        }
        else
        {
            myCamera.setCamera(myCamera.getPosition() + p, myCamera.getTarget() + t);
        }
        count++;
        if (count == 300)
            myCamera.setCamera(glm::vec3(2027.540039, 151.412918, 1974.046631), glm::vec3(2026.542969, 151.546649, 1974.084961));
    }
    else
    if (animationPoint == 5) {
        glm::vec3 p = glm::vec3(-371.095947, 137.647247, 1974.052734) - myCamera.getPosition();
        p.x = p.x / 100;
        p.y = p.y / 100;
        p.z = p.z / 100;
        glm::vec3 t = glm::vec3(-372.095734, 137.826297, 1974.056274) - myCamera.getTarget();
        t.x = t.x / 100;
        t.y = t.y / 100;
        t.z = t.z / 100;
        if (myCamera.getPosition() == glm::vec3(-371.095947, 137.647247, 1974.052734) && myCamera.getTarget() == glm::vec3(-372.095734, 137.826297, 1974.056274)) {
            animationPoint++;
            count = 0;
        }
        else
        {
            myCamera.setCamera(myCamera.getPosition() + p, myCamera.getTarget() + t);
        }
        count++;
        if (count == 300)
            myCamera.setCamera(glm::vec3(-371.095947, 137.647247, 1974.052734), glm::vec3(-372.095734, 137.826297, 1974.056274));
    }
    else
    if (animationPoint == 6) {
        glm::vec3 p = glm::vec3(-641.115234, 137.647247, 2055.281738) - myCamera.getPosition();
        p.x = p.x / 100;
        p.y = p.y / 100;
        p.z = p.z / 100;
        glm::vec3 t = glm::vec3(-641.137939, 137.847247, 2054.281982) - myCamera.getTarget();
        t.x = t.x / 100;
        t.y = t.y / 100;
        t.z = t.z / 100;
        if (myCamera.getPosition() == glm::vec3(-641.115234, 137.647247, 2055.281738) && myCamera.getTarget() == glm::vec3(-641.137939, 137.847247, 2054.281982)) {
            animationPoint++;
            count = 0;
        }
        else
        {
            myCamera.setCamera(myCamera.getPosition() + p, myCamera.getTarget() + t);
        }
        count++;
        if (count == 300)
            myCamera.setCamera(glm::vec3(-641.115234, 137.647247, 2055.281738), glm::vec3(-641.137939, 137.847247, 2054.281982));
    }
    else
    if (animationPoint == 7) {
        glm::vec3 p = glm::vec3(-673.982056, 132.169037, 545.668579) - myCamera.getPosition();
        p.x = p.x / 100;
        p.y = p.y / 100;
        p.z = p.z / 100;
        glm::vec3 t = glm::vec3(-673.996033, 132.355075, 544.668762) - myCamera.getTarget();
        t.x = t.x / 100;
        t.y = t.y / 100;
        t.z = t.z / 100;
        if (myCamera.getPosition() == glm::vec3(-673.982056, 132.169037, 545.668579) && myCamera.getTarget() == glm::vec3(-673.996033, 132.355075, 544.668762)) {
            animationPoint++;
            count = 0;
        }
        else
        {
            myCamera.setCamera(myCamera.getPosition() + p, myCamera.getTarget() + t);
        }
        count++;
        if (count == 300)
            myCamera.setCamera(glm::vec3(-673.982056, 132.169037, 545.668579), glm::vec3(-673.996033, 132.355075, 544.668762));
    }
    else
    if (animationPoint == 8) {
        glm::vec3 p = glm::vec3(-849.188354, 138.553558, 224.872681) - myCamera.getPosition();
        p.x = p.x / 100;
        p.y = p.y / 100;
        p.z = p.z / 100;
        glm::vec3 t = glm::vec3(-848.189270, 138.781482, 224.839539) - myCamera.getTarget();
        t.x = t.x / 100;
        t.y = t.y / 100;
        t.z = t.z / 100;
        if (myCamera.getPosition() == glm::vec3(-849.188354, 138.553558, 224.872681) && myCamera.getTarget() == glm::vec3(-848.189270, 138.781482, 224.839539)) {
            animationPoint++;
            count = 0;
        }
        else
        {
            myCamera.setCamera(myCamera.getPosition() + p, myCamera.getTarget() + t);
        }
        count++;
        if (count == 300)
            myCamera.setCamera(glm::vec3(-849.188354, 138.553558, 224.872681), glm::vec3(-848.189270, 138.781482, 224.839539));
    }
    else
    if (animationPoint == 9) {
        glm::vec3 p = glm::vec3(-873.189514, 361.486603, 242.407684) - myCamera.getPosition();
        p.x = p.x / 100;
        p.y = p.y / 100;
        p.z = p.z / 100;
        glm::vec3 t = glm::vec3(-872.200745, 361.538806, 242.386978) - myCamera.getTarget();
        t.x = t.x / 100;
        t.y = t.y / 100;
        t.z = t.z / 100;
        if (myCamera.getPosition() == glm::vec3(-873.189514, 361.486603, 242.407684) && myCamera.getTarget() == glm::vec3(-872.200745, 361.538806, 242.386978)) {
            animation = false;
            count = 0;
        }
        else
        {
            myCamera.setCamera(myCamera.getPosition() + p, myCamera.getTarget() + t);
        }
        count++;
        if (count == 300)
            myCamera.setCamera(glm::vec3(-873.189514, 361.486603, 242.407684), glm::vec3(-872.200745, 361.538806, 242.386978));
    }
    myBasicShader.useShaderProgram();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    glUniformMatrix4fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
}

void renderScene() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (shadow) {
        depthMapShader.useShaderProgram();
        glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"),
            1,
            GL_FALSE,
            glm::value_ptr(computeLightSpaceTrMatrix()));
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        renderMap(depthMapShader, true);
        renderCar(myBasicShader, true);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    myBasicShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "lightSpaceTrMatrix"),
        1,
        GL_FALSE,
        glm::value_ptr(computeLightSpaceTrMatrix()));
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    renderMap(myBasicShader, true);
    renderCar(myBasicShader, true);
    glClear(GL_DEPTH_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (animation == true)
        cameraAnimation();

    myBasicShader.useShaderProgram();
    positionSpot = myCamera.getPosition();
    glUniform3fv(positionSpotLoc, 1, glm::value_ptr(positionSpot));

    directionSpot = myCamera.getFront();
    glUniform3fv(directionSpotLoc, 1, glm::value_ptr(directionSpot));

    //render the scene
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "shadowMap"), 3);

    // render the teapot
    renderMap(myBasicShader, false);
    renderCar(myBasicShader, false);
    skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
    skyboxShader.useShaderProgram();
    view = myCamera.getViewMatrix();
    glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "view"), 1, GL_FALSE,
        glm::value_ptr(view));

    projection = glm::perspective(glm::radians(45.0f), (float)myWindow.getWindowDimensions().width / myWindow.getWindowDimensions().height, 0.1f, 1000.0f);
    glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "projection"), 1, GL_FALSE,
        glm::value_ptr(projection));
    mySkyBox.Draw(skyboxShader, view, projection);
}

void cleanup() {
    myWindow.Delete();
    //cleanup code for your own data
}

int main(int argc, const char * argv[]) {

    try {
        initOpenGLWindow();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    initOpenGLState();
	initModels();
	initShaders();
	initUniforms();
    setWindowCallbacks();
    initSkyBox();
    initFBO();

	//glCheckError();
	// application loop
	while (!glfwWindowShouldClose(myWindow.getWindow())) {
        processMovement();
	    renderScene();

		glfwPollEvents();
		glfwSwapBuffers(myWindow.getWindow());

		//glCheckError();
	}

	cleanup();

    return EXIT_SUCCESS;
}
