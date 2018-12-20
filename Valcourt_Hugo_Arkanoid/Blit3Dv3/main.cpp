/*
Example program that demonstrates collision handling
*/
#include "Blit3D.h"

#include <random>

#include "Physics.h"
#include "Entity.h"
#include "PaddleEntity.h"
#include "BallEntity.h"
#include "BrickEntity.h"
#include "GroundEntity.h"
#include "PowerUpEntity.h"
#include "EdgeEntity.h"

#include "MyContactListener.h" //for handling collisions

#include "CollisionMask.h"

#include "Particle.h" //particles, yay!
#include "Camera.h"

Blit3D *blit3D = NULL;

//this code sets up memory leak detection
#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <stdlib.h>
#define DEBUG_NEW new(_CLIENT_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif


//GLOBAL DATA
std::mt19937 rng;
std::uniform_real_distribution<float> plusMinus5Degrees(-5, +5);
std::uniform_real_distribution<float> plusMinus70Degrees(-70, +70);

b2Vec2 gravity; //defines our gravity vector
b2World *world; //our physics engine

				// Prepare for simulation. Typically we use a time step of 1/60 of a
				// second (60Hz) and ~10 iterations. This provides a high quality simulation
				// in most game scenarios.
int32 velocityIterations = 8;
int32 positionIterations = 3;
float timeStep = 1.f / 60.f; //one 60th of a second
float elapsedTime = 0; //used for calculating time passed

//contact listener to handle collisions between important objects
MyContactListener *contactListener;

float cursorX = 0;
PaddleEntity *paddleEntity = NULL;

enum GameState { START, PLAYING, GAMEOVER };
GameState gameState = START;
bool attachedBall = true; //is the ball ready to be launched from the paddle?
int lives = 3;

std::vector<BrickEntity *> brickEntityList; //bricks go here
std::vector<Entity *> ballEntityList; //track the balls seperately from everything else
std::vector<Entity *> entityList; //other entities in our game go here
std::vector<Entity *> deadEntityList; //dead entities

float currentBallSpeed = 60; //defaultball speed
int level = 0; //current level of play

//Sprites 
Sprite *logo = NULL;
Sprite *ballSprite = NULL;
Sprite *background = NULL;
Sprite *gameOver = NULL;
Sprite *paddleSprite = NULL;
Sprite *largePaddleSprite = NULL;


Sprite *blueBrickSprite = NULL;
Sprite *greenBrickSprite = NULL;
Sprite *greyBrickSprite = NULL;
Sprite *purpleBrickSprite = NULL;
Sprite *redBrickSprite = NULL;
Sprite *yellowBrickSprite = NULL;

Sprite *multiBallSprites[3];
Sprite *largePaddlePowerUpSprite[3];


std::vector<Sprite *> spriteList;
int editBrick = 0; //brick we are currently editing with

glm::vec2 cursor;

//particle stuff
std::vector<Particle *> particleList;

Sprite *sparkSprite1 = NULL;
Sprite *sparkSprite2 = NULL;
Sprite *sparkSprite3 = NULL;
Sprite *sparkSprite4 = NULL;
Sprite *sparkSprite5 = NULL;
Sprite *sparkSprite6 = NULL;
Sprite *sparkSprite7 = NULL;
Sprite *sparkSprite8 = NULL;
Sprite *sparkSprite9 = NULL;
Sprite *sparkSprite10 = NULL;
Sprite *sparkSprite11 = NULL;
Sprite *sparkSprite12 = NULL;

Sprite *explosionSprite1 = NULL;
Sprite *explosionSprite2 = NULL;
Sprite *explosionSprite3 = NULL;
Sprite *explosionSprite4 = NULL;
Sprite *explosionSprite5 = NULL;
Sprite *explosionSprite6 = NULL;
Sprite *explosionSprite7 = NULL;
Sprite *explosionSprite8 = NULL;
Sprite *explosionSprite9 = NULL;
Sprite *explosionSprite10 = NULL;

Sprite *boinkSprite1 = NULL;
Sprite *boinkSprite2 = NULL;
Sprite *boinkSprite3 = NULL;
Sprite *boinkSprite4 = NULL;
Sprite *boinkSprite5 = NULL;
Sprite *boinkSprite6 = NULL;
Sprite *boinkSprite7 = NULL;
Sprite *boinkSprite8 = NULL;
Sprite *boinkSprite9 = NULL;
Sprite *boinkSprite10 = NULL;

