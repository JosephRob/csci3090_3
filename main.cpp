
// OpenGL libraries
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/constants.hpp>
#include <SOIL.h>

#include <time.h>

// GUI Library
#include <imgui.h>
#include <imgui_impl_glfw_gl3.h>

// Standard Libraries
#include <iostream> // Used for std::cout
#include <vector>   // Used for std::vector<vec3>
#include <map>      // Used for std::map


// Custom headers
#include "shaders.h"
#include "mesh.h"
#include "Camera.h"

using namespace glm;

/*---------------------------- Variables ----------------------------*/
// GLFW window
GLFWwindow* window;
int width = 1280, height = 720;
float XP=0,YP=10,ZP=1, dist=15.0f;

// Shader programs
GLuint phongProgram, skyboxProgram, emissiveProgram;

// Variables for uniforms
mat4 projectionMatrix, viewMatrix, modelMatrix[10];
vec3 cameraPosition, cameraTarget, lightPosition;
float specPower = 20.0f;

// Solar system variables
float earthDays = 17.62f;
float moonRotation = 0.0f;
float simulationSpeed = 10.0f;
int viewMode = 3;

// Textures
GLuint skyboxTexture;
GLuint diffuseTexture, specularTexture, noSpecularTexture;
GLuint moonTexture, sunTexture, mercuryTexture, venusTexture, marsTexture, jupiterTexture, saturnTexture, uranusTexture, neptuneTexture, plutoTexture, rockTexture;

//rock
std::vector <vec3> rocksP, rocksV;
std::vector <mat4> rocks;
bool many = false;
bool bright = false;
int rockLock = 0;

// Camera
Camera  camera(glm::vec3(0.0f, 50.0f, 30.0f));

enum
{
    EARTH = 0,
    SUN = 1,
    MOON = 2,
	MERCURY = 3,
	VENUS = 4,
	MARS = 5,
	JUPITER = 6,
	SATURN = 7,
	URANUS = 8,
	NEPTUNE = 9
};

double distance(vec3 a, vec3 b) {
	return sqrt(pow((b[0] - a[0]), 2.0f) + pow((b[1] - a[1]), 2.0f)+ pow((b[2] - a[2]), 2.0f));
}

vec3 Gravity(vec3 P1, vec3 P2, double M1, double M2) {
	double G = 500;
	vec3 grav = vec3(0);
	vec3 dir = normalize(vec3(P2[0] - P1[0], P2[1] - P1[1], P2[2] - P1[2]));
	grav=dir * (float)(G*M1*M2/pow(distance(P2,P1),2));
	//mass is equal to radius
	return grav;
}

void Initialize()
{
    // Make a simple shader for the sphere we're drawing
    {
        GLuint vs = buildShader(GL_VERTEX_SHADER, ASSETS"simpleLights.vert");
        GLuint fs = buildShader(GL_FRAGMENT_SHADER, ASSETS"simpleLights.frag");
        phongProgram = buildProgram(vs, fs, 0);
        phongProgram = linkProgram(phongProgram);
        dumpProgram(phongProgram, "Simple program for phong lighting");
    }

    // Make a simple shader for the skybox
    {
        GLuint vs = buildShader(GL_VERTEX_SHADER, ASSETS"skybox.vert");
        GLuint fs = buildShader(GL_FRAGMENT_SHADER, ASSETS"skybox.frag");
        skyboxProgram = buildProgram(vs, fs, 0);
        skyboxProgram = linkProgram(skyboxProgram);
        dumpProgram(skyboxProgram, "Simple program for the skybox");
    }

    // Make a simple shader for the sun
    {
        GLuint vs = buildShader(GL_VERTEX_SHADER, ASSETS"emissive.vert");
        GLuint fs = buildShader(GL_FRAGMENT_SHADER, ASSETS"emissive.frag");
        emissiveProgram = buildProgram(vs, fs, 0);
        emissiveProgram = linkProgram(emissiveProgram);
        dumpProgram(emissiveProgram, "Simple program for the sun");
    }

    // Load in all 6 faces of the skybox cube
    skyboxTexture = SOIL_load_OGL_cubemap
    (
        ASSETS"textures/star_sky/stars.png", // posx
        ASSETS"textures/star_sky/stars.png", // negx
        ASSETS"textures/star_sky/stars.png", // posy
        ASSETS"textures/star_sky/stars.png", // negy
        ASSETS"textures/star_sky/stars.png", // posz
        ASSETS"textures/star_sky/stars.png", // negz
        SOIL_LOAD_RGB,      // This means we're expecting it to have RGB channels
        SOIL_CREATE_NEW_ID, // This means we want to create a new texture instead of overwriting one 
        SOIL_FLAG_MIPMAPS   // This means we want it to generate mip-maps.
    );

    diffuseTexture = SOIL_load_OGL_texture
    (
        ASSETS"textures/earthDiffuse.png",
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
    );

	specularTexture = SOIL_load_OGL_texture
	(
		ASSETS"textures/earthSpecular.png",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	);
	noSpecularTexture = SOIL_load_OGL_texture
	(
		ASSETS"textures/black.png",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	);

    moonTexture = SOIL_load_OGL_texture
    (
        ASSETS"textures/moonTexture.png",
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
    );

	sunTexture = SOIL_load_OGL_texture
	(
		ASSETS"textures/sunTexture.png",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	);
	//planets
	mercuryTexture = SOIL_load_OGL_texture
	(
		ASSETS"textures/mercuryTexture.png",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	);
	venusTexture = SOIL_load_OGL_texture
	(
		ASSETS"textures/venusTexture.png",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	);
	marsTexture = SOIL_load_OGL_texture
	(
		ASSETS"textures/marsTexture.png",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	);
	jupiterTexture = SOIL_load_OGL_texture
	(
		ASSETS"textures/jupiterTexture.png",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	);
	saturnTexture = SOIL_load_OGL_texture
	(
		ASSETS"textures/saturnTexture.png",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	);
	uranusTexture = SOIL_load_OGL_texture
	(
		ASSETS"textures/uranusTexture.png",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	);
	neptuneTexture = SOIL_load_OGL_texture
	(
		ASSETS"textures/neptuneTexture.png",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	);
	plutoTexture = SOIL_load_OGL_texture
	(
		ASSETS"textures/checker.png",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	);
	rockTexture = SOIL_load_OGL_texture
	(
		ASSETS"textures/rockTexture.png",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	);

    //cameraPosition = vec3(0, 0, -5);
    //cameraTarget = vec3(0, 0, 0);

	srand(time(0));
}

