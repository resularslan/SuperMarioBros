#include <SDL3/SDL.h>
#include <SDL3/SDL_time.h>
#include <SDL3_image/SDL_image.h>
#include <iostream>
#include <fstream>

using namespace std;

#define CELL_SIZE 16
#define WIDTH_CELL 424
#define HEIGHT_CELL 30
#define WINDOW_WIDTH (32 * CELL_SIZE)
#define WINDOW_HEIGHT (HEIGHT_CELL * CELL_SIZE)
#define CONST (CELL_SIZE / 8)

struct vector2
{
    float x;
    float y;
    vector2 normalized()
    {
        return { (float) (x > 0) - (x < 0), (float) (y > 0) - (y < 0) };
    }
};

struct Enemy
{
	SDL_FRect rect;
	vector2 velocity = { 0.75f, 0};
	vector2 oldpos;
	vector2 direction = { -1, 0 };
	bool isDiedByMario = false;
	bool isDiedByStar = false;
	bool active = true;
	int frameIndex = 0;
	char random = rand() % 2;
	Uint32 lastFrameTime = 0;
	void restart()
	{
		velocity = { 0.75f, 0 };
		isDiedByMario = false;
		isDiedByStar = false;
		active = true;
		direction = { -1, 0 };
	}
};

struct Mushroom
{
	SDL_FRect rect;
	vector2 velocity = { 0.75f, 0 };
	vector2 oldpos;
	vector2 direction = { 1, 0 };
	int animTimer = 0;
	bool active = false;
	void restart()
	{
		velocity = { 0.75f, 0 };
		animTimer = 0;
		active = false;
		direction = { 1, 0 };
	}
};

struct Star
{
	SDL_FRect rect;
	SDL_FRect srcRect;
	vector2 velocity = { 0.75f, 0 };
	vector2 oldpos;
	vector2 direction = { 1, 0 };
	int animTimer = 0;
	int frameIndex = 0;
	Uint32 lastFrameTime = 0;
	bool active = false;
	void restart()
	{
		velocity = { 0.75f, 0 };
		animTimer = 0;
		active = false;
		direction = { 1, 0 };
	}
};

struct Player
{
	SDL_FRect rect;
	SDL_FlipMode flip;
	vector2 velocity;
	vector2 oldpos;
	float oldvelocity_x;
	int frameIndex = 0;
	int animFrameDelay;
	Uint32 lastFrameTime = 0;
	bool grounded;
	bool falling;
	bool jumping;
	bool sliding;
	bool facingRight = true;
	bool isDied = false;
	bool canDie = true;
	bool starMode = false;
	float jumpTime = 0;
	bool isBig = false;
	int invisibleAnimation = 0;
	int growAnimation = -1;
	bool flagAnimation = false;
	float originalY;
	int life = 3;
	void restart()
	{
		facingRight = true;
		flagAnimation = false;
		velocity = { 0 , 0 };
		canDie = true;
		jumpTime = 0;
		isBig = false;
		invisibleAnimation = 0;
		originalY;
		isDied = false;
	}
};

struct Tile
{
	int tilemap;
	SDL_FRect destRect;
	float originalY;
	int bounceTimer;
	bool isCollided = false;
	bool anim = false;
	bool active = true;
	void restart()
	{
		isCollided = false;
		anim = false;
		active = true;
	}
};

struct Coin
{
	SDL_FRect destRect;
	SDL_FRect srcRect;
	int number = 8;
	int animTime;
	int frameIndex = 0;
	Uint32 lastFrameTime = 0;
	bool active = false;
	void restart()
	{
		number = 8;
		active = false;
	}
};

bool running = true;

SDL_Window* window;
SDL_Renderer* renderer;
SDL_Texture* tileset_texture;
SDL_Texture* marioSmallRunFrames[3];
SDL_Texture* marioSmallIdleFrame;
SDL_Texture* marioSmallJumpFrame;
SDL_Texture* marioSmallSlideFrame;
SDL_Texture* marioSmallDeathFrame;
SDL_Texture* marioSmallFlag;
SDL_Texture* marioBigRunFrames[3];
SDL_Texture* marioBigIdleFrame;
SDL_Texture* marioBigJumpFrame;
SDL_Texture* marioBigSlideFrame;
SDL_Texture* marioBigFlag;
SDL_Texture* marioMiddleFrame;
SDL_Texture* goombaWalkFrames[2];
SDL_Texture* goombaMarioKilledFrame;
SDL_Texture* goombaDeathFrame;
SDL_Texture* koopaWalkFrames[2];
SDL_Texture* koopaMarioKilledFrame;
SDL_Texture* koopaDeathFrame;
SDL_Texture* magicMushroomTexture;
SDL_Texture* upMushroomTexture;
SDL_Texture* miscTexture;
SDL_FRect camera;
Player player;
Enemy goombas[16];
Enemy koopas[1];
Coin coin;
Mushroom magicMushroom;
Mushroom upMushroom;
Star star;
Tile tile[HEIGHT_CELL][WIDTH_CELL];
Tile collisiontiles[HEIGHT_CELL / 2][WIDTH_CELL / 2];
SDL_FPoint entityCenter = { CELL_SIZE, CELL_SIZE };
SDL_FPoint center = { CELL_SIZE / 2, CELL_SIZE / 2 };
//Uint64 player.lastFrameTime = SDL_GetPerformanceCounter();
//float deltaTime;

bool keys[SDL_SCANCODE_COUNT] = { false };
bool tmpKey;

float acceleration = 0.1f * CONST;
float maxSpeed;
const float friction = 0.04f * CONST;
const float gravity = 0.2f * CONST;

bool stopMovement = false;

int enemyAnimFrameDelay = 100;
Uint32 currentTime = (Uint32)SDL_GetTicks();

// Brick pieces
int angle;
SDL_FRect srect1 = { 34, 272, 8, 8 };
SDL_FRect srect2 = { 43, 272, 8, 8 };
SDL_FRect srect3 = { 34, 281, 8, 8 };
SDL_FRect srect4 = { 43, 281, 8, 8 };

const int FPS = 60;
const int frameDelay = 1000 / FPS;
Uint64 frameStart;
int frameTime;

void initialize();
void loadTilemap();
void loadCollision(bool restart);
void updateCamera();
void renderTexture(SDL_Texture* texture, SDL_FRect* srcRect, SDL_FRect* rect, double angle, SDL_FPoint* center, SDL_FlipMode flip);
void renderTilemap();
void getOldVectors();
void renderEnemies();
void handlePlayerEnemyCollision(Player* player, Enemy* enemy, vector2* oldplayerpos, vector2* oldenemypos);
void handleEnemyEnemyCollision(Enemy* enemy, Enemy* otherEnemy, vector2* oldpos, vector2* oldOtherEnemyPos);
void handleTileCollision(SDL_FRect* entity, vector2* oldPos);
void handleEvents();
void horizontalMovement();
void veriticalMovement();
void playerAnimation();
void restart();
void renderStar();
void itemMovement();
void renderMagicMushrooms();
void renderUpMushrooms();
void enemyMovement();
void coinAnimation();
void updateBrick();
void update();
void destroyEverything();

int main()
{
	initialize();
	while (running)
	{
		frameStart = SDL_GetTicks();
		/*deltaTime = (SDL_GetPerformanceCounter() - lastFrameTime) / (float)SDL_GetPerformanceFrequency();
		lastFrameTime = SDL_GetPerformanceCounter();*/
		update();
		frameTime = (int) (SDL_GetTicks() - frameStart);
		if (frameTime < frameDelay)
		{
			SDL_Delay(frameDelay - frameTime);
		}
	}
	destroyEverything();
	return 0;
}

