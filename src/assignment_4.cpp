#include <cstdlib>
#include <iostream>

#include "mygl/shader.h"
#include "mygl/mesh.h"
#include "mygl/camera.h"

#include "planet.h"
#include "plane.h"

enum eCameraFollow
{
    PLANE,
    PLANET,
    PLANET_LOOK_AT_PLANE,
    NONE
};

/* enum for different render modes */
enum eRenderMode
{
    COLOR = 0,   // render diffuse colors
    NORMAL,      // render normals
    MODE_COUNT,
};

/* plane light directions */
const std::vector<Vector3D> planeLightDirs = {
    { 1.0f, 0.0f, 0.0f },  // left wing, red
    { 1.0f, 0.0f, 0.0f },  // left wing, white strobe
    {-1.0f, 0.0f, 0.0f },  // right wing, green
    {-1.0f, 0.0f, 0.0f },  // right wing, white strobe
    { 0.0f, 0.0f, 1.0f },  // rudder, white
    { 0.0f, 1.0f, 0.0f }   // rudder, red strobe
};

/* plane light positions */
const std::vector<Vector3D> planeLightPositions = {
    { 4.335f, -0.1395f,  1.03f },  // left wing, red
    { 4.3f,   -0.147f,   0.45f },  // left wing, white strobe
    {-4.335f, -0.1395f,  1.03f },  // right wing, green
    {-4.3f,   -0.147f,   0.45f },  // right wing, white strobe
    { 0.0f,    1.35f,   -3.918f},  // rudder, white
    { 0.0f,    1.4022f, -3.5f  }   // rudder, red strobe
};

/* struct holding all necessary state variables of the scene */
struct
{
    /* camera */
    Camera camera;
    eCameraFollow cameraFollow;
    float zoomSpeedMultiplier;

    /* planet */
    Planet planet;

    /* plane */
    Plane plane;

    /* shader */
    ShaderProgram shaderColor;
    ShaderProgram shaderNormal;
    ShaderProgram shaderFlagColor; // TODO: Add shaders for rendering the Flag
    ShaderProgram shaderFlagNormal;
    eRenderMode renderMode;
} sScene;

/* struct holding all state variables for input */
struct
{
    bool mouseLeftButtonPressed = false;
    Vector2D mousePressStart;
    bool keyPressed[Plane::eControl::CONTROL_COUNT] = {false, false, false, false};
} sInput;

// Light Data
struct PointLight{
    Vector3D position;
    Vector3D color;
    float intensity; // Base intencity of the light
    // formula = intencity / (constant + linear * d + quadratic * d^2); d->distance 
    float constant; // light near the source
    float linear; // luminous intensity decreases when distance increases
    float quadratic; // in the real world, luminous intensity decreases quadratic to the distance

    bool isStrobe; // light blinks or not
    float strobeInterval; // how often light blinks
};

std::vector <PointLight> planeLights;