void Update(float deltaTime)
{
    mat4 identity, translation, scaling, rotation;
    identity = mat4(1.0f);
    const float pi2 = 2.0f * pi<float>();

    // Add to the rotation, in days.
	if (viewMode == 4) deltaTime = 0;
    earthDays += deltaTime * simulationSpeed;

    float planetRotations = earthDays;


    // Compute orbits and rotations
    float earthOrbit    = -(planetRotations / 365.0f) * pi2;
    float earthRotate   = -fract(planetRotations) * pi2;
	float sunRotate		= -fract(planetRotations / 27.0f) * pi2;
	float moonOrbit		= (planetRotations / 27.322f) * pi2;
	float moonRotate	= fract(planetRotations / 27.0f) * pi2;
	float mercuryOrbit	= -(planetRotations / 88.0f) * pi2;
	float mercuryRotate = fract(planetRotations / 58.6) * pi2;
	float venusOrbit	= -(planetRotations / 225.0f) * pi2;
	float venusRotate	= fract(planetRotations / 116.75f) * pi2;
	float marsOrbit		= -(planetRotations / 686.0f) * pi2;
	float marsRotate	= fract(planetRotations / 58.6) * pi2;
	float jupiterOrbit	= -(planetRotations / (12.0f*356.0f)) * pi2 + 4.8;
	float jupiterRotate = fract(planetRotations / 0.375) * pi2;
	float saturnOrbit	= -(planetRotations / (29.0f*365.0f)) * pi2 + 0.5;
	float saturnRotate	= fract(planetRotations / 0.4f) * pi2;
	float uranusOrbit	= -(planetRotations / (84.0f*365.0f)) * pi2 + 1.3;
	float uranusRotate	= fract(planetRotations / 0.7f) * pi2;
	float neptuneOrbit	= -(planetRotations / (165.0f*365.0f)) * pi2 + 3.0;
	float neptuneRotate = fract(planetRotations / 0.75) * pi2;

	double radius[] = { 2.5, 5.0, 1, 1.75, 1.75, 1.75, 1.75, 1.75, 1.75, 1.75, 1.75 };

    vec3 earthPosition = vec3(cos(earthOrbit), 0, sin(earthOrbit)) * 10.0f;
    vec3 moonPosition = earthPosition + vec3(cos(moonOrbit), 0, sin(moonOrbit)) * 2.0f;
	vec3 mercuryPosition = vec3(cos(mercuryOrbit), 0, sin(mercuryOrbit)) * 5.0f;
	vec3 venusPosition = vec3(cos(venusOrbit), 0, sin(venusOrbit)) * 7.5f;
	vec3 marsPosition = vec3(cos(marsOrbit), 0, sin(marsOrbit)) * 12.5f;
	vec3 jupiterPosition = vec3(cos(jupiterOrbit), 0, sin(jupiterOrbit)) * 15.0f;
	vec3 saturnPosition = vec3(cos(saturnOrbit), 0, sin(saturnOrbit)) * 17.5f;
	vec3 uranusPosition = vec3(cos(uranusOrbit), 0, sin(uranusOrbit)) * 20.0f;
	vec3 neptunePosition = vec3(cos(neptuneOrbit), 0, sin(neptuneOrbit)) * 22.5f;

    translation = translate(identity, earthPosition);
    scaling = scale(identity, vec3(radius[EARTH])); // The earth stays the same size
    rotation = rotate(identity, earthRotate, vec3(0, 1, 0));
    modelMatrix[EARTH] = translation * rotation * scaling;

	translation = translate(identity, moonPosition);
	scaling = scale(identity, vec3(radius[MOON])); // The moon is 27% the size of earth
	rotation = rotate(identity, moonRotate, vec3(0, 1, 0));
	modelMatrix[MOON] = translation * rotation * scaling;

	translation = translate(identity, mercuryPosition);
	scaling = scale(identity, vec3(radius[MERCURY]));
	rotation = rotate(identity, mercuryRotate, vec3(0, 1, 0));
	modelMatrix[MERCURY] = translation * rotation * scaling;

	translation = translate(identity, venusPosition);
	scaling = scale(identity, vec3(radius[VENUS]));
	rotation = rotate(identity, venusRotate, vec3(0, 1, 0));
	modelMatrix[VENUS] = translation * rotation * scaling;

	translation = translate(identity, marsPosition);
	scaling = scale(identity, vec3(radius[MARS]));
	rotation = rotate(identity, marsRotate, vec3(0, 1, 0));
	modelMatrix[MARS] = translation * rotation * scaling;

	translation = translate(identity, jupiterPosition);
	scaling = scale(identity, vec3(radius[JUPITER]));
	rotation = rotate(identity, jupiterRotate, vec3(0, 1, 0));
	modelMatrix[JUPITER] = translation * rotation * scaling;

	translation = translate(identity, saturnPosition);
	scaling = scale(identity, vec3(radius[SATURN]));
	rotation = rotate(identity, saturnRotate, vec3(0, 1, 0));
	modelMatrix[SATURN] = translation * rotation * scaling;

	translation = translate(identity, uranusPosition);
	scaling = scale(identity, vec3(radius[URANUS]));
	rotation = rotate(identity, uranusRotate, vec3(0, 1, 0));
	modelMatrix[URANUS] = translation * rotation * scaling;

	translation = translate(identity, neptunePosition);
	scaling = scale(identity, vec3(radius[NEPTUNE]));
	rotation = rotate(identity, neptuneRotate, vec3(0, 1, 0));
	modelMatrix[NEPTUNE] = translation * rotation * scaling;

    scaling = scale(identity, vec3(radius[SUN]));
	rotation = rotate(identity, -sunRotate, vec3(0, 1, 0));
    modelMatrix[SUN] = rotation * scaling;

	for (int x = 0;x < rocks.size();x++) {
		if(viewMode!=4)
			rocksP[x] += rocksV[x] * simulationSpeed / 10000.0f;
		
		//gravity
		vec3 Acc =vec3(0.0);
		Acc += Gravity(rocksP[x], earthPosition, 1, radius[EARTH]);
		Acc += Gravity(rocksP[x], moonPosition, 1, radius[MOON]);
		Acc += Gravity(rocksP[x], vec3(0), 1, radius[SUN]);
		Acc += Gravity(rocksP[x], mercuryPosition, 1, radius[MERCURY]);
		Acc += Gravity(rocksP[x], marsPosition, 1, radius[MARS]);
		Acc += Gravity(rocksP[x], venusPosition, 1, radius[VENUS]);
		Acc += Gravity(rocksP[x], jupiterPosition, 1, radius[JUPITER]);
		Acc += Gravity(rocksP[x], saturnPosition, 1, radius[SATURN]);
		Acc += Gravity(rocksP[x], neptunePosition, 1, radius[NEPTUNE]);
		Acc += Gravity(rocksP[x], uranusPosition, 1, radius[URANUS]);

		//todo other rocks

		rocksV[x] += Acc * simulationSpeed / 50.0f;

		translation = translate(identity, rocksP.at(x));
		scaling = scale(identity, vec3(1));
		rotation = rotate(identity, (float)x, vec3(0, 1, 0));
		rocks.at(x)= translation * rotation * scaling;
		bool hit = false;
		if (distance(vec3(0.0), rocksP[x]) < 1 + radius[SUN] / 2) {//sun
			hit = true;
			printf("hit sun\n");
		}
		else if (distance(earthPosition, rocksP[x]) < 1 + radius[EARTH] / 2) {//earth
			hit = true;
			printf("hit earth\n");
		}
		else if (distance(moonPosition, rocksP[x]) < 1 + radius[MOON] / 2) {//moon
			hit = true;
			printf("hit moon\n");
		}
		else if (distance(marsPosition, rocksP[x]) < 1 + radius[MARS] / 2) {//mars
			hit = true;
			printf("hit mars\n");
		}
		else if (distance(mercuryPosition, rocksP[x]) < 1 + radius[MERCURY] / 2) {//mercury
			hit = true;
			printf("hit mercury\n");
		}
		else if (distance(venusPosition, rocksP[x]) < 1 + radius[VENUS] / 2) {//venus
			hit = true;
			printf("hit venus\n");
		}
		else if (distance(jupiterPosition, rocksP[x]) < 1 + radius[JUPITER] / 2) {//jupiter
			hit = true;
			printf("hit jupiter\n");
		}
		else if (distance(saturnPosition, rocksP[x]) < 1 + radius[SATURN] / 2) {//saturn
			hit = true;
			printf("hit saturn\n");
		}
		else if (distance(uranusPosition, rocksP[x]) < 1 + radius[URANUS] / 2) {//uranus
			hit = true;
			printf("hit uranus\n");
		}
		else if (distance(neptunePosition, rocksP[x]) < 1 + radius[NEPTUNE] / 2) {//neptune
			hit = true;
			printf("hit neptune\n");
		}
		else if (distance(vec3(0.0), rocksP[x]) > 100) {
			hit = true;
			printf("gone forever\n");
		}
		if (hit) {
			rocks.erase(rocks.begin() + x);
			rocksP.erase(rocksP.begin() + x);
			rocksV.erase(rocksV.begin() + x);
			x--;
			printf("\t%i left\n",rocksP.size());
		}
	}

    /*if (viewMode == 0)
    {   // Look from in front of the earth, at the earth
        viewMatrix = inverse(lookAt(earthPosition + normalize(vec3(-3.0f, 1.0f, 3.0f)) * 5.0f, earthPosition, vec3(0, 1, 0)));
    }
    else if (viewMode == 1)
    {   // Look from behind the earth, at the earth and moon
        viewMatrix = inverse(lookAt(earthPosition + normalize(vec3(-3.0f, -2.0f, 3.0f)) * -5.0f, (earthPosition + moonPosition) * 0.5f, vec3(0, 1, 0)));
    }
    else if (viewMode == 2)
    {   // Look from the sun, at the earth and moon
        viewMatrix = inverse(lookAt(vec3(0.0f), (earthPosition + moonPosition) * 0.5f, vec3(0, 1, 0)));
    }
    else if (viewMode == 3)
    {*/   // Look from the moon, at the earth
	
	//vec3 pos = dist * 5.0f * normalize(vec3(0,10,3));//dist * normalize(vec3(cos(A1), sin(A2), sin(A1))) * 5.0f;
	viewMatrix = /*camera.GetViewMatrix();*/inverse(lookAt(camera.GetPosition(), vec3(0), vec3(0, 1, 0)));
	cameraPosition = camera.GetPosition();
	for (int x = 0;x < 4;x++) {
		for (int y = 0;y < 4;y++) {
			printf("%f ",viewMatrix[x][y]);
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;
	/*}
    else if (viewMode == 4)
    {   // Static view
        viewMatrix = inverse(lookAt(moonPosition, (earthPosition), vec3(0, 1, 0)));
    }*/

	//spawn rock
	if (GetAsyncKeyState('P') & 0x8000 && (rockLock!=time(0) || many)) {
		double a1=rand()*10000.0f, a2=rand()*10000.0f;
		rockLock = time(0);
		vec3 pos= dist * normalize(vec3(cos(a1), sin(a2), sin(a1))) * 2.0f;
		rocksP.push_back(pos);
		rocksV.push_back(vec3(rand()%10-5,rand()%10-5,rand()%10-5)*0.1f);
		rocks.push_back(mat4(0.0));
		std::cout << "create rock " << rocksP.size() << " at ("<<cos(a1) << ", " << sin(a2) << ", " << sin(a1) << ")" << std::endl;
	}
	//controls FIX ME LATER I NEED TO USE MOUSE AND FREE MOVMENT
	if (GetAsyncKeyState('D') & 0x8000) {
		std::cout << "right" << std::endl;
		camera.ProcessKeyboard(RIGHT, deltaTime);
	}
	if (GetAsyncKeyState('A') & 0x8000) {
		std::cout << "left" << std::endl;
		camera.ProcessKeyboard(LEFT, deltaTime);
	}
	if (GetAsyncKeyState('W') & 0x8000) {
		std::cout << "up" << std::endl;
		camera.ProcessKeyboard(FORWARD, deltaTime);
	}
	if (GetAsyncKeyState('S') & 0x8000) {
		std::cout << "down" << std::endl;
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	}/*
	if (GetAsyncKeyState('O') & 0x8000) {
		std::cout << "out" << std::endl;
		dist -= 0.1;
	}
	if (GetAsyncKeyState('I') & 0x8000) {
		std::cout << "in" << std::endl;
		dist -= 0.1;
	}*/

    //std::cout << planetRotations << std::endl;
}

void Render()
{
    //------------------------------------------------------------------------------------------------ Draw Skybox

    {
        // Use the special skybox program
        glUseProgram(skyboxProgram);                                    // <- Use the skybox shader program. This has the vertex and fragment  shader for the skybox

        // Getting uniform locations  
        GLuint sLoc = glGetUniformLocation(skyboxProgram, "skybox");    // <- Get the uniform location for the skybox
        GLuint vLoc = glGetUniformLocation(skyboxProgram, "view");      // <- Get the uniform location for the view matrix
        GLuint pLoc = glGetUniformLocation(skyboxProgram, "proj");      // <- Get the uniform location for the projection matrix

        // Binding skybox texture
        glUniform1i(sLoc, 0);                                           // <- 1) Get the uniform location for the cubemap sampler, and set it to index zero                       
        glActiveTexture(GL_TEXTURE0);                                   // <- 2) Set the active texture to also be index zero, matching above                             
        glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);              // <- 3) Bind the skybox texture. This texture is bound to zero, so it will be sampled              
                                   
        // Passing up view-projection matrix
        glUniformMatrix4fv(vLoc, 1, GL_FALSE,                           // <- Pass through a special version of the view matrix. This has no position information, as
            &inverse(mat4(mat3(viewMatrix)))[0][0]);                    //    the position was removed by downcasting to mat3, then back up to mat4. It's inverted as well
        glUniformMatrix4fv(pLoc, 1, GL_FALSE, &projectionMatrix[0][0]); // <- Pass through the projection matrix here to the vertex shader

        // Drawing the skybox
        Primitive::DrawSkybox();                                        // <- Draw the skybox here. It's an inverted cube around the camera                                     
                          
        // Unbinding the texture and program
        glBindTexture(GL_TEXTURE_CUBE_MAP, GL_NONE);                    // <- Unbind the texture after we've drawn the skybox here                                  
        glUseProgram(GL_NONE);                                          // <- Unbind the shader program after we've used it here                                    
    }

    //------------------------------------------------------------------------------------------------ Draw Models

    {   //----------------------------------------------------------- EARTH ----------------------------------------------------------------------
        // Use the phong program
        glUseProgram(phongProgram);                                         // <- Use the phong lighting shader program

        // Getting uniform locations
        GLuint dtLoc = glGetUniformLocation(phongProgram, "diffuseTex");    // <- Get the uniform location for the diffuse texture
        GLuint stLoc = glGetUniformLocation(phongProgram, "specularTex");   // <- Get the uniform location for the diffuse texture
        GLuint cLoc = glGetUniformLocation(phongProgram, "cameraPos");      // <- Get the uniform location for the projection matrix
        GLuint mLoc = glGetUniformLocation(phongProgram, "model");          // <- Get the uniform location for the model matrix
        GLuint vLoc = glGetUniformLocation(phongProgram, "view");           // <- Get the uniform location for the view matrix
        GLuint pLoc = glGetUniformLocation(phongProgram, "proj");           // <- Get the uniform location for the projection matrix
		GLuint nLoc = glGetUniformLocation(phongProgram, "norm");           // <- Get the uniform location for the normal matrix
		GLuint specLoc = glGetUniformLocation(phongProgram, "specularPower");

		glUniform1i(glGetUniformLocation(phongProgram, "bright"),bright);

		glUniform1f(specLoc,specPower);
        // Binding diffuse texture
        glUniform1i(dtLoc, 0);                                              // <- 1) Get the uniform location for the 2D sampler, and set it to index zero                       
        glActiveTexture(GL_TEXTURE0);                                       // <- 2) Set the active texture to also be index zero, matching above                             
        glBindTexture(GL_TEXTURE_2D, diffuseTexture);                       // <- 3) Bind the diffuse texture (bound to index 0)

        // Binding specular texture
        glUniform1i(stLoc, 1);                                                                     
        glActiveTexture(GL_TEXTURE1);                                                           
        glBindTexture(GL_TEXTURE_2D, specularTexture);                      

        // Passing MVP matrix
        glUniformMatrix4fv(mLoc, 1, GL_FALSE, &modelMatrix[EARTH][0][0]);   // <- Pass through the model matrix here to the vertex shader
        glUniformMatrix4fv(vLoc, 1, GL_FALSE, &inverse(viewMatrix)[0][0]);  // <- Pass through the inverse of the view matrix here to the vertex shader
        glUniformMatrix4fv(pLoc, 1, GL_FALSE, &projectionMatrix[0][0]);     // <- Pass through the projection matrix here to the vertex shader
        glUniformMatrix4fv(nLoc, 1, GL_FALSE,                               // <- Pass through the transpose of the inverse of the model matrix
            &transpose(inverse(modelMatrix[EARTH]))[0][0]);                 //    so that we correctly transform the normals into world space as well

        // Passing up additional information
        glUniform3fv(cLoc, 1, &cameraPosition[0]);                          // <- Pass through the camera location to the shader

        Primitive::DrawSphere();    // Earth

        // Unbinding textures
        glActiveTexture(GL_TEXTURE1);                               // <- Set Texture 1 to be active, and then                
        glBindTexture(GL_TEXTURE_2D, GL_NONE);                      // <- unbind it here
        glActiveTexture(GL_TEXTURE0);                               // <- Set Texture 0 to be active, and then         
        glBindTexture(GL_TEXTURE_2D, GL_NONE);                      // <- unbind it here            

        //----------------------------------------------------------- THE MOON (see above for comments) --------------------------------------------------

        // Binding diffuse texture
        glUniform1i(dtLoc, 0);                                                           
        glActiveTexture(GL_TEXTURE0);                                                 
        glBindTexture(GL_TEXTURE_2D, moonTexture);                          

        // Binding specular texture
        glUniform1i(stLoc, 1);                                                          
        glActiveTexture(GL_TEXTURE1);                                                
        glBindTexture(GL_TEXTURE_2D, moonTexture);                          

        // Passing MVP matrix
        glUniformMatrix4fv(mLoc, 1, GL_FALSE, &modelMatrix[MOON][0][0]);    
        glUniformMatrix4fv(vLoc, 1, GL_FALSE, &inverse(viewMatrix)[0][0]);  
        glUniformMatrix4fv(pLoc, 1, GL_FALSE, &projectionMatrix[0][0]);     
        glUniformMatrix4fv(nLoc, 1, GL_FALSE,                               
            &transpose(inverse(modelMatrix[MOON]))[0][0]);                  

        // Passing up additional information
        glUniform3fv(cLoc, 1, &cameraPosition[0]);                     

        Primitive::DrawSphere();    // Moon


		//----------------------------------------------------------- Mercury
		// Binding diffuse texture
		glUniform1i(dtLoc, 0);                                              // <- 1) Get the uniform location for the 2D sampler, and set it to index zero                       
		glActiveTexture(GL_TEXTURE0);                                       // <- 2) Set the active texture to also be index zero, matching above                             
		glBindTexture(GL_TEXTURE_2D, mercuryTexture);                       // <- 3) Bind the diffuse texture (bound to index 0)

																		  // Binding specular texture
		glUniform1i(stLoc, 1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, noSpecularTexture);

		// Passing MVP matrix
		glUniformMatrix4fv(mLoc, 1, GL_FALSE, &modelMatrix[MERCURY][0][0]);   // <- Pass through the model matrix here to the vertex shader
		glUniformMatrix4fv(vLoc, 1, GL_FALSE, &inverse(viewMatrix)[0][0]);  // <- Pass through the inverse of the view matrix here to the vertex shader
		glUniformMatrix4fv(pLoc, 1, GL_FALSE, &projectionMatrix[0][0]);     // <- Pass through the projection matrix here to the vertex shader
		glUniformMatrix4fv(nLoc, 1, GL_FALSE,                               // <- Pass through the transpose of the inverse of the model matrix
			&transpose(inverse(modelMatrix[MERCURY]))[0][0]);                 //    so that we correctly transform the normals into world space as well

																			// Passing up additional information
		glUniform3fv(cLoc, 1, &cameraPosition[0]);                          // <- Pass through the camera location to the shader

		Primitive::DrawSphere();

		// Unbinding textures
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, GL_NONE);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, GL_NONE);
        //----------------------------------------------------------- Venus
		// Binding diffuse texture
		glUniform1i(dtLoc, 0);                                              // <- 1) Get the uniform location for the 2D sampler, and set it to index zero                       
		glActiveTexture(GL_TEXTURE0);                                       // <- 2) Set the active texture to also be index zero, matching above                             
		glBindTexture(GL_TEXTURE_2D, venusTexture);                       // <- 3) Bind the diffuse texture (bound to index 0)

																			// Binding specular texture
		glUniform1i(stLoc, 1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, noSpecularTexture);

		// Passing MVP matrix
		glUniformMatrix4fv(mLoc, 1, GL_FALSE, &modelMatrix[VENUS][0][0]);   // <- Pass through the model matrix here to the vertex shader
		glUniformMatrix4fv(vLoc, 1, GL_FALSE, &inverse(viewMatrix)[0][0]);  // <- Pass through the inverse of the view matrix here to the vertex shader
		glUniformMatrix4fv(pLoc, 1, GL_FALSE, &projectionMatrix[0][0]);     // <- Pass through the projection matrix here to the vertex shader
		glUniformMatrix4fv(nLoc, 1, GL_FALSE,                               // <- Pass through the transpose of the inverse of the model matrix
			&transpose(inverse(modelMatrix[VENUS]))[0][0]);                 //    so that we correctly transform the normals into world space as well

																			// Passing up additional information
		glUniform3fv(cLoc, 1, &cameraPosition[0]);                          // <- Pass through the camera location to the shader

		Primitive::DrawSphere();    

		// Unbinding textures
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, GL_NONE);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, GL_NONE);

		//----------------------------------------------------------- mars// Binding diffuse texture
		glUniform1i(dtLoc, 0);                                              // <- 1) Get the uniform location for the 2D sampler, and set it to index zero                       
		glActiveTexture(GL_TEXTURE0);                                       // <- 2) Set the active texture to also be index zero, matching above                             
		glBindTexture(GL_TEXTURE_2D, marsTexture);                       // <- 3) Bind the diffuse texture (bound to index 0)

																		 // Binding specular texture
		glUniform1i(stLoc, 1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, noSpecularTexture);

		// Passing MVP matrix
		glUniformMatrix4fv(mLoc, 1, GL_FALSE, &modelMatrix[MARS][0][0]);   // <- Pass through the model matrix here to the vertex shader
		glUniformMatrix4fv(vLoc, 1, GL_FALSE, &inverse(viewMatrix)[0][0]);  // <- Pass through the inverse of the view matrix here to the vertex shader
		glUniformMatrix4fv(pLoc, 1, GL_FALSE, &projectionMatrix[0][0]);     // <- Pass through the projection matrix here to the vertex shader
		glUniformMatrix4fv(nLoc, 1, GL_FALSE,                               // <- Pass through the transpose of the inverse of the model matrix
			&transpose(inverse(modelMatrix[MARS]))[0][0]);                 //    so that we correctly transform the normals into world space as well

																		   // Passing up additional information
		glUniform3fv(cLoc, 1, &cameraPosition[0]);                          // <- Pass through the camera location to the shader

		Primitive::DrawSphere();

		// Unbinding textures
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, GL_NONE);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, GL_NONE);

		//----------------------------------------------------------- jupiter// Binding diffuse texture
		glUniform1i(dtLoc, 0);                                              // <- 1) Get the uniform location for the 2D sampler, and set it to index zero                       
		glActiveTexture(GL_TEXTURE0);                                       // <- 2) Set the active texture to also be index zero, matching above                             
		glBindTexture(GL_TEXTURE_2D, jupiterTexture);                       // <- 3) Bind the diffuse texture (bound to index 0)

																		 // Binding specular texture
		glUniform1i(stLoc, 1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, noSpecularTexture);

		// Passing MVP matrix
		glUniformMatrix4fv(mLoc, 1, GL_FALSE, &modelMatrix[JUPITER][0][0]);   // <- Pass through the model matrix here to the vertex shader
		glUniformMatrix4fv(vLoc, 1, GL_FALSE, &inverse(viewMatrix)[0][0]);  // <- Pass through the inverse of the view matrix here to the vertex shader
		glUniformMatrix4fv(pLoc, 1, GL_FALSE, &projectionMatrix[0][0]);     // <- Pass through the projection matrix here to the vertex shader
		glUniformMatrix4fv(nLoc, 1, GL_FALSE,                               // <- Pass through the transpose of the inverse of the model matrix
			&transpose(inverse(modelMatrix[JUPITER]))[0][0]);                 //    so that we correctly transform the normals into world space as well

																		   // Passing up additional information
		glUniform3fv(cLoc, 1, &cameraPosition[0]);                          // <- Pass through the camera location to the shader

		Primitive::DrawSphere();

		// Unbinding textures
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, GL_NONE);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, GL_NONE);
		//----------------------------------------------------------- saturn// Binding diffuse texture
		glUniform1i(dtLoc, 0);                                              // <- 1) Get the uniform location for the 2D sampler, and set it to index zero                       
		glActiveTexture(GL_TEXTURE0);                                       // <- 2) Set the active texture to also be index zero, matching above                             
		glBindTexture(GL_TEXTURE_2D, saturnTexture);                       // <- 3) Bind the diffuse texture (bound to index 0)

																		 // Binding specular texture
		glUniform1i(stLoc, 1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, noSpecularTexture);

		// Passing MVP matrix
		glUniformMatrix4fv(mLoc, 1, GL_FALSE, &modelMatrix[SATURN][0][0]);   // <- Pass through the model matrix here to the vertex shader
		glUniformMatrix4fv(vLoc, 1, GL_FALSE, &inverse(viewMatrix)[0][0]);  // <- Pass through the inverse of the view matrix here to the vertex shader
		glUniformMatrix4fv(pLoc, 1, GL_FALSE, &projectionMatrix[0][0]);     // <- Pass through the projection matrix here to the vertex shader
		glUniformMatrix4fv(nLoc, 1, GL_FALSE,                               // <- Pass through the transpose of the inverse of the model matrix
			&transpose(inverse(modelMatrix[SATURN]))[0][0]);                 //    so that we correctly transform the normals into world space as well

																		   // Passing up additional information
		glUniform3fv(cLoc, 1, &cameraPosition[0]);                          // <- Pass through the camera location to the shader

		Primitive::DrawSphere();

		// Unbinding textures
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, GL_NONE);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, GL_NONE);

		//----------------------------------------------------------- uranus// Binding diffuse texture
		glUniform1i(dtLoc, 0);                                              // <- 1) Get the uniform location for the 2D sampler, and set it to index zero                       
		glActiveTexture(GL_TEXTURE0);                                       // <- 2) Set the active texture to also be index zero, matching above                             
		glBindTexture(GL_TEXTURE_2D, uranusTexture);                       // <- 3) Bind the diffuse texture (bound to index 0)

																		 // Binding specular texture
		glUniform1i(stLoc, 1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, noSpecularTexture);

		// Passing MVP matrix
		glUniformMatrix4fv(mLoc, 1, GL_FALSE, &modelMatrix[URANUS][0][0]);   // <- Pass through the model matrix here to the vertex shader
		glUniformMatrix4fv(vLoc, 1, GL_FALSE, &inverse(viewMatrix)[0][0]);  // <- Pass through the inverse of the view matrix here to the vertex shader
		glUniformMatrix4fv(pLoc, 1, GL_FALSE, &projectionMatrix[0][0]);     // <- Pass through the projection matrix here to the vertex shader
		glUniformMatrix4fv(nLoc, 1, GL_FALSE,                               // <- Pass through the transpose of the inverse of the model matrix
			&transpose(inverse(modelMatrix[URANUS]))[0][0]);                 //    so that we correctly transform the normals into world space as well

																		   // Passing up additional information
		glUniform3fv(cLoc, 1, &cameraPosition[0]);                          // <- Pass through the camera location to the shader

		Primitive::DrawSphere();

		// Unbinding textures
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, GL_NONE);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, GL_NONE);

		//----------------------------------------------------------- neptune// Binding diffuse texture
		glUniform1i(dtLoc, 0);                                              // <- 1) Get the uniform location for the 2D sampler, and set it to index zero                       
		glActiveTexture(GL_TEXTURE0);                                       // <- 2) Set the active texture to also be index zero, matching above                             
		glBindTexture(GL_TEXTURE_2D, neptuneTexture);                       // <- 3) Bind the diffuse texture (bound to index 0)

																		 // Binding specular texture
		glUniform1i(stLoc, 1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, noSpecularTexture);

		// Passing MVP matrix
		glUniformMatrix4fv(mLoc, 1, GL_FALSE, &modelMatrix[NEPTUNE][0][0]);   // <- Pass through the model matrix here to the vertex shader
		glUniformMatrix4fv(vLoc, 1, GL_FALSE, &inverse(viewMatrix)[0][0]);  // <- Pass through the inverse of the view matrix here to the vertex shader
		glUniformMatrix4fv(pLoc, 1, GL_FALSE, &projectionMatrix[0][0]);     // <- Pass through the projection matrix here to the vertex shader
		glUniformMatrix4fv(nLoc, 1, GL_FALSE,                               // <- Pass through the transpose of the inverse of the model matrix
			&transpose(inverse(modelMatrix[NEPTUNE]))[0][0]);                 //    so that we correctly transform the normals into world space as well

																		   // Passing up additional information
		glUniform3fv(cLoc, 1, &cameraPosition[0]);                          // <- Pass through the camera location to the shader

		Primitive::DrawSphere();

		// Unbinding textures
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, GL_NONE);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, GL_NONE);

		//----------------------------------------------------------- rocks
		
		glUniform1i(dtLoc, 0);                                              // <- 1) Get the uniform location for the 2D sampler, and set it to index zero                       
		glActiveTexture(GL_TEXTURE0);                                       // <- 2) Set the active texture to also be index zero, matching above                             
		glBindTexture(GL_TEXTURE_2D, rockTexture);                       // <- 3) Bind the diffuse texture (bound to index 0)

																			// Binding specular texture
		glUniform1i(stLoc, 1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, noSpecularTexture);
		for (int i = 0;i < rocks.size();i++) {
			//std::cout << "\t" << i << ": " << std::endl;
			
			glUniformMatrix4fv(mLoc, 1, GL_FALSE, &rocks.at(i)[0][0]);   // <- Pass through the model matrix here to the vertex shader
			glUniformMatrix4fv(vLoc, 1, GL_FALSE, &inverse(viewMatrix)[0][0]);  // <- Pass through the inverse of the view matrix here to the vertex shader
			glUniformMatrix4fv(pLoc, 1, GL_FALSE, &projectionMatrix[0][0]);     // <- Pass through the projection matrix here to the vertex shader
			glUniformMatrix4fv(nLoc, 1, GL_FALSE,                               // <- Pass through the transpose of the inverse of the model matrix
				&transpose(inverse(rocks.at(i)))[0][0]);


			glUniform3fv(cLoc, 1, &cameraPosition[0]);                          // <- Pass through the camera location to the shader

			Primitive::DrawSphere();
		}

		// Unbinding textures
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, GL_NONE);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, GL_NONE);
		// unbinding the shader program
		glUseProgram(GL_NONE);
        //----------------------------------------------------------- THE SUN (see above for comments) ----------------------------------------------------

        glUseProgram(emissiveProgram);                                        

        mLoc = glGetUniformLocation(emissiveProgram, "model");
        vLoc = glGetUniformLocation(emissiveProgram, "view");
        pLoc = glGetUniformLocation(emissiveProgram, "proj");
        GLuint etLoc = glGetUniformLocation(emissiveProgram, "emissiveTex");

        // Binding emissive texture
        glUniform1i(etLoc, 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, sunTexture);

        // Passing MVP matrix
        glUniformMatrix4fv(mLoc, 1, GL_FALSE, &modelMatrix[SUN][0][0]);
        glUniformMatrix4fv(vLoc, 1, GL_FALSE, &inverse(viewMatrix)[0][0]);
        glUniformMatrix4fv(pLoc, 1, GL_FALSE, &projectionMatrix[0][0]);

        Primitive::DrawSphere();    // Sun

        // Unbinding textures
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, GL_NONE);

        // unbinding the shader program
        glUseProgram(GL_NONE);
    }
}

