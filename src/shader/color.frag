#version 330 core

struct Material
{
    vec3 diffuse;
};

struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
    float constant;
    float linear;
    float quadratic;
};

out vec4 FragColor;

uniform Material uMaterial;
uniform PointLight lights[6];
uniform int numLights;

in vec3 tNormal;
in vec3 tFragPos;

void main(void)
{
    vec3 normal = normalize(tNormal);
    vec3 fragPos = tFragPos;

    vec3 result = vec3(0.0);

    for (int i = 0; i < numLights; i++)
    {
        vec3 lightDir = normalize(lights[i].position - fragPos);
        float diff = max(dot(normal, lightDir), 0.0);

        float distance = length(lights[i].position - fragPos);
        float attenuation = lights[i].intensity / (lights[i].constant + lights[i].linear * distance + lights[i].quadratic * distance * distance);

        vec3 diffuse = diff * lights[i].color * attenuation;

        result += diffuse * uMaterial.diffuse;
    }

    FragColor = vec4(result, 1.0);
}