/* GLFW callback function for keyboard events */
void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    /* close window on escape */
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }

    /* make screenshot and save in work directory */
    if (key == GLFW_KEY_P && action == GLFW_PRESS)
    {
        screenshotToPNG("screenshot.png");
    }

    /* input for camera control */
    if (key == GLFW_KEY_0 && action == GLFW_PRESS)
    {
        sScene.cameraFollow = eCameraFollow::NONE;
        sScene.camera.fov = BASE_FOV;
        sScene.camera.lookAt = sScene.planet.position;
        sScene.camera.position = BASE_CAM_POSITION;
        resetCameraRotation(sScene.camera);
    }
    if (key == GLFW_KEY_1 && action == GLFW_PRESS)
    {
        sScene.cameraFollow = eCameraFollow::PLANE;
        sScene.camera.lookAt = sScene.plane.basePosition;
        sScene.camera.position = sScene.plane.basePosition + BASE_CAM_FOLLOW_OFFSET;
        resetCameraRotation(sScene.camera);
    }
    if (key == GLFW_KEY_2 && action == GLFW_PRESS)
    {
        sScene.cameraFollow = eCameraFollow::PLANET;
        sScene.camera.fov = BASE_FOV;
        sScene.camera.lookAt = sScene.planet.position;
        sScene.camera.position = BASE_CAM_POSITION;
    }
    if (key == GLFW_KEY_3 && action == GLFW_PRESS)
    {
        sScene.cameraFollow = eCameraFollow::PLANET_LOOK_AT_PLANE;
        sScene.camera.fov = BASE_FOV;
        sScene.camera.lookAt = sScene.planet.position;
        sScene.camera.position = BASE_CAM_POSITION;
    }

    /* input for plane control */
    if (key == GLFW_KEY_W)
    {
        sInput.keyPressed[Plane::eControl::FASTER] = (action == GLFW_PRESS || action == GLFW_REPEAT);
    }
    if (key == GLFW_KEY_S)
    {
        sInput.keyPressed[Plane::eControl::SLOWER] = (action == GLFW_PRESS || action == GLFW_REPEAT);
    }

    if (key == GLFW_KEY_A)
    {
        sInput.keyPressed[Plane::eControl::LEFT] = (action == GLFW_PRESS || action == GLFW_REPEAT);
    }
    if (key == GLFW_KEY_D)
    {
        sInput.keyPressed[Plane::eControl::RIGHT] = (action == GLFW_PRESS || action == GLFW_REPEAT);
    }

    if (key == GLFW_KEY_SPACE)
    {
        sInput.keyPressed[Plane::eControl::UP] = (action == GLFW_PRESS || action == GLFW_REPEAT);
    }
    if (key == GLFW_KEY_LEFT_CONTROL)
    {
        sInput.keyPressed[Plane::eControl::DOWN] = (action == GLFW_PRESS || action == GLFW_REPEAT);
    }

    /* toggle render mode */
    if (key == GLFW_KEY_R && action == GLFW_PRESS)
    {
        sScene.renderMode = static_cast<eRenderMode>((static_cast<int>(sScene.renderMode) + 1) % eRenderMode::MODE_COUNT);
    }
}

/* GLFW callback function for mouse position events */
void mousePosCallback(GLFWwindow *window, double x, double y)
{
    /* called on cursor position change */
    if (sInput.mouseLeftButtonPressed)
    {
        Vector2D diff = sInput.mousePressStart - Vector2D(static_cast<float>(x), static_cast<float>(y));
        cameraUpdateOrbit(sScene.camera, diff, 0.0f);
        sInput.mousePressStart = Vector2D(static_cast<float>(x), static_cast<float>(y));
    }
}

/* GLFW callback function for mouse button events */
void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        sInput.mouseLeftButtonPressed = (action == GLFW_PRESS || action == GLFW_REPEAT);

        double x, y;
        glfwGetCursorPos(window, &x, &y);
        sInput.mousePressStart = Vector2D(static_cast<float>(x), static_cast<float>(y));
    }
}

/* GLFW callback function for mouse scroll events */
void mouseScrollCallback(GLFWwindow *window, double xoffset, double yoffset)
{
    cameraUpdateOrbit(sScene.camera, {0, 0}, -sScene.zoomSpeedMultiplier * static_cast<float>(yoffset));
}

/* GLFW callback function for window resize event */
void windowResizeCallback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
    sScene.camera.width = static_cast<float>(width);
    sScene.camera.height = static_cast<float>(height);
}