void Cleanup()
{
    // Cleanup the shader programs here
    glDeleteProgram(skyboxProgram);
    glDeleteProgram(phongProgram);

    // Cleanup the textures here
    glDeleteTextures(1, &skyboxTexture);
    glDeleteTextures(1, &diffuseTexture);
	glDeleteTextures(1, &specularTexture);
	glDeleteTextures(1, &marsTexture);
	glDeleteTextures(1, &venusTexture);
	glDeleteTextures(1, &mercuryTexture);
	glDeleteTextures(1, &jupiterTexture);
	glDeleteTextures(1, &saturnTexture);
	glDeleteTextures(1, &neptuneTexture);
	glDeleteTextures(1, &uranusTexture);
	glDeleteTextures(1, &plutoTexture);
}

void GUI()
{
    ImGui::Begin("Lab 8");
    {
        ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);

        ImGui::Spacing();
		ImGui::DragFloat("Simulation Speed", &simulationSpeed, 0.01f, 1000.0f); simulationSpeed = clamp(simulationSpeed, 0.01f, 1000.0f);
        ImGui::RadioButton("View 3", &viewMode, 2); ImGui::SameLine();

        ImGui::RadioButton("Static View", &viewMode, 4);
		ImGui::Checkbox("many", &many);
		ImGui::Checkbox("bright", &bright);
    }
    ImGui::End();
}