Camera2D *camera = NULL; //our camera for panning/shaking

bool bigPaddleActive = false;
float bigPaddleTimer;

bool instruction = true;
float instructionTimer = 5.f;
//font
AngelcodeFont *Font = NULL;

//______MAKE SOME BRICKS_______________________
void MakeLevel()
{
	switch (level)
	{
	case 1:
		LoadMap("level.txt", brickEntityList);
	break;

	case 2:
		LoadMap("level2.txt", brickEntityList);
		break;

	case 3:
	default:
		LoadMap("level3.txt", brickEntityList);
		break;
	}

}

//ensures that entities are only added ONCE to the deadEntityList
void AddToDeadList(Entity *e)
{
	bool unique = true;

	for (auto ent : deadEntityList)
	{
		if (ent == e)
		{
			unique = false;
			break;
		}
	}

	if (unique) deadEntityList.push_back(e);
}

void Init()
{
	//seed random generator
	std::random_device rd;
	rng.seed(rd());

	//turn off the cursor
	blit3D->ShowCursor(false);

	Font = blit3D->MakeAngelcodeFontFromBinary32("Media\\earthorbiter.bin");

	//load a sprite off of a spritesheet
	spriteList.push_back(blit3D->MakeSprite(0, 0, 64, 32, "Media\\element_blue_rectangle_glossy.png"));
	spriteList.push_back(blit3D->MakeSprite(0, 0, 64, 32, "Media\\element_green_rectangle_glossy.png"));
	spriteList.push_back(blit3D->MakeSprite(0, 0, 64, 32, "Media\\element_grey_rectangle_glossy.png"));
	spriteList.push_back(blit3D->MakeSprite(0, 0, 64, 32, "Media\\element_purple_rectangle_glossy.png"));
	spriteList.push_back(blit3D->MakeSprite(0, 0, 64, 32, "Media\\element_red_rectangle_glossy.png"));
	spriteList.push_back(blit3D->MakeSprite(0, 0, 64, 32, "Media\\element_yellow_rectangle_glossy.png"));

	blueBrickSprite = spriteList[0];
	greenBrickSprite = spriteList[1];
	greyBrickSprite = spriteList[2];
	purpleBrickSprite = spriteList[3];
	redBrickSprite = spriteList[4];
	yellowBrickSprite = spriteList[5];

	//power up animated sprite lists
	for (int i = 0; i < 3; ++i)
		multiBallSprites[i] = blit3D->MakeSprite(0, i * 32, 64, 32, "Media\\purple_powerup_list.png");

	for (int i = 0; i < 3; ++i)
		largePaddlePowerUpSprite[i] = blit3D->MakeSprite(0, i * 32, 64, 32, "Media\\blue_powerup_list.png");

	logo = blit3D->MakeSprite(0, 0, 1920, 1080, "Media\\titleScreen.jpg");

	paddleSprite = blit3D->MakeSprite(0, 0, 104, 24, "Media\\paddleGreen.png");

	background = blit3D->MakeSprite(0, 0, 1920, 1080, "Media\\background.jpg");
	
	gameOver = blit3D->MakeSprite(341, 422, 1240, 303, "Media\\GameOver.png");

	largePaddleSprite = blit3D->MakeSprite(0, 0, 350, 24, "Media\\paddleGreenLarge.png");

	ballSprite = blit3D->MakeSprite(0, 0, 22, 22, "Media\\ballRed.png");

	//particle sprites
	sparkSprite1 = blit3D->MakeSprite(18, 45, 18, 14, "Media\\sparks_effect.png");
	sparkSprite2 = blit3D->MakeSprite(16, 105, 24, 18, "Media\\sparks_effect.png");
	sparkSprite3 = blit3D->MakeSprite(14, 167, 26, 20, "Media\\sparks_effect.png");
	sparkSprite4 = blit3D->MakeSprite(12, 227, 31, 26, "Media\\sparks_effect.png");
	sparkSprite5 = blit3D->MakeSprite(10, 288, 35, 28, "Media\\sparks_effect.png");
	sparkSprite6 = blit3D->MakeSprite(7, 346, 41, 34, "Media\\sparks_effect.png");
	sparkSprite7 = blit3D->MakeSprite(5, 406, 47, 37, "Media\\sparks_effect.png");
	sparkSprite8 = blit3D->MakeSprite(5, 467, 52, 37, "Media\\sparks_effect.png");
	sparkSprite9 = blit3D->MakeSprite(22, 530, 37, 33, "Media\\sparks_effect.png");
	sparkSprite10 = blit3D->MakeSprite(22, 594, 37, 30, "Media\\sparks_effect.png");
	sparkSprite11 = blit3D->MakeSprite(22, 658, 15, 24, "Media\\sparks_effect.png");
	sparkSprite12 = blit3D->MakeSprite(22, 723, 15, 23, "Media\\sparks_effect.png");


	explosionSprite1 = blit3D->MakeSprite(12, 22, 8, 9, "Media\\magic_effect_2.png");
	explosionSprite2 = blit3D->MakeSprite(12, 22, 8, 9, "Media\\magic_effect_2.png");
	explosionSprite3 = blit3D->MakeSprite(8, 43, 14, 19, "Media\\magic_effect_2.png");
	explosionSprite4 = blit3D->MakeSprite(9, 97, 17, 28, "Media\\magic_effect_2.png");
	explosionSprite5 = blit3D->MakeSprite(5, 129, 20, 20, "Media\\magic_effect_2.png");
	explosionSprite6 = blit3D->MakeSprite(6, 160, 19, 23, "Media\\magic_effect_2.png");
	explosionSprite7 = blit3D->MakeSprite(4, 192, 25, 27, "Media\\magic_effect_2.png");
	explosionSprite8 = blit3D->MakeSprite(2, 224, 27, 24, "Media\\magic_effect_2.png");
	explosionSprite9 = blit3D->MakeSprite(4, 256, 23, 24, "Media\\magic_effect_2.png");
	explosionSprite10 = blit3D->MakeSprite(3, 289, 25, 22, "Media\\magic_effect_2.png");

	boinkSprite1 = blit3D->MakeSprite(11, 11, 11, 12, "Media\\green_effect.png");
	boinkSprite2 = blit3D->MakeSprite(10, 43, 14, 14, "Media\\green_effect.png");
	boinkSprite3 = blit3D->MakeSprite(8, 73, 16, 16, "Media\\green_effect.png");
	boinkSprite4 = blit3D->MakeSprite(7, 103, 20, 20, "Media\\green_effect.png");
	boinkSprite5 = blit3D->MakeSprite(5, 133, 22, 22, "Media\\green_effect.png");
	boinkSprite6 = blit3D->MakeSprite(5, 165, 23, 23, "Media\\green_effect.png");
	boinkSprite7 = blit3D->MakeSprite(4, 195, 26, 26, "Media\\green_effect.png");
	boinkSprite8 = blit3D->MakeSprite(2, 226, 29, 29, "Media\\green_effect.png");
	boinkSprite9 = blit3D->MakeSprite(2, 258, 29, 29, "Media\\green_effect.png");
	boinkSprite10 = blit3D->MakeSprite(2, 290, 29, 29, "Media\\green_effect.png");

	//make a camera
	camera = new Camera2D();

	//set it's valid pan area
	camera->minX = -blit3D->screenWidth / 2;
	camera->minY = -blit3D->screenHeight / 2;
	camera->maxX = 2 * blit3D->screenWidth / 2;
	camera->maxY = 2 * blit3D->screenHeight / 2;

	//pan to the center of the screen
	camera->PanTo(blit3D->screenWidth / 2, blit3D->screenHeight / 2);

	//from here on, we are setting up the Box2D physics world model

	// Define the gravity vector.
	gravity.x = 0.f;
	gravity.y = 0.f;

	// Construct a world object, which will hold and simulate the rigid bodies.
	world = new b2World(gravity);
	//world->SetGravity(gravity); <-can call this to change gravity at any time
	world->SetAllowSleeping(true); //set true to allow the physics engine to 'sleep" objects that stop moving

								   //_________GROUND OBJECT_____________
								   //make an entity for the ground
	GroundEntity *groundEntity = new GroundEntity();
	//A bodyDef for the ground
	b2BodyDef groundBodyDef;
	// Define the ground body.
	groundBodyDef.position.Set(0, 0);

	// Call the body factory which allocates memory for the ground body
	// from a pool and creates the ground box shape (also from a pool).
	// The body is also added to the world.
	groundEntity->body = world->CreateBody(&groundBodyDef);

	//an EdgeShape object, for the ground
	b2EdgeShape groundBox;

	// Define the ground as 1 edge shape at the bottom of the screen.
	b2FixtureDef boxShapeDef;

	boxShapeDef.shape = &groundBox;

	//collison masking
	boxShapeDef.filter.categoryBits = CMASK_GROUND;  //this is the ground
	boxShapeDef.filter.maskBits = CMASK_BALL | CMASK_POWERUP;		//it collides wth balls and powerups

																	//bottom
	groundBox.Set(b2Vec2(0, 0), b2Vec2(blit3D->screenWidth / PTM_RATIO, 0));
	//Create the fixture
	groundEntity->body->CreateFixture(&boxShapeDef);
	//add the userdata
	groundEntity->body->SetUserData(groundEntity);
	//add to the entity list
	entityList.push_back(groundEntity);

	//now make the other 3 edges of the screen on a seperate entity/body
	EdgeEntity * edgeEntity = new EdgeEntity();
	edgeEntity->body = world->CreateBody(&groundBodyDef);

	boxShapeDef.filter.categoryBits = CMASK_EDGES;  //this is the edges/top
	boxShapeDef.filter.maskBits = CMASK_BALL;		//it collides wth balls

													//left
	groundBox.Set(b2Vec2(0, blit3D->screenHeight / PTM_RATIO), b2Vec2(0, 0));
	edgeEntity->body->CreateFixture(&boxShapeDef);

	//top
	groundBox.Set(b2Vec2(0, blit3D->screenHeight / PTM_RATIO),
		b2Vec2(blit3D->screenWidth / PTM_RATIO, blit3D->screenHeight / PTM_RATIO));
	edgeEntity->body->CreateFixture(&boxShapeDef);

	//right
	groundBox.Set(b2Vec2(blit3D->screenWidth / PTM_RATIO,
		0), b2Vec2(blit3D->screenWidth / PTM_RATIO, blit3D->screenHeight / PTM_RATIO));
	edgeEntity->body->CreateFixture(&boxShapeDef);

	edgeEntity->body->SetUserData(edgeEntity);
	entityList.push_back(edgeEntity);
	//______MAKE A BALL_______________________
	BallEntity *ball = MakeBall(ballSprite);

	//move the ball to the center bottomof the window
	b2Vec2 pos(blit3D->screenWidth / 2, 35);
	pos = Pixels2Physics(pos);//convert coordinates from pixels to physics world
	ball->body->SetTransform(pos, 0.f);

	//add the ball to our list of (ball) entities
	ballEntityList.push_back(ball);

	// Create contact listener and use it to collect info about collisions
	contactListener = new MyContactListener();
	world->SetContactListener(contactListener);

	//paddle
	cursorX = blit3D->screenWidth / 2;
	paddleEntity = MakePaddle(blit3D->screenWidth / 2, 30, paddleSprite, 104.f, 24.f);
	entityList.push_back(paddleEntity);
}