/* function to setup and initialize the whole scene */
void sceneInit(float width, float height)
{
    /* initialize camera */
    sScene.camera = cameraCreate(width, height, BASE_FOV, 0.1f, 350.0f, sScene.plane.basePosition + BASE_CAM_FOLLOW_OFFSET, sScene.plane.basePosition);
    sScene.cameraFollow = eCameraFollow::PLANE;
    sScene.zoomSpeedMultiplier = 0.05f;

    /* setup objects in scene and create opengl buffers for meshes */
    sScene.plane = planeLoad("assets/plane/cartoon-plane.obj", "assets/plane/flag_uibk.obj");
    sScene.planet = planetLoad("assets/planet/cute-little-planet.obj");

    /* load shader from file */
    sScene.shaderColor = shaderLoad("shader/default.vert", "shader/color.frag");
    sScene.shaderNormal = shaderLoad("shader/default.vert", "shader/normal.frag");
    sScene.shaderFlagColor = shaderLoad("shader/flag.vert", "shader/color.frag");
    sScene.shaderFlagNormal = shaderLoad("shader/flag.vert", "shader/normal.frag");

    sScene.renderMode = eRenderMode::COLOR;

    // Light init

    // red = {1,0,0}, green = {0,1,0}, white = {1,1,1}

    float constant = 1.0f;
    float linear = 0.09f;
    float quadratic = 0.01f;

    planeLights.push_back({ planeLightPositions[0], {1,0,0}, 1.0f, constant, linear, quadratic, false, 0.0f});
    planeLights.push_back({ planeLightPositions[1], {1,1,1}, 1.0f, constant, linear, quadratic, true, 0.5f });
    planeLights.push_back({ planeLightPositions[2], {0,1,0}, 1.0f, constant, linear, quadratic, false, 0.0f });
    planeLights.push_back({ planeLightPositions[3], {1,1,1}, 1.0f, constant, linear, quadratic, true, 0.5f });
    planeLights.push_back({ planeLightPositions[4], {1,1,1}, 1.0f, constant, linear, quadratic, false, 0.0f });
    planeLights.push_back({ planeLightPositions[5], {1,0,0}, 1.0f, constant, linear, quadratic, true, 0.5f });
}

void lightsUpdate(float dt){
    // Update the position
    for (int i = 0; i < planeLights.size(); ++i) {
        Vector4D localPos = Vector4D(planeLightPositions[i].x, planeLightPositions[i].y, planeLightPositions[i].z, 1.0f);
        Vector4D worldPos = sScene.plane.transformation * localPos;
        planeLights[i].position = Vector3D(worldPos.x, worldPos.y, worldPos.z);
    }

    // Update blinking
    static float accumTime = 0.0f;
    accumTime += dt;
    for (auto &light : planeLights) {
        if (light.isStrobe) {
            float phase = fmod(accumTime, light.strobeInterval * 2.0f);
            if (phase < light.strobeInterval) {
                light.intensity = 1.0f;
            } else {
                light.intensity = 0.0f;
            }
        }
    }
}

/* function to move and update objects in scene (e.g., rotate cube according to user input) */
void sceneUpdate(float dt)
{
    planeMove(sScene.plane, sInput.keyPressed, dt);
    planetRotate(sScene.planet, getPlaneTurningVector(sScene.plane), sScene.plane.speed, dt);

    lightsUpdate(dt);

    if (sScene.cameraFollow == eCameraFollow::PLANE)
    {
        cameraFollow(sScene.camera, sScene.plane.position);

        /* change fov depending on speed */
        sScene.camera.fov = getSpeedFov(sScene.plane);
    }
    else if (sScene.cameraFollow == eCameraFollow::PLANET)
    {
        setCameraRotation(sScene.camera, Matrix3D(sScene.planet.rotation));
    }
    else if (sScene.cameraFollow == eCameraFollow::PLANET_LOOK_AT_PLANE)
    {
        sScene.camera.lookAt = inverse(Matrix3D(sScene.planet.rotation)) * sScene.plane.position;
        setCameraRotation(sScene.camera, Matrix3D(sScene.planet.rotation));
    }
}

