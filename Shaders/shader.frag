#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in vec4 FragPosLightSpace;

out vec4 FragColor;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform sampler2D shadowMap;

float shadowAmount(vec3 normal, vec3 lightDir)
{
    vec3 projected = FragPosLightSpace.xyz / FragPosLightSpace.w;
    projected = projected * 0.5 + 0.5;
    if (projected.z > 1.0) return 0.0;

    float bias = max(0.004 * (1.0 - dot(normal, lightDir)), 0.0012);
    vec2 texelSize = 1.0 / vec2(textureSize(shadowMap, 0));
    float shadow = 0.0;
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            float closestDepth = texture(
                shadowMap,
                projected.xy + vec2(x, y) * texelSize
            ).r;
            shadow += projected.z - bias > closestDepth ? 1.0 : 0.0;
        }
    }
    return shadow / 9.0;
}

void main()
{
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 48.0);
    float shadow = shadowAmount(norm, lightDir);
    vec3 ambient = 0.38 * objectColor;
    vec3 diffuse = (1.0 - shadow) * diff * lightColor * objectColor;
    vec3 specular = (1.0 - shadow) * 0.28 * spec * lightColor;
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}
