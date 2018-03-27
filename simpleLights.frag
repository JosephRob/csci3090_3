#version 400

out vec4 frag_colour;

in VertexData
{
	vec3 normal;
	vec3 worldPos;
	vec3 eyePos;
	vec2 texcoord;
}	inData;

uniform sampler2D diffuseTex;
uniform sampler2D specularTex; // It's already here
uniform float specularPower;
uniform bool bright;

vec3 sunPosition = vec3(0); // Sun is at the origin

void main()
{
	float luminance = 1.2f;
	vec3 light = normalize(sunPosition - inData.worldPos);
	vec3 normal = normalize(inData.normal);
	float NoL = max(0.0f, dot(normal, light));
	vec3 V = normalize(inData.worldPos - inData.eyePos);

	vec4 diffuseTexture = texture(diffuseTex, inData.texcoord);

	// Do diffuse light
	vec3 diffuse = diffuseTexture.rgb * vec3(NoL) * luminance*diffuseTexture.a;
	
	vec3 specularColor = texture(specularTex, inData.texcoord).rgb;

	// Do specular light
	vec3 R = normalize(reflect(-light, normal));
	float VoR = max(0.0f, dot(-V, R));
	vec3 specular = specularColor * pow(VoR, specularColor.r * specularPower) * (NoL > 0.0 ? 1.0 : 0.0);

	frag_colour.rgb = vec3(0.05) * diffuseTexture.rgb*diffuseTexture.a + diffuse + specular;
	if (bright){
		frag_colour.rgb = diffuseTexture.rgb;
	}
	frag_colour.a = 1.0f;
}