#include "BasicSceneRenderer.h"
#include "Image.h"
#include "Prefabs.h"
#include <GL/glut.h>
#include <stdlib.h>
#include <iostream>

int level = 0;
int baseMonsterTimers[] = { 240, 200, 160, 120, 80 };
int monsterTimer = 0;

bool playerAttacking = false;
float attackBuffer = 0;
float movementBuffer = 0;
float playerAttackingCounter = 11;
float playerWalkingCounter = 3;
bool playerAttackAnimationFinished = true;
bool monsterInvincible = false;

float dodgeBuffer = 0;
float playerDodgeingCounter = 27;
bool playerDodgingUp = false;
bool playerDodgingDown = false;
bool playerFinishedDodge = false;
//bool playerDodgingBack = false;

GLfloat d;
GLfloat monsterD;
float score;
GLfloat p1x;
GLfloat p1y;
GLfloat p1z;
const int p1radius = 1;
const int p2radius = 0;

GLfloat p2x;
GLfloat p2y;
GLfloat p2z;

BasicSceneRenderer::BasicSceneRenderer()
    : mLightingModel(BLINN_PHONG_PER_FRAGMENT_DIR_LIGHT)
    , mCamera(NULL)
    , mProjMatrix(1.0f)
    , mActiveEntityIndex(0)
    , mDbgProgram(NULL)
    , mAxes(NULL)
    , mVisualizePointLights(true)
{}

//calculates distance from player to monster
void calculateDistance(bool player) {
	//set the distance to the current distance between player and monster
	//if its calculated for the player
	if (player == true) {
		d = sqrt(((p1x - p2x) * (p1x - p2x)) + ((p1y - p2y) * (p1y - p2y)) + ((p1z - p2z) * (p1z - p2z)));
	}
}

void calculateMonsterDistance() {
	monsterD = sqrt(((p1x - p2x) * (p1x - p2x)) + ((p1y - p2y) * (p1y - p2y)) + ((p1z - p2z) * (p1z - p2z)));
}

void setVSync(bool sync)
{
	// Function pointer for the wgl extention function we need to enable/disable
	// vsync
	typedef BOOL(APIENTRY *PFNWGLSWAPINTERVALPROC)(int);
	PFNWGLSWAPINTERVALPROC wglSwapIntervalEXT = 0;

	const char *extensions = (char*)glGetString(GL_EXTENSIONS);

	if (strstr(extensions, "WGL_EXT_swap_control") == 0)
	{
		return;
	}
	else
	{
		wglSwapIntervalEXT = (PFNWGLSWAPINTERVALPROC)wglGetProcAddress("wglSwapIntervalEXT");

		if (wglSwapIntervalEXT)
			wglSwapIntervalEXT(sync);
	}
}

