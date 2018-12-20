#pragma once

#include "Entity.h"

enum class BrickEnum { RED = 0, YELLOW, GREEN, PURPLE, BLUE, GREY };

//externed sprites
extern Sprite *blueBrickSprite;
extern Sprite *greenBrickSprite;
extern Sprite *greyBrickSprite;
extern Sprite *purpleBrickSprite;
extern Sprite *redBrickSprite;
extern Sprite *yellowBrickSprite;

class BrickEntity : public Entity
{
public:
	BrickEnum brickID;
	float x, y;
	
	BrickEntity();
	bool HandleCollision();
};

BrickEntity * MakeBrick(BrickEnum brickID, float xpos, float ypos);
void LoadMap(std::string fileName, std::vector<BrickEntity *> &brickEntityList);