void update()
{
	currentTime = (Uint32)SDL_GetTicks();
	if (!stopMovement)
	{
		if (!player.flagAnimation)
		{
			handleEvents();
		}
		enemyMovement();
		itemMovement();
		getOldVectors();
		horizontalMovement();
		veriticalMovement();
	}
	SDL_SetRenderDrawColor(renderer, 92, 148, 252, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(renderer);
	renderTilemap();
	coinAnimation();
	renderEnemies();
	updateBrick();
	playerAnimation();
	SDL_RenderPresent(renderer);
	updateCamera();
}

void updateCamera()
{
	if (player.rect.x > camera.x + (camera.w / 10 * 4))
	{
		camera.x = player.rect.x - (camera.w / 10 * 4);
	}
	if (camera.x < 0) camera.x = 0;
	else if (camera.x + camera.w > WIDTH_CELL * CELL_SIZE) camera.x = WIDTH_CELL * CELL_SIZE - camera.w;
}

void renderTexture(SDL_Texture* texture, SDL_FRect* srcRect, SDL_FRect* rect, double angle, SDL_FPoint* center, SDL_FlipMode flip)
{
	SDL_FRect newRect = {
		roundf(rect->x - camera.x),
		roundf(rect->y - camera.y),
		rect->w,
		rect->h
	};
	SDL_RenderTextureRotated(renderer, texture, srcRect, &newRect, angle, center, flip);
}

void getOldVectors()
{
	player.oldpos.x = player.rect.x;
	player.oldpos.y = player.rect.y;
	player.oldvelocity_x = player.velocity.x;
	for (Enemy& goomba : goombas)
	{
		goomba.oldpos.x = goomba.rect.x;
		goomba.oldpos.y = goomba.rect.y;
	}
	for (Enemy& koopa : koopas)
	{
		koopa.oldpos.x = koopa.rect.x;
		koopa.oldpos.y = koopa.rect.y;
	}
	magicMushroom.oldpos.x = magicMushroom.rect.x;
	magicMushroom.oldpos.y = magicMushroom.rect.y;
	upMushroom.oldpos.x = upMushroom.rect.x;
	upMushroom.oldpos.y = upMushroom.rect.y;
	star.oldpos.x = star.rect.x;
	star.oldpos.y = star.rect.y;
}

void enemyMovement()
{
	for (Enemy& goomba : goombas)
	{
		if (goomba.rect.x <= camera.x + camera.w && goomba.rect.x + goomba.rect.w >= camera.x - 8 * CELL_SIZE 
			&& goomba.rect.y <= camera.y + camera.h + 8 * CELL_SIZE && !goomba.isDiedByMario && goomba.active)
		{
			goomba.velocity.y += gravity;
			goomba.rect.y += goomba.velocity.y;
			goomba.rect.x += goomba.velocity.x * goomba.direction.normalized().x * CONST;
			for (Enemy& othergoomba : goombas)
			{
				if (&othergoomba != &goomba && !goomba.isDiedByMario && !othergoomba.isDiedByMario && othergoomba.active)
				{
					handleEnemyEnemyCollision(&goomba, &othergoomba, &goomba.oldpos, &othergoomba.oldpos);
				}
			}
			for (Enemy& koopa : koopas)
			{
				if (!goomba.isDiedByStar && !goomba.isDiedByMario && koopa.active)
				{
					handleEnemyEnemyCollision(&goomba, &koopa, &goomba.oldpos, &koopa.oldpos);
				}
			}
			if (goomba.isDiedByStar)
			{
				goomba.velocity.x = 1.5f;
				if (goomba.random == 1) goomba.direction.x = 1;
				else
				{
					goomba.direction.x = -1;
				}
			}
			if (!player.isDied && !goomba.isDiedByStar)
			{
				handlePlayerEnemyCollision(&player, &goomba, &player.oldpos, &goomba.oldpos);
			}
			if (!goomba.isDiedByStar) handleTileCollision(&goomba.rect, &goomba.oldpos);
		}
	}
	for (Enemy& koopa : koopas)
	{
		if (koopa.rect.x <= camera.x + camera.w && koopa.rect.x + koopa.rect.w >= camera.x - 8 * CELL_SIZE 
			&& koopa.rect.y <= camera.y + camera.h + 8 * CELL_SIZE && koopa.active)
		{
			koopa.velocity.y += gravity;
			koopa.rect.y += koopa.velocity.y;
			koopa.rect.x += koopa.velocity.x * koopa.direction.normalized().x * CONST;
			for (Enemy& otherkoopa : koopas)
			{
				if (&otherkoopa != &koopa && !otherkoopa.isDiedByStar && otherkoopa.active)
				{
					handleEnemyEnemyCollision(&koopa, &otherkoopa, &koopa.oldpos, &otherkoopa.oldpos);
				}
			}
			for (Enemy& goomba : goombas)
			{
				if (!goomba.isDiedByStar && !goomba.isDiedByMario && koopa.active)
				{
					handleEnemyEnemyCollision(&koopa, &goomba, &koopa.oldpos, &goomba.oldpos);
				}
			}
			if (koopa.isDiedByStar)
			{
				koopa.velocity.x = 1.5f;
				if (koopa.random == 1) koopa.direction.x = 1;
				else
				{
					koopa.direction.x = -1;
				}
			}
			if (koopa.isDiedByMario && abs(koopa.velocity.x) == 0.75f)
			{
				koopa.velocity.x = 0;
			}
			else if (!koopa.isDiedByMario)
			{
				koopa.velocity.x = 0.75f;
			}
			else if (koopa.isDiedByMario && abs(koopa.velocity.x) == 3)
			{
				if (koopa.rect.x >= camera.x + camera.w || koopa.rect.x + koopa.rect.w <= camera.x) koopa.active = false;
			}
			if (!player.isDied && !koopa.isDiedByStar)
			{
				handlePlayerEnemyCollision(&player, &koopa, &player.oldpos, &koopa.oldpos);
			}
			if (!koopa.isDiedByStar) handleTileCollision(&koopa.rect, &koopa.oldpos);
		}
	}
}

void renderEnemies()
{
	for (Enemy& goomba : goombas)
	{
		if (goomba.rect.x <= camera.x + camera.w && goomba.rect.x + goomba.rect.w >= camera.x &&
			goomba.rect.y <= camera.y + camera.h && goomba.rect.y + goomba.rect.h >= camera.y && goomba.active)
		{
			if (!goomba.isDiedByStar && !goomba.isDiedByMario)
			{
				if (currentTime > goomba.lastFrameTime + enemyAnimFrameDelay && !stopMovement)
				{
					goomba.lastFrameTime = currentTime;
					goomba.frameIndex = (goomba.frameIndex + 1) % 2;
				}
				renderTexture(goombaWalkFrames[goomba.frameIndex], NULL, &goomba.rect, 0, &entityCenter, SDL_FLIP_NONE);
			}
			else if (goomba.isDiedByMario)
			{
				if (currentTime < goomba.lastFrameTime + 400)
				{
					renderTexture(goombaMarioKilledFrame, NULL, &goomba.rect, 0, &entityCenter, SDL_FLIP_NONE);
				}
				else
				{
					goomba.active = false;
				}
			}
			else if (goomba.isDiedByStar)
			{
				renderTexture(goombaDeathFrame, NULL, &goomba.rect, 0, &entityCenter, SDL_FLIP_NONE);
			}
		}
	}
	for (Enemy& koopa : koopas)
	{
		if (koopa.rect.x <= camera.x + camera.w && koopa.rect.x + koopa.rect.w >= camera.x &&
			koopa.rect.y <= camera.y + camera.h && koopa.rect.y + koopa.rect.h >= camera.y && koopa.active)
		{
			SDL_FlipMode flip = koopa.direction.x < 0 ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;
			if (!koopa.isDiedByStar && !koopa.isDiedByMario)
			{
				if (currentTime > koopa.lastFrameTime + enemyAnimFrameDelay && !stopMovement)
				{
					koopa.lastFrameTime = currentTime;
					koopa.frameIndex = (koopa.frameIndex + 1) % 2;
				}
				renderTexture(koopaWalkFrames[koopa.frameIndex], NULL, &koopa.rect, 0, &entityCenter, flip);
			}
			else if (koopa.isDiedByMario)
			{
				renderTexture(koopaMarioKilledFrame, NULL, &koopa.rect, 0, &entityCenter, flip);
				if (currentTime > koopa.lastFrameTime + 5000 && koopa.velocity.x == 0)
				{
					koopa.isDiedByMario = false;
				}
			}
			else if (koopa.isDiedByStar)
			{
				renderTexture(koopaDeathFrame, NULL, &koopa.rect, 0, &entityCenter, flip);
			}
		}
	}
}

void itemMovement()
{	
	if (magicMushroom.active && magicMushroom.rect.x <= camera.x + camera.w && magicMushroom.rect.x + magicMushroom.rect.w >= camera.x &&
		magicMushroom.rect.y <= camera.y + camera.h + 8 * CELL_SIZE && !stopMovement)
	{
		if (magicMushroom.animTimer > 0)
		{
			magicMushroom.rect.y -= 1 * CONST;
			magicMushroom.animTimer--;
		}
		else
		{
			magicMushroom.velocity.y += gravity;
			magicMushroom.rect.y += magicMushroom.velocity.y;
			magicMushroom.rect.x += magicMushroom.velocity.x * magicMushroom.direction.normalized().x * CONST;
			handleTileCollision(&magicMushroom.rect, &magicMushroom.oldpos);
		}
		if (SDL_HasRectIntersectionFloat(&magicMushroom.rect, &player.rect))
		{
			magicMushroom.active = false;
			if (!player.isBig) player.growAnimation = 45;
			player.originalY = player.rect.y;
		}
	}
	else
	{
		magicMushroom.direction.x = 1;
		magicMushroom.active = false;
	}
	if (upMushroom.active && upMushroom.rect.x <= camera.x + camera.w && upMushroom.rect.x + upMushroom.rect.w >= camera.x &&
		upMushroom.rect.y <= camera.y + camera.h + 8 * CELL_SIZE && !stopMovement)
	{
		if (upMushroom.animTimer > 0)
		{
			upMushroom.rect.y -= 1 * CONST;
			upMushroom.animTimer--;
		}
		else
		{
			upMushroom.velocity.y += gravity;
			upMushroom.rect.y += upMushroom.velocity.y;
			upMushroom.rect.x += upMushroom.velocity.x * upMushroom.direction.normalized().x * CONST;
			handleTileCollision(&upMushroom.rect, &upMushroom.oldpos);
		}
		if (SDL_HasRectIntersectionFloat(&upMushroom.rect, &player.rect))
		{
			upMushroom.active = false;
			player.life++;
		}
	}
	else
	{
		upMushroom.direction.x = 1;
		upMushroom.active = false;
	}
	if (star.active && star.rect.x <= camera.x + camera.w && star.rect.x + star.rect.w >= camera.x &&
		star.rect.y <= camera.y + camera.h + 8 * CELL_SIZE && !stopMovement)
	{
		if (star.animTimer > 0)
		{
			star.rect.y -= 1 * CONST;
			star.animTimer--;
		}
		else
		{
			star.velocity.y += gravity;
			star.rect.y += star.velocity.y;
			star.rect.x += star.velocity.x * star.direction.normalized().x * CONST;
			handleTileCollision(&star.rect, &star.oldpos);
		}
		if (SDL_HasRectIntersectionFloat(&star.rect, &player.rect))
		{
			star.active = false;
			player.invisibleAnimation = 500;
			player.starMode = true;
		}
	}
	else
	{
		star.direction.x = 1;
		star.active = false;
	}
}

void coinAnimation()
{
	if (currentTime > coin.lastFrameTime + 10 && !stopMovement)
	{
		coin.lastFrameTime = currentTime;
		coin.frameIndex = (coin.frameIndex + 1) % 4;
	}
	coin.srcRect = { coin.frameIndex * 17.0f, 17, 16, 16 };
	if (coin.animTime > 10) {
		coin.destRect.y -= 7 * CONST;
	}
	else {
		coin.destRect.y += 3.5f * CONST;
	}
	if (coin.animTime <= 0) coin.active = false;
	if (coin.active) renderTexture(miscTexture, &coin.srcRect, &coin.destRect, 0, &entityCenter, SDL_FLIP_NONE);
	coin.animTime--;
}

void renderStar()
{
	if (star.active)
	{
		if (currentTime > star.lastFrameTime + 30 && !stopMovement)
		{
			star.lastFrameTime = currentTime;
			star.frameIndex = (star.frameIndex + 1) % 4;
		}
		star.srcRect = { star.frameIndex * 17.0f, 13 * 17, 16, 16 };
		renderTexture(miscTexture, &star.srcRect, &star.rect, 0, &entityCenter, SDL_FLIP_NONE);
	}
}

void renderMagicMushrooms()
{
	if (magicMushroom.active)
	{
		renderTexture(magicMushroomTexture, NULL, &magicMushroom.rect, 0, &entityCenter, SDL_FLIP_NONE);
	}
}

void renderUpMushrooms()
{
	if (upMushroom.active)
	{
		renderTexture(upMushroomTexture, NULL, &upMushroom.rect, 0, &entityCenter, SDL_FLIP_NONE);
	}
}

void playerAnimation()
{
	player.flip = player.facingRight ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;
	player.animFrameDelay = (int) ((80 * CONST) / abs(player.velocity.x));
	player.sliding = ((player.velocity.x < 0 && player.oldvelocity_x < player.velocity.x && keys[SDL_SCANCODE_RIGHT]) || (player.velocity.x > 0 && player.oldvelocity_x > player.velocity.x && keys[SDL_SCANCODE_LEFT])) && !(keys[SDL_SCANCODE_LEFT] && keys[SDL_SCANCODE_RIGHT]);
	/*player.rect.h = player.isBig ? CELL_SIZE * 4.0f : CELL_SIZE * 2.0f;*/
	if (player.rect.y > camera.y + camera.h) player.isDied = true;
	if (player.isDied && player.life > 0)
	{
		if (currentTime > player.lastFrameTime + 3000) restart();
	}
	if (player.life <= 0) {
		running = false;
	}
	if (player.invisibleAnimation > 0)
	{
		player.invisibleAnimation--;
	}
	else if (player.invisibleAnimation <= 0)
	{
		if (player.starMode) player.starMode = false;
		player.canDie = true;
	}
	if (player.growAnimation > 0)
	{
		player.growAnimation--;
		stopMovement = true;
	}
	else if (player.growAnimation == 0)
	{
		stopMovement = false;
		player.growAnimation--;
	}
	if (!stopMovement && (player.invisibleAnimation / 10) % 2 == 0)
	{
		if (player.velocity.x != 0 && !player.jumping && !player.sliding && !player.isDied)
		{
			if (currentTime > player.lastFrameTime + player.animFrameDelay && player.grounded)
			{
				player.lastFrameTime = currentTime;
				player.frameIndex = (player.frameIndex + 1) % 3;
			}
			if (player.isBig)
			{
				renderTexture(marioBigRunFrames[player.frameIndex], NULL, &player.rect, 0, &entityCenter, player.flip);
			}
			else
			{
				renderTexture(marioSmallRunFrames[player.frameIndex], NULL, &player.rect, 0, &entityCenter, player.flip);
			}
		}
		else if (player.flagAnimation && player.velocity.x == 0)
		{
			if (player.isBig)
			{
				renderTexture(marioBigFlag, NULL, &player.rect, 0, &entityCenter, player.flip);
			}
			else
			{
				renderTexture(marioSmallFlag, NULL, &player.rect, 0, &entityCenter, player.flip);
			}
		}
		else if (player.sliding && !player.jumping && !player.isDied)
		{
			if (player.isBig)
			{
				renderTexture(marioBigSlideFrame, NULL, &player.rect, 0, &entityCenter, player.flip);
			}
			else
			{
				renderTexture(marioSmallSlideFrame, NULL, &player.rect, 0, &entityCenter, player.flip);
			}
		}
		else if (player.jumping && !player.isDied)
		{
			if (player.isBig)
			{
				renderTexture(marioBigJumpFrame, NULL, &player.rect, 0, &entityCenter, player.flip);
			}
			else
			{
				renderTexture(marioSmallJumpFrame, NULL, &player.rect, 0, &entityCenter, player.flip);
			}
		}
		else if (player.isDied)
		{
			renderTexture(marioSmallDeathFrame, NULL, &player.rect, 0, &entityCenter, player.flip);
		}
		else
		{
			if (player.isBig)
			{
				renderTexture(marioBigIdleFrame, NULL, &player.rect, 0, &entityCenter, player.flip);
			}
			else
			{
				renderTexture(marioSmallIdleFrame, NULL, &player.rect, 0, &entityCenter, player.flip);
			}
		}
	}
	else
	{
		if (!player.isBig)
		{
			if (player.growAnimation <= 45 && player.growAnimation >= 38 || player.growAnimation <= 33 &&
				player.growAnimation >= 30 || player.growAnimation <= 25 && player.growAnimation >= 22 &&
				player.growAnimation <= 13 && player.growAnimation >= 10 || player.growAnimation == 1)
			{
				player.rect.y = player.originalY;
				player.rect.h = CELL_SIZE * 2;
				renderTexture(marioSmallIdleFrame, NULL, &player.rect, 0, &entityCenter, player.flip);
			}
			else if (player.growAnimation <= 37 && player.growAnimation >= 34 || player.growAnimation <= 29 &&
				player.growAnimation >= 26 || player.growAnimation <= 21 && player.growAnimation >= 18 ||
				player.growAnimation <= 9 && player.growAnimation >= 6)
			{
				player.rect.y = player.originalY - CELL_SIZE;
				player.rect.h = CELL_SIZE * 3;
				renderTexture(marioMiddleFrame, NULL, &player.rect, 0, &entityCenter, player.flip);
			}
			else
			{
				if (player.growAnimation > 0)
				{
					player.rect.y = player.originalY - CELL_SIZE * 2;
					player.rect.h = CELL_SIZE * 4;
					renderTexture(marioBigIdleFrame, NULL, &player.rect, 0, &entityCenter, player.flip);
				}
				else if (player.growAnimation == 0)
				{
					player.isBig = true;
					player.rect.y = player.originalY - CELL_SIZE * 2;
					player.rect.h = CELL_SIZE * 4;
				}
			}
		}
		else
		{
			if (player.growAnimation <= 45 && player.growAnimation >= 38 || player.growAnimation <= 33 &&
				player.growAnimation >= 30 || player.growAnimation <= 25 && player.growAnimation >= 22 &&
				player.growAnimation <= 13 && player.growAnimation >= 10 || player.growAnimation == 1)
			{
				player.rect.y = player.originalY;
				player.rect.h = CELL_SIZE * 4;
				renderTexture(marioBigIdleFrame, NULL, &player.rect, 0, &entityCenter, player.flip);
			}
			else if (player.growAnimation <= 37 && player.growAnimation >= 34 || player.growAnimation <= 29 &&
				player.growAnimation >= 26 || player.growAnimation <= 21 && player.growAnimation >= 18 ||
				player.growAnimation <= 9 && player.growAnimation >= 6)
			{
				player.rect.y = player.originalY + CELL_SIZE;
				player.rect.h = CELL_SIZE * 3;
				renderTexture(marioMiddleFrame, NULL, &player.rect, 0, &entityCenter, player.flip);
			}
			else
			{
				if (player.growAnimation > 0)
				{
					player.rect.y = player.originalY + CELL_SIZE * 2;
					player.rect.h = CELL_SIZE * 2;
					renderTexture(marioSmallIdleFrame, NULL, &player.rect, 0, &entityCenter, player.flip);
				}
				else if (player.growAnimation == 0)
				{
					player.isBig = false;
					player.oldpos.y = player.originalY + CELL_SIZE * 2;
					player.rect.y = player.originalY + CELL_SIZE * 2;
					player.rect.h = CELL_SIZE * 2;
					player.canDie = false;
					player.invisibleAnimation = 200;
				}
			}
		}
	}
}

void veriticalMovement()
{
	if (!player.flagAnimation)
	{
		player.velocity.y += gravity;
	}
	else
	{
		player.velocity.y = 2 * CONST;
	}
	player.falling = player.velocity.y > gravity || !keys[SDL_SCANCODE_X];
	if (keys[SDL_SCANCODE_X] && !player.jumping && tmpKey == false && !player.isDied && !player.falling)
	{
		player.jumpTime = 2.45f * CONST;
		player.velocity.y = -player.jumpTime;
		player.jumping = true;
		player.grounded = false;
	}
	else if (keys[SDL_SCANCODE_X] && !player.falling && player.jumpTime < 3.45f * CONST && !player.isDied)
	{
		player.jumpTime += 0.1f * CONST;
		player.velocity.y = min(player.velocity.y, -player.jumpTime);
	}
	else if (player.falling && !player.isDied)
	{
		player.jumpTime = 3.45f * CONST;
		player.grounded = false;
	}
	player.rect.y += player.velocity.y;
	if (!player.isDied)
	{
		handleTileCollision(&player.rect, &player.oldpos);
	}
}

void horizontalMovement()
{
	maxSpeed = (keys[SDL_SCANCODE_Z] && player.grounded) ? 3 * CONST : 1.5f * CONST;
	acceleration = (keys[SDL_SCANCODE_Z] && player.grounded && player.velocity.x >= 2) ? 0.2f * CONST : 0.1f * CONST;
	// friction = (keys[SDL_SCANCODE_Z] && player.grounded) ? 0.04f * CONST : 0.04f * CONST;
	if (keys[SDL_SCANCODE_RIGHT] && player.velocity.x < maxSpeed)
	{
		player.velocity.x += acceleration;
		if (player.velocity.x > maxSpeed) player.velocity.x = maxSpeed;
	}
	if (keys[SDL_SCANCODE_LEFT] && player.velocity.x > -maxSpeed)
	{
		player.velocity.x -= acceleration;
		if (player.velocity.x < -maxSpeed) player.velocity.x = -maxSpeed;
	}
	if (player.velocity.x > 0 && !player.jumping)
	{
		player.facingRight = true;
		player.velocity.x -= friction;
		if (player.velocity.x < 0) player.velocity.x = 0;
	}
	else if (player.velocity.x < 0 && !player.jumping)
	{
		player.facingRight = false;
		player.velocity.x += friction;
		if (player.velocity.x > 0) player.velocity.x = 0;
	}
	if (player.rect.x < camera.x)
	{
		if (keys[SDL_SCANCODE_LEFT]) player.velocity.x = -1;
		else
		{
			player.velocity.x = 0;
		}
		if (keys[SDL_SCANCODE_RIGHT]) player.velocity.x = 0;
		player.rect.x = camera.x;
	}
	if (player.flagAnimation)
	{
		if (!player.grounded) player.velocity.x = 0;
		else
		{
			player.velocity.x = 1.5f;
		}
	}
	if (!player.isDied) player.rect.x += player.velocity.x;
}

void loadCollision(bool restart)
{
	ifstream file("assets/TileMap/collisions.map");
	if (!file) {
		cerr << "Failed to open file: " << "collisions.map" << std::endl;
		return;
	}

	for (int row = 0; row < HEIGHT_CELL / 2; row++) {
		for (int col = 0; col < WIDTH_CELL / 2; col++) {
			file >> collisiontiles[row][col].tilemap;
			collisiontiles[row][col].destRect = { (float)col * CELL_SIZE * 2 + 1 * CONST, (float)row * CELL_SIZE * 2 + 1 * CONST, CELL_SIZE * 2 - 2 * CONST, CELL_SIZE * 2 - 2 * CONST };
			collisiontiles[row][col].originalY = collisiontiles[row][col].destRect.y;
			if (collisiontiles[row][col].tilemap == 9 && restart) collisiontiles[row][col].tilemap = -1;
			collisiontiles[row][col].restart();
		}
	}
	
	file.close();
}

void handleEnemyEnemyCollision(Enemy* enemy, Enemy* otherEnemy, vector2* oldpos, vector2* oldOtherEnemyPos)
{
	if (enemy->rect.x + enemy->rect.w > otherEnemy->rect.x &&
		otherEnemy->rect.x + otherEnemy->rect.w > enemy->rect.x &&
		enemy->rect.y + enemy->rect.h > otherEnemy->rect.y &&
		otherEnemy->rect.y + otherEnemy->rect.h > enemy->rect.y)
	{
		if (oldpos->y + enemy->rect.h <= otherEnemy->rect.y && enemy->rect.y + enemy->rect.h >= otherEnemy->rect.y)
		{
			enemy->rect.y = otherEnemy->rect.y - enemy->rect.h;
			otherEnemy->rect.y = enemy->rect.y + enemy->rect.h;
		}
		else if (oldpos->x >= oldOtherEnemyPos->x + otherEnemy->rect.w && enemy->rect.x <= otherEnemy->rect.x + otherEnemy->rect.w)
		{
			enemy->rect.x = otherEnemy->rect.x + otherEnemy->rect.w;
			otherEnemy->rect.x = enemy->rect.x - enemy->rect.w;
			if (enemy->isDiedByMario && enemy->velocity.x > 0)
			{
				otherEnemy->isDiedByStar = true;
			}
			else if (!enemy->isDiedByMario || enemy->isDiedByMario && enemy->velocity.x == 0)
			{
				enemy->direction.x = -enemy->direction.x;
			}
		}
		else if (oldpos->x + enemy->rect.w <= oldOtherEnemyPos->x && enemy->rect.x + enemy->rect.w >= otherEnemy->rect.x)
		{
			enemy->rect.x = otherEnemy->rect.x - enemy->rect.w;
			otherEnemy->rect.x = enemy->rect.x + enemy->rect.w;
			if (enemy->isDiedByMario && enemy->velocity.x > 0)
			{
				otherEnemy->isDiedByStar = true;
			}
			else if (!enemy->isDiedByMario || enemy->isDiedByMario && enemy->velocity.x == 0)
			{
				enemy->direction.x = -enemy->direction.x;
			}
		}
		else if (oldpos->y >= oldOtherEnemyPos->y + otherEnemy->rect.h && enemy->rect.y <= otherEnemy->rect.y + otherEnemy->rect.h)
		{
			enemy->rect.y = otherEnemy->rect.y + otherEnemy->rect.h;
			otherEnemy->rect.y = enemy->rect.y - enemy->rect.h;
		}
	}
}

void handlePlayerEnemyCollision(Player* player, Enemy* enemy, vector2* oldplayerpos, vector2* oldenemypos)
{
	if (player->rect.x + player->rect.w > enemy->rect.x &&
		enemy->rect.x + enemy->rect.w > player->rect.x &&
		player->rect.y + player->rect.h > enemy->rect.y &&
		enemy->rect.y + enemy->rect.h > player->rect.y)
	{
		if (oldplayerpos->y + player->rect.h <= enemy->rect.y && player->rect.y + player->rect.h >= enemy->rect.y)
		{
			if (!player->starMode)
			{
				player->velocity.y = -2.4f * CONST;
				player->grounded = false;
				if (enemy->isDiedByMario)
				{
					if (oldplayerpos->x < oldenemypos->x)
					{
						enemy->direction.x = 1;
						enemy->velocity.x = 3;
					}
					else if (oldplayerpos->x > oldenemypos->x)
					{
						enemy->direction.x = -1;
						enemy->velocity.x = 3;
					}
				}
				enemy->isDiedByMario = true;
			}
			else
			{
				enemy->isDiedByStar = true;
			}
		}
		else if (oldplayerpos->x >= oldenemypos->x + enemy->rect.w && player->rect.x <= enemy->rect.x + enemy->rect.w)
		{
			if (!player->starMode)
			{
				if (!enemy->isDiedByMario && !enemy->isDiedByStar)
				{
					if (player->isBig)
					{
						player->originalY = player->rect.y;
						player->growAnimation = 45;
					}
					else
					{
						if (player->canDie)
						{
							player->velocity.x = 0;
							player->isDied = true;
							player->velocity.y = -4.8f * CONST;
						}
					}
				}
				else if (enemy->isDiedByMario)
				{
					if (enemy->velocity.x == 0)
					{
						enemy->direction.x = -1;
						enemy->velocity.x = 3;
					}
					else if (enemy->velocity.x != 0)
					{
						if (player->isBig)
						{
							player->originalY = player->rect.y;
							player->growAnimation = 45;
						}
						else
						{
							if (player->canDie)
							{
								player->velocity.x = 0;
								player->isDied = true;
								player->velocity.y = -4.8f * CONST;
							}
						}
					}
				}
			}
			else
			{
				enemy->isDiedByStar = true;
			}
		}
		else if (oldplayerpos->x + player->rect.w <= oldenemypos->x && player->rect.x + player->rect.w >= enemy->rect.x)
		{
			if (!player->starMode)
			{
				if (!enemy->isDiedByMario && !enemy->isDiedByStar)
				{
					if (player->isBig)
					{
						player->originalY = player->rect.y;
						player->growAnimation = 45;
					}
					else
					{
						if (player->canDie)
						{
							player->velocity.x = 0;
							player->isDied = true;
							player->velocity.y = -4.8f * CONST;
						}
					}
				}
				else if (enemy->isDiedByMario)
				{
					if (enemy->velocity.x == 0)
					{
						enemy->direction.x = 1;
						enemy->velocity.x = 3;
					}
					else if (enemy->velocity.x != 0)
					{
						if (player->isBig)
						{
							player->originalY = player->rect.y;
							player->growAnimation = 45;
						}
						else
						{
							if (player->canDie)
							{
								player->velocity.x = 0;
								player->isDied = true;
								player->velocity.y = -4.8f * CONST;
							}
						}
					}
				}
			}
			else
			{
				enemy->isDiedByStar = true;
			}
		}
		else if (oldplayerpos->y >= oldenemypos->y + enemy->rect.h && player->rect.y <= enemy->rect.y + enemy->rect.h)
		{
			if (!player->starMode)
			{
				if (player->isBig)
				{
					player->originalY = player->rect.y;
					player->growAnimation = 45;
				}
				else
				{
					if (player->canDie)
					{
						player->velocity.x = 0;
						player->isDied = true;
						player->velocity.y = -4.8f * CONST;
					}
				}
			}
			else
			{
				enemy->isDiedByStar = true;
			}
		}
	}
}

void handleTileCollision(SDL_FRect* entity, vector2* oldPos)
{
	for (int row = 0; row < HEIGHT_CELL / 2; row++) {
		for (int col = 0; col < WIDTH_CELL / 2; col++) {
			if (collisiontiles[row][col].tilemap == -1 || !collisiontiles[row][col].active) continue;
			if (entity->x + entity->w > collisiontiles[row][col].destRect.x &&
				collisiontiles[row][col].destRect.x + collisiontiles[row][col].destRect.w > entity->x &&
				entity->y + entity->h > collisiontiles[row][col].destRect.y &&
				collisiontiles[row][col].destRect.y + collisiontiles[row][col].destRect.h > entity->y)
				{
					if (entity == &player.rect && collisiontiles[row][col].tilemap == 12)
					{
						player.flagAnimation = true;
						if (player.velocity.x == 0) player.rect.x = collisiontiles[row][col].destRect.x + 8 - player.rect.w;
					}
					if (entity == &player.rect && collisiontiles[row][col].tilemap == 13) running = false;
					if (oldPos->y + entity->h <= collisiontiles[row][col].destRect.y &&
						collisiontiles[row][col].tilemap != 9 && collisiontiles[row][col].tilemap != 12)
					{
						entity->y = collisiontiles[row][col].destRect.y - entity->h;
						for (Enemy& goomba : goombas)
						{
							if (entity == &goomba.rect)
							{
								goomba.velocity.y = 0;
							}
						}
						for (Enemy& koopa : koopas)
						{
							if (entity == &koopa.rect)
							{
								koopa.velocity.y = 0;
							}
						}
						if (entity == &magicMushroom.rect)
						{
							magicMushroom.velocity.y = 0;
						}
						if (entity == &upMushroom.rect)
						{
							upMushroom.velocity.y = 0;
						}
						if (entity == &star.rect)
						{
							star.velocity.y = -5 * CONST;
						}
						if (entity == &player.rect)
						{
							player.velocity.y = 0;
							tmpKey = keys[SDL_SCANCODE_X];
							player.jumping = false;
							player.grounded = true;
						}
					}
					else if (oldPos->x >= collisiontiles[row][col].destRect.x + collisiontiles[row][col].destRect.w &&
						collisiontiles[row][col].tilemap != 9 && collisiontiles[row][col].tilemap != 12)
					{
						entity->x = collisiontiles[row][col].destRect.x + collisiontiles[row][col].destRect.w;
						for (Enemy& goomba : goombas)
						{
							if (entity == &goomba.rect)
							{
								goomba.direction.x = 1;
							}
						}
						for (Enemy& koopa : koopas)
						{
							if (entity == &koopa.rect)
							{
								koopa.direction.x = 1;
							}
						}
						if (entity == &magicMushroom.rect)
						{
							magicMushroom.direction.x = 1;
						}
						if (entity == &upMushroom.rect)
						{
							upMushroom.direction.x = 1;
						}
						if (entity == &star.rect)
						{
							star.direction.x = 1;
						}
						if (entity == &player.rect)
						{
							if (keys[SDL_SCANCODE_LEFT]) player.velocity.x = -1;
							else
							{
								player.velocity.x = 0;
							}
							if (keys[SDL_SCANCODE_RIGHT]) player.velocity.x = 0;
						}
					}
					else if (oldPos->x + entity->w <= collisiontiles[row][col].destRect.x &&
						collisiontiles[row][col].tilemap != 9 && collisiontiles[row][col].tilemap != 12)
					{
						entity->x = collisiontiles[row][col].destRect.x - entity->w;
						for (Enemy& goomba : goombas)
						{
							if (entity == &goomba.rect)
							{
								goomba.direction.x = -1;
							}

						}
						for (Enemy& koopa : koopas)
						{
							if (entity == &koopa.rect)
							{
								koopa.direction.x = -1;
							}
						}
						if (entity == &magicMushroom.rect)
						{
							magicMushroom.direction.x = -1;
						}
						if (entity == &upMushroom.rect)
						{
							upMushroom.direction.x = -1;
						}
						if (entity == &star.rect)
						{
							star.direction.x = -1;
						}
						if (entity == &player.rect)
						{	
							if (keys[SDL_SCANCODE_RIGHT]) player.velocity.x = 1;
							else
							{
								player.velocity.x = 0;
							}
							if (keys[SDL_SCANCODE_LEFT]) player.velocity.x = 0;
						}
					}
					else if (oldPos->y >= collisiontiles[row][col].destRect.y + collisiontiles[row][col].destRect.h && collisiontiles[row][col].tilemap != 12)
					{
						if (entity != &star.rect)
						{
							entity->y = collisiontiles[row][col].destRect.y + collisiontiles[row][col].destRect.h;
						}
						if (entity == &player.rect)
						{
							player.velocity.y = 0;
							if ((col + 1) < (WIDTH_CELL / 2) && (col - 1) >= 0)
							{
								if ((collisiontiles[row][col].tilemap == 3 || collisiontiles[row][col].tilemap == 4 || collisiontiles[row][col].tilemap == 9 ||
									collisiontiles[row][col].tilemap == 10 || collisiontiles[row][col].tilemap == 11) && !collisiontiles[row][col - 1].isCollided && !collisiontiles[row][col + 1].isCollided)
								{
									if (collisiontiles[row][col].tilemap == 11 && coin.number >= 0)
									{
										coin.active = true;
										coin.destRect = { collisiontiles[row][col].destRect.x ,collisiontiles[row][col].destRect.y - collisiontiles[row][col].destRect.h, collisiontiles[row][col].destRect.w, collisiontiles[row][col].destRect.h };;
										coin.animTime = 16;
										coin.frameIndex = 0;
									}
									if (collisiontiles[row][col].tilemap == 3)
									{
										coin.active = true;
										coin.destRect = { collisiontiles[row][col].destRect.x ,collisiontiles[row][col].destRect.y - collisiontiles[row][col].destRect.h, collisiontiles[row][col].destRect.w, collisiontiles[row][col].destRect.h };
										coin.animTime = 16;
										coin.frameIndex = 0;
										tile[2 * row][2 * col].tilemap = 31;
										tile[2 * row][2 * col + 1].tilemap = 32;
										tile[2 * row + 1][2 * col].tilemap = 35;
										tile[2 * row + 1][2 * col + 1].tilemap = 36;
										collisiontiles[row][col].tilemap = 0;
									}
									else if (collisiontiles[row][col].tilemap == 10)
									{
										star.active = true;
										star.rect = collisiontiles[row][col].destRect;
										star.animTimer = 17;
										star.frameIndex = 0;
										tile[2 * row][2 * col].tilemap = 31;
										tile[2 * row][2 * col + 1].tilemap = 32;
										tile[2 * row + 1][2 * col].tilemap = 35;
										tile[2 * row + 1][2 * col + 1].tilemap = 36;
										collisiontiles[row][col].tilemap = 0;
									}
									else if (collisiontiles[row][col].tilemap == 4)
									{
										magicMushroom.active = true;
										magicMushroom.rect = { collisiontiles[row][col].destRect.x, collisiontiles[row][col].originalY, collisiontiles[row][col].destRect.w, collisiontiles[row][col].destRect.h };
										magicMushroom.animTimer = 17;
										tile[2 * row][2 * col].tilemap = 31;
										tile[2 * row][2 * col + 1].tilemap = 32;
										tile[2 * row + 1][2 * col].tilemap = 35;
										tile[2 * row + 1][2 * col + 1].tilemap = 36;
										collisiontiles[row][col].tilemap = 0;
									}
									else if (collisiontiles[row][col].tilemap == 9)
									{
										upMushroom.active = true;
										upMushroom.rect = { collisiontiles[row][col].destRect.x, collisiontiles[row][col].originalY, collisiontiles[row][col].destRect.w, collisiontiles[row][col].destRect.h };
										upMushroom.animTimer = 17;
										tile[2 * row][2 * col].tilemap = 31;
										tile[2 * row][2 * col + 1].tilemap = 32;
										tile[2 * row + 1][2 * col].tilemap = 35;
										tile[2 * row + 1][2 * col + 1].tilemap = 36;
										collisiontiles[row][col].tilemap = 0;
									}
									collisiontiles[row][col].anim = true;
									collisiontiles[row][col].isCollided = true;
									collisiontiles[row][col].bounceTimer = 20;
								}
								else if ((collisiontiles[row][col].tilemap == 1 || collisiontiles[row][col].tilemap == 2) && !collisiontiles[row][col - 1].isCollided && !collisiontiles[row][col + 1].isCollided)
								{
									collisiontiles[row][col].anim = true;
									collisiontiles[row][col].isCollided = true;
									if (!player.isBig)
									{
										collisiontiles[row][col].bounceTimer = 20;
									}
									else
									{
										collisiontiles[row][col].bounceTimer = 80;
										angle = 0;
									}	
								}
							}
						}
					}
				}
			}
		}
	}

void updateBrick()
{
	for (int row = 0; row < HEIGHT_CELL / 2; row++) {
		for (int col = 0; col < WIDTH_CELL / 2; col++) {
			if (collisiontiles[row][col].anim) {
				for (Enemy& goomba : goombas)
				{
					if (SDL_HasRectIntersectionFloat(&collisiontiles[row][col].destRect, &goomba.rect) && collisiontiles[row][col].active &&
						((collisiontiles[row][col].bounceTimer == 20 && !player.isBig) || (collisiontiles[row][col].bounceTimer == 80 && player.isBig)))
					{
						goomba.isDiedByStar = true;
					}
				}
				for (Enemy& koopa : koopas)
				{
					if (SDL_HasRectIntersectionFloat(&collisiontiles[row][col].destRect, &koopa.rect) && collisiontiles[row][col].active &&
						((collisiontiles[row][col].bounceTimer == 20 && !player.isBig) || (collisiontiles[row][col].bounceTimer == 80 && player.isBig)))
					{
						koopa.isDiedByStar = true;
					}
				}
				if (SDL_HasRectIntersectionFloat(&collisiontiles[row][col].destRect, &magicMushroom.rect) && collisiontiles[row][col].active &&
					magicMushroom.rect.x != collisiontiles[row][col].destRect.x && magicMushroom.rect.y != collisiontiles[row][col].destRect.y && collisiontiles[row][col].bounceTimer == 20)
				{
					magicMushroom.velocity.y = -2.4f * CONST;
					magicMushroom.direction.x = -magicMushroom.direction.x;
				}
				if (collisiontiles[row][col].tilemap == 11 && collisiontiles[row][col].bounceTimer == 19) coin.number--;
				if (collisiontiles[row][col].tilemap == 11 && coin.number == 0 && collisiontiles[row][col].bounceTimer == 20)
				{
					tile[2 * row][2 * col].tilemap = 31;
					tile[2 * row][2 * col + 1].tilemap = 32;
					tile[2 * row + 1][2 * col].tilemap = 35;
					tile[2 * row + 1][2 * col + 1].tilemap = 36;
					collisiontiles[row][col].tilemap = 0;
				}
				if (collisiontiles[row][col].bounceTimer == 15) collisiontiles[row][col].isCollided = false;
				if (collisiontiles[row][col].bounceTimer > 10) {
					tile[2 * row][2 * col].destRect.y -= 0.75f * CONST;
					tile[2 * row][2 * col + 1].destRect.y -= 0.75f * CONST;
					tile[2 * row + 1][2 * col].destRect.y -= 0.75f * CONST;
					tile[2 * row + 1][2 * col + 1].destRect.y -= 0.75f * CONST;
					/*collisiontiles[row][col].destrect.y -= 0.75f * const;*/
				}
				else {
					tile[2 * row][2 * col].destRect.y += 0.75f * CONST;
					tile[2 * row][2 * col + 1].destRect.y += 0.75f * CONST;
					tile[2 * row + 1][2 * col].destRect.y += 0.75f * CONST;
					tile[2 * row + 1][2 * col + 1].destRect.y += 0.75f * CONST;
					/*collisiontiles[row][col].destRect.y += 0.75f * CONST;*/
				}

				if (player.isBig && (collisiontiles[row][col].tilemap == 1 || collisiontiles[row][col].tilemap == 2))
				{
					tile[2 * row][2 * col].active = false;
					tile[2 * row][2 * col + 1].active = false;
					tile[2 * row + 1][2 * col].active = false;
					tile[2 * row + 1][2 * col + 1].active = false;
					if (collisiontiles[row][col].bounceTimer == 79)
					{
						collisiontiles[row][col].active = false;
					}
					if (collisiontiles[row][col].bounceTimer == 75) collisiontiles[row][col].isCollided = false;
					else if (collisiontiles[row][col].bounceTimer > 60)
					{
						tile[2 * row][2 * col].destRect.y -= 3 * CONST;
						tile[2 * row][2 * col + 1].destRect.y -= 3 * CONST;
						tile[2 * row + 1][2 * col].destRect.y -= 0.5f * CONST;
						tile[2 * row + 1][2 * col + 1].destRect.y -= 0.5f * CONST;
					}
					else
					{
						tile[2 * row][2 * col].destRect.y += 5 * CONST;
						tile[2 * row][2 * col + 1].destRect.y += 5 * CONST;
						tile[2 * row + 1][2 * col].destRect.y += 5 * CONST;
						tile[2 * row + 1][2 * col + 1].destRect.y += 5 * CONST;
					}
					tile[2 * row][2 * col].destRect.x -= 1 * CONST;
					tile[2 * row][2 * col + 1].destRect.x += 1 * CONST;
					tile[2 * row + 1][2 * col].destRect.x -= 1 * CONST;
					tile[2 * row + 1][2 * col + 1].destRect.x += 1 * CONST;
					angle += 30;
					renderTexture(miscTexture, &srect1, &tile[2 * row][2 * col].destRect, -angle, &center, SDL_FLIP_HORIZONTAL);
					renderTexture(miscTexture, &srect2, &tile[2 * row][2 * col + 1].destRect, angle, &center, SDL_FLIP_HORIZONTAL);
					renderTexture(miscTexture, &srect3, &tile[2 * row + 1][2 * col].destRect, -angle, &center, SDL_FLIP_HORIZONTAL);
					renderTexture(miscTexture, &srect4, &tile[2 * row + 1][2 * col + 1].destRect, angle, &center, SDL_FLIP_HORIZONTAL);
				}

				collisiontiles[row][col].bounceTimer--;

				if (collisiontiles[row][col].bounceTimer <= 0) {
					/*collisiontiles[row][col].destRect.y = collisiontiles[row][col].originalY;*/
					tile[2 * row][2 * col].destRect.y = tile[2 * row][2 * col].originalY;
					tile[2 * row][2 * col + 1].destRect.y = tile[2 * row][2 * col + 1].originalY;
					tile[2 * row + 1][2 * col].destRect.y = tile[2 * row + 1][2 * col].originalY;
					tile[2 * row + 1][2 * col + 1].destRect.y = tile[2 * row + 1][2 * col + 1].originalY;
					collisiontiles[row][col].anim = false;
				}
			}
		}
	}
}

void renderTilemap()
{
	if (magicMushroom.animTimer > 0) renderMagicMushrooms();
	if (upMushroom.animTimer > 0) renderUpMushrooms();
	if (star.animTimer > 0) renderStar();
	for (int row = 0; row < HEIGHT_CELL; row++) {
		for (int col = 0; col < WIDTH_CELL; col++) {
			if (tile[row][col].tilemap == 0) continue;
			SDL_FRect srcRect = { (float)(tile[row][col].tilemap % 32) * 8, (float)floor(tile[row][col].tilemap / 32) * 8, 8, 8 };
			if (tile[row][col].destRect.x <= camera.x + camera.w && tile[row][col].destRect.x + tile[row][col].destRect.w >= camera.x && tile[row][col].active)
			{
				renderTexture(tileset_texture, &srcRect, &tile[row][col].destRect, 0, &center, SDL_FLIP_NONE);
			}
		}
	}
	if (magicMushroom.animTimer <= 0) renderMagicMushrooms();
	if (upMushroom.animTimer <= 0) renderUpMushrooms();
	if (star.animTimer <= 0) renderStar();
}

void loadTilemap() {
	ifstream file("assets/TileMap/tiledAll.map");
	if (!file) {
		cerr << "Failed to open file: " << "tiledAll.map" << std::endl;
		return; 
	}
	for (int row = 0; row < HEIGHT_CELL; row++) {
		for (int col = 0; col < WIDTH_CELL; col++) {
			file >> tile[row][col].tilemap;
			tile[row][col].destRect = { (float)col * CELL_SIZE, (float)row * CELL_SIZE, CELL_SIZE, CELL_SIZE };
			tile[row][col].originalY = tile[row][col].destRect.y;
			tile[row][col].restart();
		}
	}

	file.close();
}

void restart()
{
	player.life--;
	player.rect = { CELL_SIZE * 5, WINDOW_HEIGHT - 6 * CELL_SIZE , CELL_SIZE * 2, CELL_SIZE * 2 };
	goombas[0].rect = { 44 * CELL_SIZE, WINDOW_HEIGHT - 6 * CELL_SIZE, CELL_SIZE * 2, CELL_SIZE * 2 };
	goombas[1].rect = { 80 * CELL_SIZE, WINDOW_HEIGHT - 6 * CELL_SIZE, CELL_SIZE * 2, CELL_SIZE * 2 };
	goombas[2].rect = { 102 * CELL_SIZE, WINDOW_HEIGHT - 6 * CELL_SIZE, CELL_SIZE * 2, CELL_SIZE * 2 };
	goombas[3].rect = { 105 * CELL_SIZE, WINDOW_HEIGHT - 6 * CELL_SIZE, CELL_SIZE * 2, CELL_SIZE * 2 };
	goombas[4].rect = { 160 * CELL_SIZE, WINDOW_HEIGHT - 22 * CELL_SIZE, CELL_SIZE * 2, CELL_SIZE * 2 };
	goombas[5].rect = { 164 * CELL_SIZE, WINDOW_HEIGHT - 22 * CELL_SIZE, CELL_SIZE * 2, CELL_SIZE * 2 };
	goombas[6].rect = { 194 * CELL_SIZE, WINDOW_HEIGHT - 6 * CELL_SIZE, CELL_SIZE * 2, CELL_SIZE * 2 };
	goombas[7].rect = { 197 * CELL_SIZE, WINDOW_HEIGHT - 6 * CELL_SIZE, CELL_SIZE * 2, CELL_SIZE * 2 };
	koopas[0].rect = { 214 * CELL_SIZE, WINDOW_HEIGHT - 6 * CELL_SIZE, CELL_SIZE * 2, CELL_SIZE * 2 };
	goombas[8].rect = { 228 * CELL_SIZE, WINDOW_HEIGHT - 6 * CELL_SIZE, CELL_SIZE * 2, CELL_SIZE * 2 };
	goombas[9].rect = { 231 * CELL_SIZE, WINDOW_HEIGHT - 6 * CELL_SIZE, CELL_SIZE * 2, CELL_SIZE * 2 };
	goombas[10].rect = { 248 * CELL_SIZE, WINDOW_HEIGHT - 6 * CELL_SIZE, CELL_SIZE * 2, CELL_SIZE * 2 };
	goombas[11].rect = { 251 * CELL_SIZE, WINDOW_HEIGHT - 6 * CELL_SIZE, CELL_SIZE * 2, CELL_SIZE * 2 };
	goombas[12].rect = { 256 * CELL_SIZE, WINDOW_HEIGHT - 6 * CELL_SIZE, CELL_SIZE * 2, CELL_SIZE * 2 };
	goombas[13].rect = { 259 * CELL_SIZE, WINDOW_HEIGHT - 6 * CELL_SIZE, CELL_SIZE * 2, CELL_SIZE * 2 };
	goombas[14].rect = { 348 * CELL_SIZE, WINDOW_HEIGHT - 6 * CELL_SIZE, CELL_SIZE * 2, CELL_SIZE * 2 };
	goombas[15].rect = { 351 * CELL_SIZE, WINDOW_HEIGHT - 6 * CELL_SIZE, CELL_SIZE * 2, CELL_SIZE * 2 };
	camera = { 0 , 0 , WINDOW_WIDTH, WINDOW_HEIGHT };
	for (Enemy& goomba : goombas) goomba.restart();
	for (Enemy& koopa : koopas) koopa.restart();
	upMushroom.restart();
	magicMushroom.restart();
	coin.restart();
	loadTilemap();
	loadCollision(true);
	player.restart();	
}
void initialize()
{
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		cerr << "SDL Init Error: " << SDL_GetError() << endl;
		running = false;
	}

	if (!SDL_CreateWindowAndRenderer("Super Mario Bros", WINDOW_WIDTH, WINDOW_HEIGHT, 0, &window, &renderer))
	{
		cerr << "SDL Init Error: " << SDL_GetError() << endl;
		running = false;
	}

	player.rect = { CELL_SIZE * 5, WINDOW_HEIGHT - 6 * CELL_SIZE , CELL_SIZE * 2, CELL_SIZE * 2 };
	goombas[0].rect = { 44 * CELL_SIZE, WINDOW_HEIGHT - 6 * CELL_SIZE, CELL_SIZE * 2, CELL_SIZE * 2 };
	goombas[1].rect = { 80 * CELL_SIZE, WINDOW_HEIGHT - 6 * CELL_SIZE, CELL_SIZE * 2, CELL_SIZE * 2 };
	goombas[2].rect = { 102 * CELL_SIZE, WINDOW_HEIGHT - 6 * CELL_SIZE, CELL_SIZE * 2, CELL_SIZE * 2 };
	goombas[3].rect = { 105 * CELL_SIZE, WINDOW_HEIGHT - 6 * CELL_SIZE, CELL_SIZE * 2, CELL_SIZE * 2 };
	goombas[4].rect = { 160 * CELL_SIZE, WINDOW_HEIGHT - 22 * CELL_SIZE, CELL_SIZE * 2, CELL_SIZE * 2 };
	goombas[5].rect = { 164 * CELL_SIZE, WINDOW_HEIGHT - 22 * CELL_SIZE, CELL_SIZE * 2, CELL_SIZE * 2 };
	goombas[6].rect = { 194 * CELL_SIZE, WINDOW_HEIGHT - 6 * CELL_SIZE, CELL_SIZE * 2, CELL_SIZE * 2 };
	goombas[7].rect = { 197 * CELL_SIZE, WINDOW_HEIGHT - 6 * CELL_SIZE, CELL_SIZE * 2, CELL_SIZE * 2 };
	koopas[0].rect = { 214 * CELL_SIZE, WINDOW_HEIGHT - 6 * CELL_SIZE, CELL_SIZE * 2, CELL_SIZE * 2};
	goombas[8].rect = { 228 * CELL_SIZE, WINDOW_HEIGHT - 6 * CELL_SIZE, CELL_SIZE * 2, CELL_SIZE * 2 };
	goombas[9].rect = { 231 * CELL_SIZE, WINDOW_HEIGHT - 6 * CELL_SIZE, CELL_SIZE * 2, CELL_SIZE * 2 };
	goombas[10].rect = { 248 * CELL_SIZE, WINDOW_HEIGHT - 6 * CELL_SIZE, CELL_SIZE * 2, CELL_SIZE * 2 };
	goombas[11].rect = { 251 * CELL_SIZE, WINDOW_HEIGHT - 6 * CELL_SIZE, CELL_SIZE * 2, CELL_SIZE * 2 };
	goombas[12].rect = { 256 * CELL_SIZE, WINDOW_HEIGHT - 6 * CELL_SIZE, CELL_SIZE * 2, CELL_SIZE * 2 };
	goombas[13].rect = { 259 * CELL_SIZE, WINDOW_HEIGHT - 6 * CELL_SIZE, CELL_SIZE * 2, CELL_SIZE * 2 };
	goombas[14].rect = { 348 * CELL_SIZE, WINDOW_HEIGHT - 6 * CELL_SIZE, CELL_SIZE * 2, CELL_SIZE * 2 };
	goombas[15].rect = { 351 * CELL_SIZE, WINDOW_HEIGHT - 6 * CELL_SIZE, CELL_SIZE * 2, CELL_SIZE * 2 };
	camera = { 0 , 0 , WINDOW_WIDTH, WINDOW_HEIGHT };
	tileset_texture = IMG_LoadTexture(renderer, "assets/TileMap/tiles.png");
	marioSmallRunFrames[0] = IMG_LoadTexture(renderer, "assets/Mario/Mario_Small_Run1.png");
	marioSmallRunFrames[1] = IMG_LoadTexture(renderer, "assets/Mario/Mario_Small_Run2.png");
	marioSmallRunFrames[2] = IMG_LoadTexture(renderer, "assets/Mario/Mario_Small_Run3.png");
	marioSmallJumpFrame = IMG_LoadTexture(renderer, "assets/Mario/Mario_Small_Jump.png");
	marioSmallIdleFrame = IMG_LoadTexture(renderer, "assets/Mario/Mario_Small_Idle.png");
	marioSmallSlideFrame = IMG_LoadTexture(renderer, "assets/Mario/Mario_Small_Slide.png");
	marioSmallDeathFrame = IMG_LoadTexture(renderer, "assets/Mario/Mario_Small_Death.png");
	marioSmallFlag = IMG_LoadTexture(renderer, "assets/Mario/Mario_Small_Flag.png");
	marioBigRunFrames[0] = IMG_LoadTexture(renderer, "assets/Mario/Mario_Big_Run1.png");
	marioBigRunFrames[1] = IMG_LoadTexture(renderer, "assets/Mario/Mario_Big_Run2.png");
	marioBigRunFrames[2] = IMG_LoadTexture(renderer, "assets/Mario/Mario_Big_Run3.png");
	marioBigIdleFrame = IMG_LoadTexture(renderer, "assets/Mario/Mario_Big_Idle.png");
	marioBigJumpFrame = IMG_LoadTexture(renderer, "assets/Mario/Mario_Big_Jump.png");
	marioBigSlideFrame = IMG_LoadTexture(renderer, "assets/Mario/Mario_Big_Slide.png");
	marioBigFlag = IMG_LoadTexture(renderer, "assets/Mario/Mario_Big_Flag.png");
	marioMiddleFrame = IMG_LoadTexture(renderer, "assets/Mario/Mario_Middle.png");
	goombaWalkFrames[0] = IMG_LoadTexture(renderer, "assets/Goomba/Goomba_Walk1.png");
	goombaWalkFrames[1] = IMG_LoadTexture(renderer, "assets/Goomba/Goomba_Walk2.png");
	goombaMarioKilledFrame = IMG_LoadTexture(renderer, "assets/Goomba/Goomba_Flat.png");
	goombaDeathFrame = IMG_LoadTexture(renderer, "assets/Goomba/Goomba_Death.png");
	koopaWalkFrames[0] = IMG_LoadTexture(renderer, "assets/Koopa/Koopa_Walk1.png");
	koopaWalkFrames[1] = IMG_LoadTexture(renderer, "assets/Koopa/Koopa_Walk2.png");
	koopaMarioKilledFrame = IMG_LoadTexture(renderer, "assets/Koopa/Koopa_Shell.png");
	koopaDeathFrame = IMG_LoadTexture(renderer, "assets/Koopa/Koopa_Death.png");
	magicMushroomTexture = IMG_LoadTexture(renderer, "assets/Items/MagicMushroom.png");
	upMushroomTexture = IMG_LoadTexture(renderer, "assets/Items/1upMushroom.png");
	miscTexture = IMG_LoadTexture(renderer, "assets/Misc/misc.png");
	if (!tileset_texture || !marioSmallIdleFrame || !marioSmallRunFrames || !marioSmallJumpFrame || 
		!goombaWalkFrames || !goombaDeathFrame || !goombaMarioKilledFrame ||
		!koopaWalkFrames || !koopaDeathFrame || !koopaMarioKilledFrame || !magicMushroomTexture || !miscTexture)
	{
		cerr << "SDL Init Error: " << SDL_GetError() << endl;
		running = false;
	}
	loadTilemap();
	loadCollision(false);
}