void BasicSceneRenderer::initialize()
{
    // print usage instructions
    std::cout << "Controls:" << std::endl;
    std::cout << "  Movement:           Z/X/Arrow Keys" << std::endl;

    glClearColor(0.494117f, 0.75294f, 0.93333f, 1.0f);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    // enable blending (needed for textures with alpha channel)
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    mPrograms.resize(NUM_LIGHTING_MODELS);

    mPrograms[PER_VERTEX_DIR_LIGHT] = new ShaderProgram("shaders/PerVertexDirLight-vs.glsl",
                                                                "shaders/PerVertexDirLight-fs.glsl");
    
    mPrograms[BLINN_PHONG_PER_FRAGMENT_DIR_LIGHT] = new ShaderProgram("shaders/BlinnPhongPerFragment-vs.glsl",
                                                                      "shaders/BlinnPhongPerFragmentDirLight-fs.glsl");

    mPrograms[BLINN_PHONG_PER_FRAGMENT_POINT_LIGHT] = new ShaderProgram("shaders/BlinnPhongPerFragment-vs.glsl",
                                                                        "shaders/BlinnPhongPerFragmentPointLight-fs.glsl");

    mPrograms[BLINN_PHONG_PER_FRAGMENT_MULTI_LIGHT] = new ShaderProgram("shaders/BlinnPhongPerFragment-vs.glsl",
                                                                        "shaders/BlinnPhongPerFragmentMultiLight-fs.glsl");

    //
    // Create meshes
    //
	//TODO: Mesh Creation starts here
    mMeshes.push_back(CreateTexturedCube(1));
    mMeshes.push_back(CreateChunkyTexturedCylinder(0.5f, 1, 8));
    mMeshes.push_back(CreateSmoothTexturedCylinder(0.5f, 1, 15));

    float roomWidth = 12;
    float roomHeight = 24;
    float roomDepth = 680;
    float roomTilesPerUnit = 0.25f;

    //floor
    Mesh* cfMesh = CreateTexturedQuad(roomWidth, roomDepth, roomWidth * roomTilesPerUnit, roomDepth * roomTilesPerUnit);
    mMeshes.push_back(cfMesh);

	//player sprite
	Mesh* flMesh = CreateTexturedQuad(1, 1, 1, 1);
	mMeshes.push_back(flMesh);

	Mesh* dragonMesh = CreateTexturedQuad(4, 4, 1, 1);
	mMeshes.push_back(dragonMesh);

    //TODO: Load textures
    std::vector<std::string> texNames;
	texNames.push_back("textures/Road.tga");
    texNames.push_back("textures/monsters/Dragon.tga");
    texNames.push_back("textures/Rock.tga");
	
	//player walk cycle, has 8 frames
	texNames.push_back("textures/playerWalk/0.tga");
	texNames.push_back("textures/playerWalk/1.tga");
	texNames.push_back("textures/playerWalk/2.tga");
	texNames.push_back("textures/playerWalk/3.tga");
	texNames.push_back("textures/playerWalk/4.tga");
	texNames.push_back("textures/playerWalk/5.tga");
	texNames.push_back("textures/playerWalk/6.tga");
	texNames.push_back("textures/playerWalk/7.tga");

	//player attack cycle, has 16 frames
	texNames.push_back("textures/playerAttack/0.tga");
	texNames.push_back("textures/playerAttack/1.tga");
	texNames.push_back("textures/playerAttack/2.tga");
	texNames.push_back("textures/playerAttack/3.tga");
	texNames.push_back("textures/playerAttack/4.tga");
	texNames.push_back("textures/playerAttack/5.tga");
	texNames.push_back("textures/playerAttack/6.tga");
	texNames.push_back("textures/playerAttack/7.tga");
	texNames.push_back("textures/playerAttack/8.tga");
	texNames.push_back("textures/playerAttack/9.tga");
	texNames.push_back("textures/playerAttack/10.tga");
	texNames.push_back("textures/playerAttack/11.tga");
	texNames.push_back("textures/playerAttack/12.tga");
	texNames.push_back("textures/playerAttack/13.tga");
	texNames.push_back("textures/playerAttack/14.tga");
	texNames.push_back("textures/playerAttack/15.tga");

	//player dodge animation (only one frame) should be texNames[27]
	texNames.push_back("textures/playerDodge/0.tga");


    for (unsigned i = 0; i < texNames.size(); i++)
        mTextures.push_back(new Texture(texNames[i], GL_REPEAT, GL_LINEAR));

    //TODO:  Create materials
	/*
	mTextures [3] - mTextures[10] inclusive hold the player's walk cycle
	11 to 18 hold player's attack animation
	*/
    for (unsigned i = 0; i < texNames.size(); i++)
		//TODO: set textures here
        mMaterials.push_back(new Material(mTextures[i]));

    //TODO: set extra material properties go here (we don't need that shit)

	// floor
	mEntities.push_back(new Entity(cfMesh, mMaterials[0], Transform(0, -0.5f * roomHeight, -300, glm::angleAxis(glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)))));
	//player. make sure it's rendered last
	mEntities.push_back(new Entity(flMesh, mMaterials[3], Transform(-5.0f, -11.5f, 27.0f)));
	//test monster
	mEntities.push_back(new Entity(dragonMesh, mMaterials[1], Transform(3.5f, -10.0f, 27.0f)));

    // create the camera

    mCamera = new Camera(this);
    mCamera->setPosition(0, -8, 40);
    mCamera->lookAt(0, -8, 0);
    mCamera->setSpeed(6);

    // create shader program for debug geometry
    mDbgProgram = new ShaderProgram("shaders/vpc-vs.glsl",
                                    "shaders/vcolor-fs.glsl");
    // create geometry for axes
    mAxes = CreateAxes(2);

	setVSync(1);  // enable VSync

    CHECK_GL_ERRORS("initialization");
}