void renderPlanetAndPlane(ShaderProgram& shader, bool renderNormal) {
    /* setup camera and model matrices */
    Matrix4D proj = cameraProjection(sScene.camera); // perspective projection (3D -> 2D coordinates on the screen)
    Matrix4D view = cameraView(sScene.camera);

    glUseProgram(shader.id);
    shaderUniform(shader, "uProj",  proj);
    shaderUniform(shader, "uView",  view);
    shaderUniform(shader, "uModel",  sScene.plane.transformation);
    if (renderNormal)
    {
        shaderUniform(shader, "uViewPos", cameraPosition(sScene.camera));
        shaderUniform(shader, "isFlag", false);
    }

    /* render plane */
    for (unsigned int i = 0; i < sScene.plane.partModel.size(); i++)
    {
        auto& model = sScene.plane.partModel[i];
        auto& transform = sScene.plane.partTransformations[i];
        glBindVertexArray(model.mesh.vao);

        shaderUniform(shader, "uModel", sScene.plane.transformation * transform);

        for(auto& material : model.material)
        {
            if (!renderNormal)
            {
                /* set material properties */
                shaderUniform(shader, "uMaterial.diffuse", material.diffuse);
            }
            glDrawElements(GL_TRIANGLES, material.indexCount, GL_UNSIGNED_INT, (const void*) (material.indexOffset*sizeof(unsigned int)) );
        }
    }

    /* render planet */
    for(unsigned int i=0; i < sScene.planet.partModel.size(); i++)
    {
        auto& model = sScene.planet.partModel[i];
        glBindVertexArray(model.mesh.vao);

        shaderUniform(shader, "uModel", sScene.planet.transformation);

        for(auto& material : model.material)
        {
            if (!renderNormal)
            {
                /* set material properties */
                shaderUniform(shader, "uMaterial.diffuse", material.diffuse);
            }
            glDrawElements(GL_TRIANGLES, material.indexCount, GL_UNSIGNED_INT, (const void*) (material.indexOffset*sizeof(unsigned int)) );
        }
    }

    /* cleanup opengl state */
    glBindVertexArray(0);
    glUseProgram(0);
}

void renderFlag(ShaderProgram& shader, bool renderNormal) {

    /* setup camera and model matrices */
    Matrix4D proj = cameraProjection(sScene.camera); // perspective projection (3D -> 2D coordinates on the screen)
    Matrix4D view = cameraView(sScene.camera);

    glUseProgram(shader.id);
    shaderUniform(shader, "uProj",  proj);
    shaderUniform(shader, "uView",  view);
    shaderUniform(shader, "uModel",  sScene.plane.transformation);
    if (renderNormal)
    {
        shaderUniform(shader, "uViewPos", cameraPosition(sScene.camera));
        shaderUniform(shader, "isFlag", false);
    }

    /* render flag */
    {
        auto& model = sScene.plane.flag.model;
        // uModel: Transforms local vertices to world space coordinates!
        shaderUniform(shader, "uModel", sScene.plane.transformation * sScene.plane.flagModelMatrix * sScene.plane.flagNegativeRotation);
        glBindVertexArray(model.mesh.vao);

        // TODO: Pass the wave parameter and time as uniforms to the shader for displacement calculations
        for (int i = 0; i < 3; i++) {
            shaderUniform(shader, "amplitudes[" + std::to_string(i) + "]", sScene.plane.flagSim.parameter[i].amplitude);
            shaderUniform(shader, "phases[" + std::to_string(i) + "]", sScene.plane.flagSim.parameter[i].phi);
            shaderUniform(shader, "frequencies[" + std::to_string(i) + "]", sScene.plane.flagSim.parameter[i].omega);
            shaderUniform(shader, "directions[" + std::to_string(i) + "]", sScene.plane.flagSim.parameter[i].direction);
        }
        shaderUniform(shader, "zPosMin", sScene.plane.flag.minPosZ);
        shaderUniform(shader, "accumTime", sScene.plane.flagSim.accumTime);

        for(auto& material : model.material)
        {
            if (!renderNormal)
            {
                /* set material properties */
                shaderUniform(shader, "uMaterial.diffuse", material.diffuse);
                // TODO: Add the wave parameters as uniforms (they act as constants within a draw call)
                // shaderUniform()sScene.plane.flag.vertices
            }
            else
            {
                shaderUniform(shader, "isFlag", true);
            }
            glDrawElements(GL_TRIANGLES, material.indexCount, GL_UNSIGNED_INT, (const void*) (material.indexOffset*sizeof(unsigned int)) );
        }
    }

    /* cleanup opengl state */
    glBindVertexArray(0);
    glUseProgram(0);
}

