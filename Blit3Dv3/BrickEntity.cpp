#include "BrickEntity.h"
#include "CollisionMask.h"

extern b2World *world;

BrickEntity::BrickEntity()
{
	typeID = ENTITYBRICK;
	brickID = BrickEnum::BLUE;
}

//Decide whether to change colour or die off:
//return true if this object should be removed
bool BrickEntity::HandleCollision()
{
	bool retval = false;

	switch(brickID)
	{
	case BrickEnum::RED:
		brickID = BrickEnum::YELLOW;
		sprite = yellowBrickSprite;
		break;

	case BrickEnum::YELLOW:
		brickID = BrickEnum::GREEN;
		sprite = greenBrickSprite;
		break;

	case BrickEnum::GREEN:
		retval = true;
		break;
		
	case BrickEnum::PURPLE:
		retval = true;
		break;

	case BrickEnum::BLUE:
		retval = true;
		break;

	case BrickEnum::GREY:
		
		break;

	default:
		retval = true;
		break;
	}

	return retval;
}

BrickEntity * MakeBrick(BrickEnum brickID, float xpos, float ypos)
{
	BrickEntity *brickEntity = new BrickEntity();
	brickEntity->brickID = brickID;

	//set the sprite to draw with
	switch(brickID)
	{
	case BrickEnum::BLUE:
		brickEntity->sprite = blueBrickSprite;
		break;

	case BrickEnum::GREEN:
		brickEntity->sprite = greenBrickSprite;
		break;

	case BrickEnum::GREY:
		brickEntity->sprite = greyBrickSprite;
		break;

	case BrickEnum::PURPLE:
		brickEntity->sprite = purpleBrickSprite;
		break;

	case BrickEnum::RED:
		brickEntity->sprite = redBrickSprite;
		break;

	case BrickEnum::YELLOW:
		brickEntity->sprite = yellowBrickSprite;
		break;

	default: 
		brickEntity->sprite = blueBrickSprite;
		break;
	}

	//make the physics body
	b2BodyDef brickBodyDef;

	//set the position of the center of the body, 
	//converting from pxel coords to physics measurements
	brickBodyDef.position.Set(xpos / PTM_RATIO, ypos / PTM_RATIO);
	brickBodyDef.type = b2_kinematicBody; //make it a kinematic body i.e. one moved by us

	brickEntity->body = world->CreateBody(&brickBodyDef); //create the body and add it to the world

	// Define a box shape for our dynamic body.
	b2PolygonShape boxShape;
	//SetAsBox() takes as arguments the half-width and half-height of the box
	boxShape.SetAsBox(64.0f / (2.f*PTM_RATIO), 32.0f / (2.f*PTM_RATIO));

	b2FixtureDef brickFixtureDef;
	brickFixtureDef.shape = &boxShape;
	brickFixtureDef.density = 1.0f; //won't matter, as we made this kinematic
	brickFixtureDef.restitution = 0;
	brickFixtureDef.friction = 0.1f;

	//collison masking
	brickFixtureDef.filter.categoryBits = CMASK_BRICK;  //this is a brick
	brickFixtureDef.filter.maskBits = CMASK_BALL;		//it collides with balls

	brickEntity->body->CreateFixture(&brickFixtureDef);

	//make the userdata point back to this entity
	brickEntity->body->SetUserData(brickEntity);

	return brickEntity;
}

void LoadMap(std::string fileName, std::vector<BrickEntity *> &brickEntityList)
{
	//clear the current brickList
	for (auto B : brickEntityList) delete B;
	brickEntityList.clear();

	//open file
	std::ifstream myfile;
	myfile.open(fileName);

	if (myfile.is_open())
	{
		//read in # of bricks
		int brickNum = 0;
		myfile >> brickNum;

		//read in each brick
		for (; brickNum > 0; --brickNum)
		{
			

			//make a brick
		
			int t = 0;
			myfile >> t;
			float x, y;
		
			myfile >> x;
			myfile >> y;
			BrickEntity *B = MakeBrick((BrickEnum)t, x, y);
			brickEntityList.push_back(B);
		}

		myfile.close();
	}
}