void BasicSceneRenderer::shutdown()
{
    for (unsigned i = 0; i < mPrograms.size(); i++)
        delete mPrograms[i];
    mPrograms.clear();

    delete mDbgProgram;
    mDbgProgram = NULL;

    delete mCamera;
    mCamera = NULL;

    for (unsigned i = 0; i < mEntities.size(); i++)
        delete mEntities[i];
    mEntities.clear();

    for (unsigned i = 0; i < mMeshes.size(); i++)
        delete mMeshes[i];
    mMeshes.clear();

    for (unsigned i = 0; i < mMaterials.size(); i++)
        delete mMaterials[i];
    mMaterials.clear();
    
    for (unsigned i = 0; i < mTextures.size(); i++)
        delete mTextures[i];
    mTextures.clear();

    delete mDbgProgram;
    mDbgProgram = NULL;
    
    delete mAxes;
    mAxes = NULL;
}

void BasicSceneRenderer::resize(int width, int height)
{
    glViewport(0, 0, width, height);

    // compute new projection matrix
    mProjMatrix = glm::perspective(glm::radians(50.f), width / (float)height, 0.1f, 1000.0f);
}

void BasicSceneRenderer::draw()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // activate current program
    ShaderProgram* prog = mPrograms[mLightingModel];
    prog->activate();

    // send projection matrix
    prog->sendUniform("u_ProjectionMatrix", mProjMatrix);

    // send the texture sampler id to shader
    prog->sendUniformInt("u_TexSampler", 0);

    // get the view matrix from the camera
    glm::mat4 viewMatrix = mCamera->getViewMatrix();

    // light setup depends on lighting model

    if (mLightingModel == PER_VERTEX_DIR_LIGHT) {

        // Basic directional light (no ambient, specular, or emissive contributions)        //

        // direction to light
        glm::vec4 lightDir = glm::normalize(glm::vec4(1, 3, 2, 0));

        // send light direction in eye space
        prog->sendUniform("u_LightDir", glm::vec3(viewMatrix * lightDir));

        // send light color/intensity
        prog->sendUniform("u_LightColor", glm::vec3(1.0f, 1.0f, 1.0f));

    } else if (mLightingModel == BLINN_PHONG_PER_FRAGMENT_DIR_LIGHT) {

        // Directional light with ambient, specular, and emissive contributions             //

        prog->sendUniform("u_AmbientLightColor", glm::vec3(0.2f, 0.2f, 0.2f));

        // direction to light
        glm::vec4 lightDir = glm::normalize(glm::vec4(1, 3, 2, 0));

        // send light direction in eye space
        prog->sendUniform("u_LightDir", glm::vec3(viewMatrix * lightDir));

        // send light color/intensity
        prog->sendUniform("u_LightColor", glm::vec3(0.8f, 0.8f, 0.8f));

    } else if (mLightingModel == BLINN_PHONG_PER_FRAGMENT_POINT_LIGHT) {

        // Point light with ambient, specular, and emissive contributions, and attenuation  //

        prog->sendUniform("u_AmbientLightColor", glm::vec3(0.1f, 0.1f, 0.1f));

        // point light position
        glm::vec3 lightPos = glm::vec3(0, 7, 0);
        glm::vec3 lightColor = glm::vec3(1.0f, 0.9f, 0.8f);

        // send light position in eye space
        prog->sendUniform("u_LightPos", glm::vec3(viewMatrix * glm::vec4(lightPos, 1)));

        // send light color/intensity
        prog->sendUniform("u_LightColor", lightColor);

        prog->sendUniform("u_AttQuat", 0.005f);
        prog->sendUniform("u_AttLin", 0.05f);
        prog->sendUniform("u_AttConst", 1.0f);

        // render the light as an emissive cube, if desired
        if (mVisualizePointLights) {
            const Mesh* lightMesh = mMeshes[0];
            lightMesh->activate();
            glBindTexture(GL_TEXTURE_2D, mTextures[7]->id());  // use black texture
            prog->sendUniform("u_MatEmissiveColor", lightColor);
            prog->sendUniform("u_ModelviewMatrix", glm::translate(viewMatrix, glm::vec3(lightPos)));
            prog->sendUniform("u_NormalMatrix", glm::mat3(1.0f));
            lightMesh->draw();
        }

    }

    // render all entities
    for (unsigned i = 0; i < mEntities.size(); i++) {

        Entity* ent = mEntities[i];

        // use the entity's material
        const Material* mat = ent->getMaterial();
        glBindTexture(GL_TEXTURE_2D, mat->tex->id());   // bind texture
        prog->sendUniform("u_Tint", mat->tint);     // send tint color

        // send the Blinn-Phong parameters, if required
        if (mLightingModel > PER_VERTEX_DIR_LIGHT) {
            prog->sendUniform("u_MatEmissiveColor", mat->emissive);
            prog->sendUniform("u_MatSpecularColor", mat->specular);
            prog->sendUniform("u_MatShininess", mat->shininess);
        }

        // compute modelview matrix
        glm::mat4 modelview = viewMatrix * ent->getWorldMatrix();

        // send the entity's modelview and normal matrix
        prog->sendUniform("u_ModelviewMatrix", modelview);
        prog->sendUniform("u_NormalMatrix", glm::transpose(glm::inverse(glm::mat3(modelview))));

        // use the entity's mesh
        const Mesh* mesh = ent->getMesh();
        mesh->activate();
        mesh->draw();
    }

    //
    // draw local axes for current entity
    //

    mDbgProgram->activate();
    mDbgProgram->sendUniform("u_ProjectionMatrix", mProjMatrix);

    Entity* activeEntity = mEntities[mActiveEntityIndex];
    mDbgProgram->sendUniform("u_ModelviewMatrix", viewMatrix * activeEntity->getWorldMatrix());
    mAxes->activate();
    //comment out the line below to get rid of axes
	//mAxes->draw();

    CHECK_GL_ERRORS("drawing");
}

