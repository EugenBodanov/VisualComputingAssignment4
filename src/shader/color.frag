#version 330 core

struct Material
{
    vec3 diffuse;
    vec3 ambient;
    vec3 specular;
    vec3 emission;
    float shininess;
};

// Structure for a global/directional light source
struct Light
{
    vec3 lightPos;
    vec3 globalAmbientLightColor;
    vec3 lightColor;
    float ka;                       // ambient coefficient  [0, 1]
    float kd;                       // diffuse coefficient  [0, 1]
    float ks;                       // specular coefficient [0, 1]
};

// Structure for multiple point light sources with attenuation
struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
    float constant;
    float linear;
    float quadratic;

    vec3 direction;
    float angle;
};

uniform Material uMaterial;
uniform Light uLight;
uniform vec3 uCameraPos; // camera position needed for specular computations

uniform PointLight lights[6];
uniform int numLights;

in vec3 tNormal;
in vec3 tFragPos;

out vec4 FragColor;


// ----------------------- Function to calculate directional/global light (Blinn Phong) ----------------------- //
// --- Code inspiration by https://learnopengl.com/Advanced-Lighting/Advanced-Lighting --- //
vec3 directionalLight(vec3 normal, vec3 lightPos) {
    vec3 lightDir = normalize(lightPos - tFragPos);
    vec3 viewDir = normalize(uCameraPos - tFragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);

    // Ambient Component: ojects always get some color from surrounding light sources
    vec3 ambientComponent = uLight.ka * uMaterial.ambient * uLight.globalAmbientLightColor;

    // Diffuse Component: The more a part of an object faces the light source the brihter it gets
    vec3 diffuseComponent = uLight.kd * uMaterial.diffuse * uLight.lightColor * dot(normal, lightDir);

    // Specular component
    float specFactor = pow(max(dot(normal, halfwayDir), 0.0), uMaterial.shininess);
    vec3 specularComponent = uLight.ks * uMaterial.specular * uLight.lightColor * specFactor;

    return ambientComponent + diffuseComponent + specularComponent;
}

void main(void)
{
    vec3 normal = normalize(tNormal);

    // 1) Compute the directional/global light contribution
    vec3 lightResult = directionalLight(normal, uLight.lightPos);

    // 2) Compute the contribution from the point lights (diffuse only, as in the original code)
    vec3 pointLightResult = vec3(0.0);
    for (int i = 0; i < numLights; i++)
    {
        vec3 fragToLight = normalize(lights[i].position - tFragPos);

        // Compute the cosine of the angle between the spotlight direction and the vector from the light to the fragment
        float theta = dot(fragToLight, normalize(lights[i].direction));
        float cosCutoff = cos(lights[i].angle);

        // Check if fragment is inside the spotlight cone
        if (theta >= cosCutoff) {
            // Fragment is inside the cone, calculate lighting as usual
            float diff = max(dot(normal, fragToLight), 0.0);

            float distance = length(lights[i].position - tFragPos);
            float attenuation = lights[i].intensity / (lights[i].constant + lights[i].linear * distance + lights[i].quadratic * distance * distance);

            vec3 diffuse = diff * lights[i].color * attenuation;
            pointLightResult += diffuse * uMaterial.diffuse;
        }
    }

    // Combine directional light and point lights results
    vec3 finalColor = lightResult + pointLightResult + uMaterial.emission;

    FragColor = vec4(finalColor, 1.0);
}
