#version 410 core

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoords;
in vec4 fPosLightSpace;
in vec4 fPosEye;

out vec4 fColor;

//matrices
uniform mat4 model;
uniform mat4 view;
uniform mat3 normalMatrix;
//lighting
uniform vec3 lightDir;
uniform vec3 lightColor;

uniform vec3 pointLight;

// textures
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;


uniform vec3 point;
uniform vec3 fog;

//components
vec3 ambient;
float ambientStrength = 0.2f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;
float shadow;

float shininess = 32.0f;

float constant = 1.0f;
float linear = 0.045f;
float quadratic = 0.0073f;

float computeShadow()
{
    // perform perspective divide
    vec3 normalizedCoords = fPosLightSpace.xyz / fPosLightSpace.w;

    // Transform to [0,1] range
    normalizedCoords = normalizedCoords * 0.5 + 0.5;

    if (normalizedCoords.z > 1.0f) return 0.0f;

    // Get closest depth value from light's perspective
    float closestDepth = texture(shadowMap, normalizedCoords.xy).r;
    // Get depth of current fragment from light's perspective
    float currentDepth = normalizedCoords.z;
    //Check wheter current frag pos is in shadow
    float bias= 0.05f;
    float shadow= currentDepth - bias > closestDepth ? 1.0 : 0.0;
    return shadow;

}

vec3 CalcPointLight(vec3 light)
{
	vec3 cameraPosEye = light;//in eye coordinates, the viewer is situated at the origin
	shininess = 50.0f;
	vec3 yellow = vec3(1,1,1);
	//transform normal
	vec3 normalEye = normalize(fNormal);
	
	//compute light direction
	vec3 lightDirN = normalize(pointLight.xyz - fPosition);
	
	//compute view direction 
	vec3 viewDirN = normalize(cameraPosEye - fPosition);
	
	
	//compute half vector
	vec3 halfVector = normalize(lightDirN + viewDirN);
	
	//compute distance to light
	float dist = length(pointLight.xyz - fPosition);
	//compute attenuation
	//float att = (1.0 / (constant + linear * dist + quadratic * (dist * dist)));
	float att = 0.25f;
	
	
	//compute ambient light
	ambient = att * ambientStrength * yellow;
	ambient *= texture(diffuseTexture, fTexCoords).rgb;

	//compute diffuse light
	diffuse = att * max(dot(normalEye, lightDirN), 0.0f) * yellow;
	diffuse *= texture(diffuseTexture, fTexCoords).rgb;

	//compute specular light
	float specCoeff = pow(max(dot(normalEye, halfVector), 0.0f), shininess);
	specular = att * specularStrength * specCoeff * yellow;
	specular *= texture(specularTexture, fTexCoords).rgb;

	return ambient + diffuse + specular;
} 

vec3 computeDirLight()
{
    //compute eye space coordinates
    vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
    vec3 normalEye = normalize(normalMatrix * fNormal);

    //normalize light direction
    vec3 lightDirN = vec3(normalize(view * vec4(lightDir, 0.0f)));

    //compute view direction (in eye coordinates, the viewer is situated at the origin
    vec3 viewDir = normalize(- fPosEye.xyz);

    //compute ambient light
    ambient = ambientStrength * lightColor;

    //compute diffuse light
    diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;

    //compute specular light
    vec3 reflectDir = reflect(-lightDirN, normalEye);
    float specCoeff = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
    specular = specularStrength * specCoeff * lightColor;
	
	return ambient + diffuse + specular;
}

float computeFog()
{
	vec4 fPosEye = view * model * vec4(fPosition, 1.0f);

	float fogDensity = 0.0015f;
	float fragmentDistance = length(fPosEye);
	float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));

	return clamp(fogFactor, 0.0f, 1.0f);
}
vec4 colorE;
void main() 
{
	vec3 natural = computeDirLight();
    //compute final vertex color
	shadow = computeShadow();
    vec3 color = min((ambient + (1.0f - shadow) * diffuse) * texture(diffuseTexture, fTexCoords).rgb + (1.0f - shadow) * specular * texture(specularTexture, fTexCoords).rgb, 1.0f);
	
	vec3 res = CalcPointLight(pointLight);
	
	if (point != vec3(1.0f, 1.0f, 0.0f))
		colorE = vec4(res + color, 1.0f);
	else
		 colorE = vec4(color, 1.0f);
	
	fColor = colorE;
	
	float fogFactor = computeFog();
	vec4 fogColor = vec4(0.1f, 0.1f, 0.1f, 1.0f);
	
	//fColor = fogColor * (1 – fogFactor) + colorE * fogFactor;
	
	if (fog == vec3(1.0f, 0.0f, 0.0f))
		fColor = mix(fogColor, colorE, fogFactor);
}