bool BasicSceneRenderer::update(float dt)
{
	//TODO: Update starts here
	const Keyboard* kb = getKeyboard();

	if (kb->keyPressed(KC_ESCAPE))
		return false;

	mActiveEntityIndex = 1;
	int rbuffer = rand() % 5 + 1;
	int randomX = rand() % 5 - rbuffer;
	int randomZ = rand() % 32 + 16;

	//player is the first thing we render so it's mEntities 1
	Entity* playerVehicle = mEntities[1];
	//test obstacle
	Entity* monster = mEntities[2];
	//position of the player, can use carPos.x, etc. for coordinates
	glm::vec3 playerPosition = playerVehicle->getPosition();
	//position of test obstacle
	glm::vec3 monsterPosition = monster->getPosition();

	// set up for pickup rotation 
	float rotSpeed = 90;
	float rotAmount = rotSpeed * dt;

	//capture of car position
	p1x = playerPosition.x;
	p1y = playerPosition.y;
	p1z = playerPosition.z;

	//capture of object position
	p2x = monsterPosition.x;
	p2y = monsterPosition.y;
	p2z = monsterPosition.z;
	
	monsterTimer += 1;

	//collision check with pickups
	if (playerAttacking && !monsterInvincible) {
		calculateDistance(true);
	}
	
	if (playerAttacking == true) {
		attackBuffer += 1;
	}
	//std::cout << framebuffer << std::endl;
	//playerAttackAnimationFinished actually denotes whether or not the animation is playing
	if (playerAttacking == true && playerAttackAnimationFinished == true) {
		
		if (d > 3) {
			//movement buffer is a frame counter
			movementBuffer += 1;
			//std::cout << movementBuffer << std::endl;
			playerVehicle->translateLocal(0.1, 0, 0);
		}
		//change image
		playerVehicle->setMaterial(mMaterials[playerWalkingCounter]);
		//counter for animation cell, we have 8 frames of animation
		if (playerWalkingCounter < 10) {
			playerWalkingCounter += 1;
			//reset counter if it's 10 or higher
		}else { playerWalkingCounter = 3; }
	}

	//once certain walking position is hit, begin attack
	if (movementBuffer == 60 && playerAttacking == true && monsterInvincible == false) {
		//attack animation here
		playerAttackAnimationFinished = false;
		playerVehicle->setMaterial(mMaterials[playerAttackingCounter]);
		if (playerAttackingCounter < 26) {
			//increase counter so we can show another frame
			playerAttackingCounter += 1;
		}
		else {
			//reset counter and finish attacking so we can return to start
			playerAttackingCounter = 11;
			playerAttackAnimationFinished = true;
			playerAttacking = false;
			//TODO: code to calculate damage goes here, make sure it happens before the local translate
			if (monsterInvincible==true) { std::cout << "Player attack missed!" << std::endl; }
			else { std::cout << "Player successfully attacks!" << std::endl; }
			playerVehicle->translateLocal(-6.0f, 0, 0);
			//make sure you reset the movement buffer when you finish animation
			movementBuffer = 0;
		}
	}

	//dodging up
	if (playerDodgingUp == true && playerDodgingDown == false) {
		//update animation
		playerVehicle->setMaterial(mMaterials[playerDodgeingCounter]);

		if (dodgeBuffer < 40) {
			//dodge buffer is a frame counter
			dodgeBuffer += 1;
			playerVehicle->translateLocal(0, 0, -0.1);
		}
		else {
			playerDodgingUp = false;
			playerVehicle->translateLocal(0, 0, 4);
			dodgeBuffer = 0;

			//restore image
			playerVehicle->setMaterial(mMaterials[playerWalkingCounter]);
		}

	}

	//dodging down
	if (playerDodgingDown == true && playerDodgingUp == false) {
		//update animation
		playerVehicle->setMaterial(mMaterials[playerDodgeingCounter]);

		if (dodgeBuffer < 40) {
			//dodge buffer is a frame counter
			dodgeBuffer += 1;
			playerVehicle->translateLocal(0, 0, 0.1);
		}
		else {
			playerDodgingDown = false;
			playerVehicle->translateLocal(0, 0, -4);
			dodgeBuffer = 0;

			//restore image
			playerVehicle->setMaterial(mMaterials[playerWalkingCounter]);
		}
	}



	//player attacks
	if (kb->isKeyDown(KC_RIGHT) && playerAttacking == false && playerDodgingUp == false && playerDodgingDown == false) {
		playerAttacking = true;
	}

	//player dodges up
	if (kb->isKeyDown(KC_UP) && playerAttacking == false && playerDodgingUp == false && playerDodgingDown == false) {
		playerDodgingUp = true;
		//playerVehicle->translateLocal(0, 0, -4);
	}

	//player dodges down
	if (kb->isKeyDown(KC_DOWN) && playerAttacking == false && playerDodgingDown == false && playerDodgingUp == false) {
		playerDodgingDown = true;
		//playerVehicle->translateLocal(0, 0, 4);
	}

	//not available for now
	//player dodges left
	/*
	if (kb->isKeyDown(KC_LEFT) && playerDodgingBack == false && playerAttacking == false && playerDodgingDown == false && playerDodgingUp == false) {
		playerDodgingBack = true;
		playerVehicle->translateLocal(-3, 0, 0);
	}
	*/

	
	if (monsterInvincible == true) {
		calculateMonsterDistance();
		//std::cout << monsterD << std::endl;
		if (monsterD <= 4 && !playerDodgingUp && !playerDodgingDown) {
			std::cout << "hit" << std::endl;
		}
		else { std::cout << "dodge" << std::endl; }
		monsterTimer = 0;
		monster->translateLocal(6, 0, 0);
		monsterInvincible = false;
	}

	//monster attacks
	if (monsterTimer == baseMonsterTimers[level]) {
		monsterInvincible = true;
		//TODO: figure out why the hell the monster doesn't actually move
		monster->translateLocal(-6, 0, 0);
	}

    // update the camera
    mCamera->update(dt);

    return true;
}