void DeInit(void)
{
	//delete all particles
	for (auto p : particleList) delete p;
	particleList.clear();

	//delete all the entities
	for (auto e : entityList) delete e;
	entityList.clear();

	for (auto b : ballEntityList) delete b;
	ballEntityList.clear();

	for (auto b : brickEntityList) delete b;
	brickEntityList.clear();

	for (auto b : brickEntityList) delete b;
	brickEntityList.clear();

	//delete the contact listener
	delete contactListener;

	//Free all physics game data we allocated
	delete world;

	if (camera != NULL) delete camera;

	//any sprites/fonts still allocated are freed automatcally by the Blit3D object when we destroy it
}

void Update(double seconds)
{
	switch (gameState)
	{
	case PLAYING:
	{
		//we must update the camera every update;
		//this applies the pans and shakes
		camera->Update((float)seconds);

		if (instruction == true)
		{
			instructionTimer -= seconds;
			if(instructionTimer <= 0)
				instruction = false;
		}
		//stop it from lagging hard if more than a small amount of time has passed
		if (seconds > 1.0 / 30) elapsedTime += 1.f / 30;
		else elapsedTime += (float)seconds;

		//is the big paddle timer activated
		if (bigPaddleActive == true)
			bigPaddleTimer -= seconds;

		//is the timer for the big paddle over?
		if (bigPaddleTimer <= 0 && bigPaddleActive == true)
		{
			//delete old paddel and create a new one
			AddToDeadList(paddleEntity);
			bigPaddleActive = false;
			b2Vec2 paddlePosition = Physics2Pixels(paddleEntity->body->GetPosition());
			paddleEntity = MakePaddle(paddlePosition.x, paddlePosition.y, paddleSprite, 104.f, 24.f);
			entityList.push_back(paddleEntity);

		}
		//did we finish the level?
		if (brickEntityList.size() == 0)
		{
			level++;
			MakeLevel();
		}

		//move paddle to where the cursor is
		b2Vec2 paddlePos;
		paddlePos.y = 30;
		paddlePos.x = cursorX;
		paddlePos = Pixels2Physics(paddlePos);
		paddleEntity->body->SetTransform(paddlePos, 0);

		//make sure the balls keep moving at the right speed, and delete any way out of bounds
		for (int i = ballEntityList.size() - 1; i >= 0; --i)
		{
			b2Vec2 dir = ballEntityList[i]->body->GetLinearVelocity();
			dir.Normalize();
			dir *= currentBallSpeed; //scale up the velocity tp correct ball speed
			ballEntityList[i]->body->SetLinearVelocity(dir); //apply velocity to kinematic object

			b2Vec2 pos = ballEntityList[i]->body->GetPosition();
			if (pos.x < 0 || pos.y < 0
				|| pos.x * PTM_RATIO > blit3D->screenWidth
				|| pos.y * PTM_RATIO > blit3D->screenHeight)
			{
				//kill this ball, it is out of bounds
				world->DestroyBody(ballEntityList[i]->body);
				delete ballEntityList[i];
				ballEntityList.erase(ballEntityList.begin() + i);
			}
		}

		//sanity check, should always have at least one ball!
		if (ballEntityList.size() == 0)
		{
			//welp, let's make a ball
			BallEntity *ball = MakeBall(ballSprite);
			ballEntityList.push_back(ball);
			attachedBall = true;
		}

		//if ball is attached to paddle, move ball to where paddle is
		if (attachedBall)
		{
			assert(ballEntityList.size() > 0); //make sure there is at least one ball
			b2Vec2 ballPos = paddleEntity->body->GetPosition();
			ballPos.y = 52 / PTM_RATIO;
			ballEntityList[0]->body->SetTransform(ballPos, 0);
			ballEntityList[0]->body->SetLinearVelocity(b2Vec2(0.f, 0.f));
		}

		//don't apply physics unless at least a timestep worth of time has passed
		while (elapsedTime >= timeStep)
		{
			//update the physics world
			world->Step(timeStep, velocityIterations, positionIterations);

			// Clear applied body forces. 
			world->ClearForces();

			elapsedTime -= timeStep;

			//update game logic/animation
			for (auto e : entityList) e->Update(timeStep);
			for (auto b : ballEntityList) b->Update(timeStep);
			for (auto b : brickEntityList) b->Update(timeStep);

			//update the particle list and remove dead particles
			for (int i = particleList.size() - 1; i >= 0; --i)
			{
				if (particleList[i]->Update(timeStep))
				{
					//time to die!
					delete particleList[i];
					particleList.erase(particleList.begin() + i);
				}
			}

			//loop over contacts
			for (int pos = 0; pos < contactListener->contacts.size(); ++pos)
			{
				MyContact contact = contactListener->contacts[pos];

				//fetch the entities from the body userdata
				Entity *A = (Entity *)contact.fixtureA->GetBody()->GetUserData();
				Entity *B = (Entity *)contact.fixtureB->GetBody()->GetUserData();

				if (A != NULL && B != NULL) //if there is an entity for these objects...
				{

					if (A->typeID == ENTITYBALL)
					{
						//swap A and B
						Entity *C = A;
						A = B;
						B = C;
					}
					
					if (B->typeID == ENTITYBALL && A->typeID == ENTITYEDGE)
					{
						//add a particle effect
						Particle *p = new Particle();
						p->spriteList.push_back(boinkSprite1);
						p->spriteList.push_back(boinkSprite2);
						p->spriteList.push_back(boinkSprite2);
						p->spriteList.push_back(boinkSprite2);
						p->spriteList.push_back(boinkSprite3);
						p->spriteList.push_back(boinkSprite3);
						p->spriteList.push_back(boinkSprite3);
						p->spriteList.push_back(boinkSprite4);
						p->spriteList.push_back(boinkSprite4);
						p->spriteList.push_back(boinkSprite5);
						p->spriteList.push_back(boinkSprite5);
						p->spriteList.push_back(boinkSprite6);
						p->spriteList.push_back(boinkSprite6);
						p->spriteList.push_back(boinkSprite7);
						p->spriteList.push_back(boinkSprite7);
						p->spriteList.push_back(boinkSprite8);
						p->spriteList.push_back(boinkSprite8);
						p->spriteList.push_back(boinkSprite9);
						p->spriteList.push_back(boinkSprite9);
						p->spriteList.push_back(boinkSprite10);
						p->spriteList.push_back(boinkSprite10);

						p->rotationSpeed = (float)(rand() % 181) - 90;
						p->angle = rand() % 360;

						p->direction = b2Vec2(0.f, 0.f);
						p->startingSpeed = 0;
						p->targetSpeed = 0;
						p->totalTimeToLive = 0.5f;
						//get coords of contact
						p->coords = Physics2Pixels(contact.contactPoint);

						particleList.push_back(p);
					}

					if (B->typeID == ENTITYBALL && A->typeID == ENTITYBRICK)
					{
						camera->Shake(10.f); //shake the camera, radius 100 pixels at start

						BrickEntity *be = (BrickEntity *)A;
						if (be->HandleCollision())
						{
							//we need to remove this brick from the world, 
							//but can't do that until all collisions have been processed
							AddToDeadList(be);
						}

						//did we hit a brick that should spawn a power up?
						if (be->brickID == BrickEnum::PURPLE)
						{
							b2Vec2 position = be->body->GetPosition();
							position = Physics2Pixels(position);
							PowerUpEntity *p = MakePowerUp(PowerUpType::MULTIBALL, position.x, position.y);
							entityList.push_back(p);
						}

						//did we hit a brick that should spawn a power up?
						if (be->brickID == BrickEnum::BLUE)
						{
							b2Vec2 position = be->body->GetPosition();
							position = Physics2Pixels(position);
							PowerUpEntity *p = MakePowerUp(PowerUpType::LARGEPADDLE, position.x, position.y);
							entityList.push_back(p);
						}

						//add a particle effect
						Particle *p = new Particle();
						p->spriteList.push_back(sparkSprite1);
						p->spriteList.push_back(sparkSprite2);
						p->spriteList.push_back(sparkSprite3);
						p->spriteList.push_back(sparkSprite4);
						p->spriteList.push_back(sparkSprite5);
						p->spriteList.push_back(sparkSprite6);
						p->spriteList.push_back(sparkSprite7);
						p->spriteList.push_back(sparkSprite8);
						p->spriteList.push_back(sparkSprite9);
						p->spriteList.push_back(sparkSprite10);
						p->spriteList.push_back(sparkSprite11);
						p->spriteList.push_back(sparkSprite12);
						p->rotationSpeed = (float)(rand() % 181) - 90;
						p->angle = rand() % 360;
						//let's make it 'follow' after the ball
						b2Vec2 dir = B->body->GetLinearVelocity();
						float speed = dir.Length() * PTM_RATIO;
						dir.Normalize();
						p->direction = dir;
						p->startingSpeed = speed / 5;
						p->targetSpeed = speed / 10;
						p->totalTimeToLive = 0.2f;
						//get coords of contact
						p->coords = Physics2Pixels(contact.contactPoint);

						particleList.push_back(p);
					}

					if (B->typeID == ENTITYBALL && A->typeID == ENTITYPADDLE)
					{
						PaddleEntity *pe = (PaddleEntity *)A;
						//bend the ball's flight
						pe->HandleCollision(B->body);
					}

					if (B->typeID == ENTITYBALL && A->typeID == ENTITYGROUND)
					{
						if (ballEntityList.size() > 1)
						{
							//remove this ball, but we have others
							AddToDeadList(B);
						}
						else
						{
							//lose a life
							lives--;
							attachedBall = true; //attach the ball for launching
							ballEntityList[0]->body->SetLinearVelocity(b2Vec2(0.f, 0.f));
							if (lives < 1)
							{
								//gameover
								gameState = GAMEOVER;
							}
						}

						//add a particle effect
						Particle *p = new Particle();
						p->spriteList.push_back(explosionSprite4);
						p->spriteList.push_back(explosionSprite1);
						p->spriteList.push_back(explosionSprite1);
						p->spriteList.push_back(explosionSprite2);
						p->spriteList.push_back(explosionSprite3);
						p->spriteList.push_back(explosionSprite4);
						p->spriteList.push_back(explosionSprite5);
						p->spriteList.push_back(explosionSprite6);
						p->spriteList.push_back(explosionSprite7);
						p->spriteList.push_back(explosionSprite8);
						p->spriteList.push_back(explosionSprite9);
						p->spriteList.push_back(explosionSprite10);

						p->spriteList.push_back(sparkSprite3);
						p->spriteList.push_back(sparkSprite2);
						p->spriteList.push_back(sparkSprite1);
						p->spriteList.push_back(sparkSprite1);
						p->spriteList.push_back(sparkSprite2);
						p->spriteList.push_back(sparkSprite3);

						p->rotationSpeed = (float)(rand() % 181) - 90;
						p->angle = rand() % 360;
						p->direction = b2Vec2(0.f, 0.f);;
						p->startingSpeed = 0;
						p->targetSpeed = 0;
						p->totalTimeToLive = 0.5f;
						//get coords of ball
						p->coords = Physics2Pixels(B->body->GetPosition());

						particleList.push_back(p);
					}

					//check and see if paddle or ground hit a powerup
					if (A->typeID == ENTITYPOWERUP)
					{
						//swap A and B
						Entity *C = A;
						A = B;
						B = C;
					}

					if (B->typeID == ENTITYPOWERUP)
					{
						AddToDeadList(B);

					if (A->typeID == ENTITYPADDLE)
					{
						

							PowerUpEntity *p = (PowerUpEntity *)B;
							switch (p->powerUpType)
							{
							case PowerUpType::MULTIBALL:
								for (int j = ballEntityList.size() - 1; j >= 0; --j)
								{
									//make 2 new balls
									for (int i = 0; i < 2; ++i)
									{
										//add extra balls if we haven't reached pur limit
										if (ballEntityList.size() < 30)
										{
											BallEntity *b = MakeBall(ballSprite);
											b2Vec2 ballPos = ballEntityList[j]->body->GetPosition();
											ballPos.x += (i * 18 - 9) / PTM_RATIO;
											b->body->SetTransform(ballPos, 0);
											//kick the ball in a random direction upwards			
											b2Vec2 dir = deg2vec(90 + plusMinus70Degrees(rng)); //between 20-160 degrees

											//make the ball move
											dir *= currentBallSpeed; //scale up the velocity
											b->body->SetLinearVelocity(dir); //apply velocity to kinematic object	
																			 //add to the ballist
											ballEntityList.push_back(b);
										}
									}
								}
								break;

							case PowerUpType::LARGEPADDLE:
							{
								AddToDeadList(paddleEntity);
								b2Vec2 paddlePosition = Physics2Pixels(paddleEntity->body->GetPosition());
								paddleEntity = MakePaddle(paddlePosition.x, paddlePosition.y, largePaddleSprite, 350.f, 24.f);
								entityList.push_back(paddleEntity);

								//delet old paddle
								

								bigPaddleTimer = 10.f;
								bigPaddleActive = true;
								
							}
								
								break;

							default:
								assert("Unknown PowerUp type" == 0);
								break;
							}//end switch on powerup type
						}
					}
				}//end of checking if they are both NULL userdata
			}//end of collison handling
			 //clean up dead entities
			for (auto e : deadEntityList)
			{
				//remove body from the physics world and free the body data
				world->DestroyBody(e->body);
				//remove the entity from the appropriate entityList
				if (e->typeID == ENTITYBALL)
				{
					for (int i = 0; i < ballEntityList.size(); ++i)
					{
						if (e == ballEntityList[i])
						{
							delete ballEntityList[i];
							ballEntityList.erase(ballEntityList.begin() + i);
							break;
						}
					}
				}
				else if (e->typeID == ENTITYBRICK)
				{
					for (int i = 0; i < brickEntityList.size(); ++i)
					{
						if (e == brickEntityList[i])
						{
							delete brickEntityList[i];
							brickEntityList.erase(brickEntityList.begin() + i);
							break;
						}
					}
				}
				else
				{
					for (int i = 0; i < entityList.size(); ++i)
					{
						if (e == entityList[i])
						{
							delete entityList[i];
							entityList.erase(entityList.begin() + i);
							break;
						}
					}
				}
			}

			deadEntityList.clear();
		}
	}
	break; //end case PLAYING
	case START:

	default:
		//do nada here
		break;
	}//end switch(gameState)
}