void OnWindowResized(GLFWwindow* win, int w, int h)
{
    // Set the viewport incase the window size changed
    width = w; height = h;
    glViewport(0, 0, width, height);
    float ratio = width / (float)height;
    projectionMatrix = perspective(camera.GetZoom(), ratio, 0.1f, 1000.0f);
}

int main()
{
    // start GL context and O/S window using the GLFW helper library
    if (!glfwInit()) {
        fprintf(stderr, "ERROR: could not start GLFW3\n");
        return 1;
    }

    window = glfwCreateWindow(width, height, "Laboratory 8", NULL, NULL);
    if (!window) {
        fprintf(stderr, "ERROR: could not open window with GLFW3\n");
        glfwTerminate();
        return 1;
    }
    glfwSetWindowSizeCallback(window, OnWindowResized);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(0); // essentially turn off vsync

    // start GL3W
    gl3wInit();

    // Resize at least once
    OnWindowResized(window, width, height);

    // Setup ImGui binding
    ImGui_ImplGlfwGL3_Init(window, true);
    ImGui::StyleColorsLight();

    // get version info
    const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
    const GLubyte* version = glGetString(GL_VERSION); // version as a string
    printf("Renderer: %s\n", renderer);
    printf("OpenGL version supported %s\n", version);

    // tell GL to only draw onto a pixel if the shape is closer to the viewer
    glEnable(GL_DEPTH_TEST); // enable depth-testing
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    glEnable(GL_CULL_FACE);
    glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"

    Initialize();

    float oldTime = 0.0f, currentTime = 0.0f, deltaTime = 0.0f;
    while (!glfwWindowShouldClose(window))
    {
        do { currentTime = (float)glfwGetTime(); } while (currentTime - oldTime < 1.0f / 120.0f);

        // update other events like input handling 
        glfwPollEvents();
        
        // Clear the screen
        glClearColor(0.96f, 0.97f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplGlfwGL3_NewFrame();

        deltaTime = currentTime - oldTime; // Difference in time
        oldTime = currentTime;
        
        // Call the helper functions
        Update(deltaTime);
        Render();
        GUI();

        // Finish by drawing the GUI
        ImGui::Render();
        glfwSwapBuffers(window);
    }

    // close GL context and any other GLFW resources
    glfwTerminate();
    ImGui_ImplGlfwGL3_Shutdown();
    Cleanup();
    return 0;
}