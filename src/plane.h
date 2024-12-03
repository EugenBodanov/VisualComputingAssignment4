#pragma once

#include "mygl/base.h"
#include "mygl/model.h"

#include "flag.h"

#include <vector>
#include <map>

/* translation and color for the flag plane */
namespace flagPlane
{
    const Matrix4D trans = Matrix4D::translation({0.0f, -0.0f, -8.5f});
}

struct Plane
{
    enum eControl
    {
        LEFT,
        RIGHT,
        UP,
        DOWN,
        FASTER,
        SLOWER,
        CONTROL_COUNT
    };

    enum ePart
    {
        PROPELLER = 0,
        WHEEL_CARCASS_BACK,
        TYRE_BACK,
        WHEEL_CARCASS_LEFT,
        TYRE_LEFT,
        WHEEL_CARCASS_RIGHT,
        TYRE_RIGHT,
        HULL,
        STROBE_RUDDER,
        LIGHT_LEFT_WING,
        STROBE_RIGHT_WING,
        STROBE_LEFT_WING,
        LIGHT_RIGHT_WING,
        LIGHT_RUDDER,
        FLAG_CONNECTOR,
        PART_COUNT
    };

    std::vector<Matrix4D> partTransformations;
    std::vector<Model> partModel;
    std::map<int, Vector3D> emissionColors;

    Matrix4D transformation = Matrix4D::identity();
    Matrix4D rotation = Matrix4D::identity();

    Vector3D basePosition = {0.0, 45.0, -5.0};
    Vector3D position = {0.0, 0.0, 0.0};
    Vector3D angles = {0.0, 0.0, 0.0};
    float dYAngle = 0.0f;

    float minHeight = 45.0f;
    float maxHeight = 60.0f;

    float speed = 20.0f;
    float minSpeed = 15.0f;
    float maxSpeed = 50.0f;

    float minFov = 40.0f;
    float maxFov = 65.0f;

    float minPropellerRotationSpeed = static_cast<float>(8 * M_PI);
    float maxPropellerRotationSpeed = static_cast<float>(16 * M_PI);

    /* flag */
    FlagSim flagSim;
    Flag flag;
    Matrix4D flagModelMatrix;
    Matrix4D flagNegativeRotation = Matrix4D::identity();
};

/**
 * @brief Initializes the plane object with all its meshes, including its flag object. Initially, the plane is placed according to 'basePosition'.
 *
 * @return Initialized plane.
 */
Plane planeLoad(const std::string& planeFilePath, const std::string& flagFilePath);

/**
 * @brief Deletes the given plane object, including its flag object.
 */
void planeDelete(Plane &plane);

/**
 * @brief Animates the given plane according to the given input and changes its height accordingly. Also animates the propeller and flag.
 * Does NOT animate the planet to simulate forward movement.
 */
void planeMove(Plane &plane, bool control[], float dt);

/**
 * @brief Calculates a vector perpendicular to the y-axis and in positive x-direction depending on the plane's yaw and roll.
 * The returned vector can then be used as the basis for the planet's rotation.
 * 
 * @return A vector to be used as the basis for the planet's rotation.
 */
Vector3D getPlaneTurningVector(Plane &plane);

/**
 * @brief Calculates in percent how fast the plane is moving currently depending on its min and max speed.
 * 
 * @return The factor of the current speed in [0, 1].
 */
float getSpeedFactor(Plane &plane);

/**
 * @brief Calculates the FOV depending on the plane's current speed. The faster the plane is moving, the higher the FOV.
 * Depends on min and max FOV and the speed factor.
 * 
 * @return The desired FOV of the current speed.
 */
float getSpeedFov(Plane &plane);

/**
 * @brief Sets the emission of the lights of the given plane model to be on or off.
 */
void setEmission(Plane &plane, bool emission);
