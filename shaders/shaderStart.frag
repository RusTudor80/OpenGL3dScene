#version 400 core

in vec3 fNormal;
in vec4 fPosEye;
in vec2 fTexCoords;
in vec4 fragPosLightSpace;

out vec4 fColor;

//lighting
uniform	vec3 lightDir;
uniform	vec3 lightColor;

uniform	vec3 lightDir2;
uniform	vec3 lightColor2;

//texture
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;
uniform float fog;
uniform float bias = 0.05f;

vec3 ambient;
vec3 ambient2;
float ambientStrength = 0.2f;
vec3 diffuse;
vec3 diffuse2;
vec3 specular;
vec3 specular2;
float specularStrength = 0.5f;
float shininess = 32.0f;
float shadow;

float computeShadow(){
	vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	// Transform to [0,1] range
	normalizedCoords = normalizedCoords * 0.5 + 0.5;
	if (normalizedCoords.z > 1.0f)
	return 0.0f;

	// Get closest depth value from light's perspective
	float closestDepth = texture(shadowMap, normalizedCoords.xy).r;
	float currentDepth = normalizedCoords.z;
	// Check whether current frag pos is in shadow
	float bias = max(0.05f * (1.0f - dot(fNormal, lightDir)), 0.005f);
	currentDepth-=bias;
	float shadow = currentDepth > closestDepth ? 1.0f : 0.0f;
	return shadow;
}
float computeFog()
{
 float fogDensity;
 if(fog == 0.0f)
    fogDensity = 0.0006f;
 else
    fogDensity = 0.0f;
 float fragmentDistance = length(fPosEye);
 float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));

 return clamp(fogFactor, 0.0f, 1.0f);
}
void computeLightComponents()
{		
	vec3 cameraPosEye = vec3(0.0f);//in eye coordinates, the viewer is situated at the origin
	
	//transform normal
	vec3 normalEye = normalize(fNormal);	
	
	//compute light direction
	vec3 lightDirN = normalize(lightDir);
	vec3 lightDirN2 = normalize(lightDir2);
	
	//compute view direction 
	vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);
		
	//compute ambient light
	ambient = ambientStrength * lightColor;
	ambient2 = ambientStrength * lightColor2;
	
	//compute diffuse light
	diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;
	diffuse2 = max(dot(normalEye, lightDirN2), 0.0f) * lightColor2;
	
	//compute specular light
	vec3 reflection = reflect(-lightDirN, normalEye);
	vec3 reflection2 = reflect(-lightDirN2, normalEye);
	float specCoeff = pow(max(dot(viewDirN, reflection), 0.0f), shininess);
	float specCoeff2 = pow(max(dot(viewDirN, reflection2), 0.0f), shininess);
	specular = specularStrength * specCoeff * lightColor;
	specular2 = specularStrength * specCoeff2 * lightColor2;
}

void main() 
{
	computeLightComponents();
	
	vec3 baseColor = vec3(0.9f, 0.35f, 0.0f);//orange
	
	ambient *= texture(diffuseTexture, fTexCoords).rgb;
	diffuse *= texture(diffuseTexture, fTexCoords).rgb;
	specular *= texture(specularTexture, fTexCoords).rgb;

	ambient2 *= texture(diffuseTexture, fTexCoords).rgb;
	diffuse2 *= texture(diffuseTexture, fTexCoords).rgb;
	specular2 *= texture(specularTexture, fTexCoords).rgb;
	shadow = computeShadow();
	vec3 color = min((ambient + (1.0f - shadow)*diffuse) + (1.0f - shadow)*specular, 1.0f);
	vec3 color2 = min((ambient2 + (1.0f - shadow)*diffuse2) + (1.0f - shadow)*specular2, 1.0f);
   	 fColor = vec4(color+color2, 1.0f);
	vec4 colorTest = vec4(color+color2, 1.0f);
	float fogFactor = computeFog();
	vec4 fogColor = vec4(192.0f, 192.0f, 192.0f, 1.0f);
	fColor = mix(fogColor, colorTest, fogFactor);
}

