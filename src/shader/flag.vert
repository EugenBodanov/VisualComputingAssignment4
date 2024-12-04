#version 330 core
/* We used a separate vertex shader for the flag rendering.
 * Alternatively one could just use the standard shader and set a flag to determine if displacement is necessary within <default.vert>
 */

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUV;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;

uniform float amplitudes[3];
uniform float phases[3];
uniform float frequencies[3];
uniform vec2 directions[3];
uniform float zPosMin;
uniform float accumTime;

out vec3 tNormal;
out vec3 tFragPos;


float getDisplacement(void) {
    float displacement = 0.0f;
    for (int i = 0; i < 3; i++) {
        displacement += amplitudes[i] * sin(dot(directions[i], aPosition.yz) * frequencies[i] + accumTime * phases[i]);
    }
    return displacement * (aPosition.z / zPosMin);
}

void main(void)
{
    vec3 modifiedPos = aPosition;
    modifiedPos.x += getDisplacement(); // Displacement on x-axis
    gl_Position = uProj * uView * uModel * vec4(modifiedPos, 1.0);
    tFragPos = vec3(uModel * vec4(modifiedPos, 1.0));
    tNormal = normalize(mat3(transpose(inverse(uModel))) * aNormal);
}