/* Function to set correct shader programs for the different rendering settings */
void renderColor(bool renderNormal) {
    ShaderProgram shaderScene;
    ShaderProgram shaderFlag;
    if (renderNormal) {
        shaderScene = sScene.shaderNormal;
        shaderFlag = sScene.shaderFlagNormal;
    } else {
        shaderScene = sScene.shaderColor;
        shaderFlag = sScene.shaderFlagColor;
    }

    glUseProgram(shaderScene.id);
    // Give data about lights
    shaderUniform(shaderScene, "numLights", (int)planeLights.size());
    for (int i = 0; i < (int)planeLights.size(); ++i) {
        std::string base = "lights[" + std::to_string(i) + "]";
        shaderUniform(shaderScene, (base + ".position").c_str(), planeLights[i].position);
        shaderUniform(shaderScene, (base + ".color").c_str(), planeLights[i].color);
        shaderUniform(shaderScene, (base + ".intensity").c_str(), planeLights[i].intensity);
        shaderUniform(shaderScene, (base + ".constant").c_str(), planeLights[i].constant);
        shaderUniform(shaderScene, (base + ".linear").c_str(), planeLights[i].linear);
        shaderUniform(shaderScene, (base + ".quadratic").c_str(), planeLights[i].quadratic);
    }

    renderPlanetAndPlane(shaderScene, renderNormal);

    glUseProgram(shaderFlag.id);
    shaderUniform(shaderFlag, "numLights", (int)planeLights.size());
    for (int i = 0; i < (int)planeLights.size(); ++i) {
        std::string base = "lights[" + std::to_string(i) + "]";
        shaderUniform(shaderFlag, (base + ".position").c_str(), planeLights[i].position);
        shaderUniform(shaderFlag, (base + ".color").c_str(), planeLights[i].color);
        shaderUniform(shaderFlag, (base + ".intensity").c_str(), planeLights[i].intensity);
        shaderUniform(shaderFlag, (base + ".constant").c_str(), planeLights[i].constant);
        shaderUniform(shaderFlag, (base + ".linear").c_str(), planeLights[i].linear);
        shaderUniform(shaderFlag, (base + ".quadratic").c_str(), planeLights[i].quadratic);
    }

    renderFlag(shaderFlag, renderNormal);

    glUseProgram(0);
}

/* function to draw all objects in the scene */
void sceneDraw()
{
    /* clear framebuffer color */
    glClearColor(135.0 / 255, 206.0 / 255, 235.0 / 255, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /*------------ render scene -------------*/
    {
        if (sScene.renderMode == eRenderMode::COLOR)
        {
            renderColor(false); // Changed the renderColor function prototype so that flag can be created with an individual vertex shader!
        }
        else if (sScene.renderMode == eRenderMode::NORMAL)
        {
            renderColor(true);
        }
    }
    glCheckError();

    /* cleanup opengl state */
    glBindVertexArray(0);
    glUseProgram(0);
}

int main(int argc, char **argv)
{
    /* create window/context */
    int width = 1280;
    int height = 720;
    GLFWwindow *window = windowCreate("Assignment 4 - Shader Programming", width, height);
    if (!window)
    {
        return EXIT_FAILURE;
    }

    /* set window callbacks */
    glfwSetKeyCallback(window, keyCallback);
    glfwSetCursorPosCallback(window, mousePosCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetScrollCallback(window, mouseScrollCallback);
    glfwSetFramebufferSizeCallback(window, windowResizeCallback);

    /*---------- init opengl stuff ------------*/
    glEnable(GL_DEPTH_TEST);

    /* setup scene */
    sceneInit(static_cast<float>(width), static_cast<float>(height));

    /*-------------- main loop ----------------*/
    double timeStamp = glfwGetTime();
    double timeStampNew = 0.0;

    /* loop until user closes window */
    while (!glfwWindowShouldClose(window))
    {
        /* poll and process input and window events */
        glfwPollEvents();

        /* update model matrix of cube */
        timeStampNew = glfwGetTime();
        sceneUpdate(static_cast<float>(timeStampNew - timeStamp));
        timeStamp = timeStampNew;

        /* draw all objects in the scene */
        sceneDraw();

        /* swap front and back buffer */
        glfwSwapBuffers(window);
    }

    /*-------- cleanup --------*/
    /* delete opengl shader and buffers */
    shaderDelete(sScene.shaderColor);
    shaderDelete(sScene.shaderNormal);
    planeDelete(sScene.plane);
    planetDelete(sScene.planet);

    /* cleanup glfw/glcontext */
    windowDelete(window);

    return EXIT_SUCCESS;
}
