#version 330 core

// Material with ambient, diffuse, specular, and shininess properties
struct Material
{
    vec3 diffuse;
    vec3 ambient;
    vec3 specular;
    float shininess;
};

// Structure for a global/directional light source
struct Light
{
    vec3 lightPos;
    vec3 globalAmbientLightColor;
    vec3 lightColor;
    float ka; // ambient coefficient
    float kd; // diffuse coefficient
    float ks; // specular coefficient
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

// Uniforms
uniform Material uMaterial;
uniform Light uLight;

uniform PointLight lights[6];
uniform int numLights;

in vec3 tNormal;
in vec3 tFragPos;
in vec3 tCameraPos; // camera position needed for specular computations

out vec4 FragColor;

// Function to compute directional/global light contribution (Blinn-Phong)
vec3 directionalLight(vec3 normal, vec3 lightPos) {
    vec3 lightDir = normalize(lightPos - tFragPos);
    vec3 viewDir = normalize(tCameraPos - tFragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);

    // Ambient component
    vec3 ambientComponent = uLight.ka * uMaterial.ambient * uLight.globalAmbientLightColor;

    // Diffuse component
    float diffFactor = max(dot(normal, lightDir), 0.0);
    vec3 diffuseComponent = uLight.kd * uMaterial.diffuse * uLight.lightColor * diffFactor;

    // Specular component (Blinn-Phong)
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
    vec3 finalColor = lightResult + pointLightResult;

    FragColor = vec4(finalColor, 1.0);
}
