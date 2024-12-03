#pragma once

#include "mygl/base.h"
#include "mygl/model.h"

#include <map>

struct Planet
{
    std::vector<Model> partModel;
    std::map<int, std::map<int, Vector3D>> emissionColors;

    Matrix4D transformation = Matrix4D::identity();
    Matrix4D rotation = Matrix4D::identity();

    Vector3D position = {0.0, 0.0, 0.0};
};

/**
 * @brief Initializes the planet object with all its meshes.
 *
 * @return Initialized planet.
 */
Planet planetLoad(const std::string &planetFilePath);

/**
 * @brief Deletes the given planet object.
 */
void planetDelete(Planet &planet);

/**
 * @brief Rotates the planet according to the given rotation vector depending on the given speed.
 */
void planetRotate(Planet &planet, Vector3D rotationVec, float planeSpeed, float dt);

/**
 * @brief Sets the emission of the given planet model to be on or off.
 */
void setEmisson(Planet &planet, bool emission);
