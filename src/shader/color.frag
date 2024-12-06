#version 330 core

struct Material
{
    vec3 diffuse;
    vec3 ambient;
    vec3 specular;
    float shininess;
};

// Struct for Scene light's properties
struct Light
{
    vec3 lightPos;
    vec3 globalAmbientLightColor;
    vec3 lightColor;
    float ka;                       // ambient coefficient  [0, 1]
    float kd;                       // diffuse coefficient  [0, 1]
    float ks;                       // specular coefficient [0, 1]
};

in vec3 tNormal;
in vec3 tFragPos;
in vec3 tCameraPos;
out vec4 FragColor;

uniform Material uMaterial;
uniform Light uLight;

// ----------------------- Function to calculate directional light ----------------------- //
// --- Code inspiration by https://learnopengl.com/Advanced-Lighting/Advanced-Lighting --- //
vec3 directionalLight(vec3 normal, vec3 lightPos) {
    vec3 lightDir = normalize(lightPos - tFragPos);
    vec3 viewDir = normalize(tCameraPos - tFragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);

    // Ambient Component: ojects always get some color from surrounding light sources
    vec3 ambientComponent = uLight.ka * uMaterial.ambient * uLight.globalAmbientLightColor;

    // Diffuse Component: The more a part of an object faces the light source the brihter it gets
    vec3 diffuseComponent = uLight.kd * uMaterial.diffuse * uLight.lightColor * dot(normal, lightDir);

    // Specular component: Considers bright spot of a light that appears on shiny objects
    vec3 specularComponent = uLight.ks * uMaterial.specular * uLight.lightColor * pow(max(dot(normal, halfwayDir), 0.0), uMaterial.shininess);

    // Blinn-Phong
    vec3 blinnPhong = ambientComponent + diffuseComponent + specularComponent;
    return blinnPhong;
}

void main(void)
{
    vec3 light = directionalLight(tNormal, uLight.lightPos);

    FragColor = vec4(light, 1.0);
}
