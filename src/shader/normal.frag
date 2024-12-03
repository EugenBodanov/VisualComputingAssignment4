#version 330 core

in vec3 tNormal;
in vec3 tFragPos;
out vec4 FragColor;

uniform vec3 uViewPos;
uniform bool isFlag;

void main(void)
{
    vec3 normal = normalize(tNormal);

    /* for flag check if normal is facing the camera */
    vec3 viewDir = normalize(uViewPos - tFragPos);
    if (isFlag && dot(normal, viewDir) < 0.0)
    {
        normal = -normal;
    }
    FragColor = vec4((normal + vec3(1.0, 1.0, 1.0)) * 0.5, 1.0);
}