void Draw(void)
{
	glClearColor(0.8f, 0.6f, 0.7f, 0.0f);	//clear colour: r,g,b,a 	
											// wipe the drawing surface clear
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	std::string lifeString = "Lives: " + std::to_string(lives);
	std::string instructionGameOver = "Click to exit";
	std::string instructionSuicide = "'K' to suicidee if needed";

	//draw stuff here
	camera->Draw();

	switch (gameState)
	{
	case START:
		logo->Blit(blit3D->screenWidth / 2, blit3D->screenHeight / 2);
		break;

	case PLAYING:
		background->Blit(blit3D->screenWidth / 2, blit3D->screenHeight / 2, 1.1f, 1.1f);

		if (instructionTimer > 0)
			Font->BlitText(775, 150, instructionSuicide);

		//loop over all entities and draw them
		for (auto e : entityList) e->Draw();
		for (auto b : brickEntityList) b->Draw();
		for (auto b : ballEntityList) b->Draw();
		for (auto p : particleList) p->Draw();
		
		Font->BlitText(10, 35, lifeString);

		break;

	case GAMEOVER:
		background->Blit(blit3D->screenWidth / 2, blit3D->screenHeight / 2);

		//loop over all entities and draw them
		for (auto e : entityList) e->Draw();
		for (auto b : brickEntityList) b->Draw();
		for (auto b : ballEntityList) b->Draw();
		for (auto p : particleList) p->Draw();

		Font->BlitText(10, 35, lifeString);
		
		gameOver->Blit(blit3D->screenWidth / 2, blit3D->screenHeight / 2);
		Font->BlitText(850, 100, instructionGameOver);

	}
}

