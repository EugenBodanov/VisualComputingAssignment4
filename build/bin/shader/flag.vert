#version 330 core
/* We used a separate vertex shader for the flag rendering.
 * Alternatively one could just use one standard shader and set a flag to determine if displacement is necessary within <default.vert>
 */

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUV;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;

uniform float amplitudes[3];
uniform float phases[3];      // == phi
uniform float frequencies[3]; // == omega
uniform vec2 directions[3];
uniform float zPosMin;
uniform float accumTime; // is updated within the main program!

out vec3 tNormal;
out vec3 tFragPos;


float getDisplacement(vec2 pos) {
    float displacement = 0.0f;
    for (int i = 0; i < 3; i++) {
        displacement += amplitudes[i] * sin(dot(directions[i], pos) * frequencies[i] + accumTime * phases[i]);
    }
    return displacement * (aPosition.z / zPosMin);
}

// -------- Gives the partial derivative of the H(p, t) function w.r.t. y if (deriveY == true), otherwise w.r.t. z -------- //
float getPartialDerivative(bool deriveY, vec2 pos, float time) {
    float result = 0.0f;
    for (int i = 0; i < 3; ++i) {
        // ----- alpha denotes the inner term when using the chain rule for derivation ----- //
        float alpha = (frequencies[i] * dot(pos, directions[i])) + time * phases[i];
        if (deriveY) {
            result += frequencies[i] * directions[i].x * amplitudes[i] * cos(alpha);
        } else {
            result += frequencies[i] * directions[i].y * amplitudes[i] * cos(alpha);
        }
    }

    return result * (pos.y / zPosMin); // in the 2D pos, y equals z
}


void main(void)
{
    // Calculate the partial derivatives of H(p, t) with respect to y and z
    vec2 partialDeriv;
    vec3 modifiedPos = aPosition;

    modifiedPos.x += getDisplacement(aPosition.yz); // Displacement on x-axis

    float partialDerivY = getPartialDerivative(true, aPosition.yz, accumTime);
    float partialDerivZ = getPartialDerivative(false, aPosition.yz, accumTime);

    // New normals which consider the displacement of the flag
    vec3 normal = normalize(cross(vec3(partialDerivY, 1.0f, 0.0f), vec3(partialDerivZ, 0.0f, 1.0f)));


    gl_Position = uProj * uView * uModel * vec4(modifiedPos, 1.0);
    tFragPos = vec3(uModel * vec4(modifiedPos, 1.0));

    tNormal = normalize(mat3(transpose(inverse(uModel))) * normal);
}