void destroyEverything()
{
	SDL_DestroyTexture(tileset_texture);
	SDL_DestroyTexture(marioSmallIdleFrame);
	for (int i = 0; i < 3; i++) {
		SDL_DestroyTexture(marioSmallRunFrames[i]);
	}
	SDL_DestroyTexture(marioSmallJumpFrame);
	SDL_DestroyTexture(marioSmallSlideFrame);
	SDL_DestroyTexture(marioSmallDeathFrame);
	for (int i = 0; i < 3; i++) {
		SDL_DestroyTexture(marioBigRunFrames[i]);
	}
	SDL_DestroyTexture(marioBigJumpFrame);
	SDL_DestroyTexture(marioBigSlideFrame);
	for (int i = 0; i < 2; i++) {
		SDL_DestroyTexture(goombaWalkFrames[i]);
	}
	SDL_DestroyTexture(goombaMarioKilledFrame);
	SDL_DestroyTexture(goombaDeathFrame);
	for (int i = 0; i < 2; i++) {
		SDL_DestroyTexture(koopaWalkFrames[i]);
	}
	SDL_DestroyTexture(koopaMarioKilledFrame);
	SDL_DestroyTexture(koopaDeathFrame);
	SDL_DestroyTexture(magicMushroomTexture);
	SDL_DestroyTexture(miscTexture);
	SDL_DestroyTexture(marioMiddleFrame);
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	SDL_Quit();
}

void handleEvents()
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_EVENT_QUIT:
		{
			running = false;
			break;
		}
		case SDL_EVENT_KEY_DOWN:
			keys[event.key.scancode] = true;
			break;
		case SDL_EVENT_KEY_UP:
			keys[event.key.scancode] = false;
			break;
		}
	}
}