//the key codes/actions/mods for DoInput are from GLFW: check its documentation for their values
void DoInput(int key, int scancode, int action, int mods)
{
	if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		blit3D->Quit(); //start the shutdown sequence
	switch (gameState)
	{
	case PLAYING:
	//suicide button is K
		if (key == GLFW_KEY_K && action == GLFW_PRESS)
		{
			if (gameState == PLAYING)
			{
				//kill off the current first ball
				lives--;
				attachedBall = true; //attach the ball for launching

									 //safety check
				//assert(ballEntityList.size() > 0); //make sure there is at least one ball

				ballEntityList[0]->body->SetLinearVelocity(b2Vec2(0.f, 0.f));
				if (lives < 1)
				{
					//gameover
					gameState = GAMEOVER;
				}
			}
		}
		break;

	}
	
}

void DoCursor(double x, double y)
{
	//scale display size
	cursorX = static_cast<float>(x) * (blit3D->screenWidth / blit3D->trueScreenWidth);

	cursor.x = x * (blit3D->screenWidth / blit3D->trueScreenWidth);
	cursor.y = (blit3D->trueScreenHeight - y) * (blit3D->screenHeight / blit3D->trueScreenHeight);
}

void DoMouseButton(int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		switch (gameState)
		{
		case START:
			gameState = PLAYING;
			attachedBall = true;
			lives = 3;
			level = 1;
			MakeLevel();
			break;

		case PLAYING:
			if (attachedBall)
			{
				attachedBall = false;
				//launch the ball

				//safety check
				assert(ballEntityList.size() >= 1); //make sure there is at least one ball	

				Entity *b = ballEntityList[0];
				//kick the ball in a random direction upwards			
				b2Vec2 dir = deg2vec(90 + plusMinus5Degrees(rng)); //between 85-95 degrees
														//make the ball move
				dir *= currentBallSpeed; //scale up the velocity
				b->body->SetLinearVelocity(dir); //apply velocity to kinematic object				
			}
			break;

		case GAMEOVER:
			if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
				blit3D->Quit();
			break;
		}
	}
}



int main(int argc, char *argv[])
{
	//memory leak detection
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	blit3D = new Blit3D(Blit3DWindowModel::BORDERLESSFULLSCREEN, 1920, 1080);

	//set our callback funcs
	blit3D->SetInit(Init);
	blit3D->SetDeInit(DeInit);
	blit3D->SetUpdate(Update);
	blit3D->SetDraw(Draw);
	blit3D->SetDoInput(DoInput);
	blit3D->SetDoCursor(DoCursor);
	blit3D->SetDoMouseButton(DoMouseButton);

	//Run() blocks until the window is closed
	blit3D->Run(Blit3DThreadModel::SINGLETHREADED);
	if (blit3D) delete blit3D;
}