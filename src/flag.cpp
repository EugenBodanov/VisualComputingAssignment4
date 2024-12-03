#include <algorithm>

#include "flag.h"

#include <stdexcept>

float getDisplacementValue(Vector2D pos, const FlagSim &sim, int waveParamIndex)
{
    const WaveParams &params = sim.parameter[waveParamIndex];
    
    return params.amplitude * static_cast<float>(sin(dot(normalize(params.direction), pos) * params.omega + sim.accumTime * params.phi));
}

float flagDisplacement(const FlagSim &sim, Vector2D position, float minPosZ)
{
    float displacement = getDisplacementValue(position, sim, 0) + getDisplacementValue(position, sim, 1) + getDisplacementValue(position, sim, 2);
    float positionScale = position[1] / minPosZ;
    return displacement * positionScale;
}

Flag flagCreate(const std::string& flagFilePath)
{
    Flag flag;
    std::vector<Model> models = modelLoad(flagFilePath);

    if(models.size() != 1)
    {
        throw std::runtime_error("[Flag] number of parts do not match!" + std::to_string(models.size()));
    }
    flag.model = models[0];
    flag.minPosZ = -8.0f;

    /* 
     * Load vertices for flag, not required in implementation of shader based animation
     *
     * TODO - Remove this part if flag animation in shader implemented
     */
    flag.vertices = verticesLoad(flagFilePath);

    return flag;
}

void flagDelete(Flag &flag)
{
    modelDelete(flag.model);
}

void updateSimulation(FlagSim& flagSim, float speedFactor, float dt)
{
    float dtMultiplier = (flagSim.maxDtFactor - flagSim.minDtFactor) * speedFactor + flagSim.minDtFactor;
    flagSim.accumTime += dtMultiplier * dt;
}

void animateFlag(Flag &flag, FlagSim &flagSim)
{
    for (unsigned i = 0; i < flag.vertices.size(); i++)
    {
        flag.vertices[i].pos.x = flagDisplacement(flagSim, {flag.vertices[i].pos.y, flag.vertices[i].pos.z}, flag.minPosZ);
    }
    glBindVertexArray(flag.model.mesh.vao);
    {
        glBindBuffer(GL_ARRAY_BUFFER, flag.model.mesh.vbo);
        glBufferData(GL_ARRAY_BUFFER, flag.vertices.size() * sizeof(Vertex), flag.vertices.data(), GL_DYNAMIC_DRAW);
        glCheckError();
    }
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
