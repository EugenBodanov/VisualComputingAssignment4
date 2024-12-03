#pragma once

#include "mygl/base.h"
#include "mygl/model.h"

struct WaveParams
{
    float amplitude;
    float phi;
    float omega;
    Vector2D direction;
};

struct FlagSim
{
    /**
     * Parameters for the 3 wave functions for the flag simulation
     */
    WaveParams parameter[3] = {
        { 1.0f,  1.0f,  0.25f, normalize(Vector2D{0.0f,  1.0f}) },
        { 0.2f,  1.5f, 0.75f,  normalize(Vector2D{0.0f, 1.0f}) },
        { 0.1f,  5.0f,  2.0f,  normalize(Vector2D{-1.0f / 3.0f, 1.0f}) },
    };

    float minDtFactor = 0.8f;
    float maxDtFactor = 4.0f;

    float accumTime = 0.0f;
};

struct Flag {
    Model model;

    /* vertices of the flag, not required anymore in solution with shader based animation */
    std::vector<Vertex> vertices;

    float minPosZ;
};

/**
 * @brief Initializes a plane grid to visualize flag surface. For that a vector containing all grid vertices is created and
 * a mesh (see function meshCreate(...)) is setup with these vertices.
 *
 * @param colors Vector of base colors of the Flag.
 *
 * @return Object containing the vector of vertices and an initialized mesh structure that can be drawn with OpenGL.
 *
 * usage:
 *
 *   Flag myFlag = flagCreate({0.0, 0.0, 1.0, 0.5})
 *   glBindVertexArray(myFlag.mesh.vao);
 *   glDrawElements(GL_TRIANGLES, myFlag.size_ibo, GL_UNSIGNED_INT, nullptr);
 *
 */
Flag flagCreate(const std::string& flagFilePath);


/**
 * @brief Cleanup and delete all OpenGL buffers of the flag mesh. Has to be called for each flag after it is not used anymore.
 *
 * @param flag Flag to delete.
 */
void flagDelete(Flag& flag);

/**
 * @brief Updates the flag simulation by increasing the time accumulator.
 *
 * @param flagSim Flag simulation to update.
 * @param speedFactor Factor to adjust the simulation speed.
 * @param dt Time passed since last update.
 */
void updateSimulation(FlagSim& flagSim, float speedFactor, float dt);

/**
 * @brief Animates the flag by updating the vertex positions of the flag mesh.
 *
 * TODO - This function should be replaced by a shader based animation in the assignment and the it should be removed.
 *
 * @param flag Flag to be animated.
 * @param flagSim Object for flag simulation.
 */
void animateFlag(Flag& flag, FlagSim& flagSim);
