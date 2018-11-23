
#include "constants.h"
#include <cstdlib>
#include <ctime>
#include <array>

#include <windows.h>






struct Player
{
	Vec2 mPos = Vec2(683,384);
	Vec2 mDirec = Vec2(0,0);
	Vec2 mVel = Vec2(0, 0);
	sf::FloatRect mPlayerBounds;
	uint16_t mScore = 0;
};


struct Asteroid
{
	Vec2 mPos;
	Vec2 mVel;
	Vec2 mScale = Vec2(1.0f,1.0f);
	sf::Texture mTex;
	float mHealth = 0.0f;
};

struct EnemySpaceShip
{
	Vec2 mPos;
	Vec2 mVel;
	Vec2 mScale = Vec2(1.0f, 1.0f);
	float mHealth = 0.0f;
};

struct Bullet
{
	Vec2 mPos;
	Vec2 mVel;
	float mRotation;
	float mLife = 0.0f;
};


struct Particle 
{
	Vec2 mPos;
	Vec2 mVel;
	Vec2 mScale = Vec2(0.3f, 1.0f);
	float mRotation;
	float mLife = 0.0f;
};


struct Resources
{
	sf::Texture mPlayerTex;
	sf::Texture mPlayerTexFlame;
	sf::Texture mAsteroidTex1;
	sf::Texture mAsteroidTex2;
	sf::Texture mAsteroidTex3;
	sf::Texture mAsteroidTex4;
	sf::Texture mUfoTex;
	sf::Texture mBulletTex;
	sf::Font mScoreFont;
};





////////////////////////////////////////////////////////////////////////////////////////////////
// GAMEPLAY PARAMETERS

static const float sAsteroidSpawnTime = 0.8f;
static const float sBulletSpawnTime = 0.25f;
static const float sEnemyBulletSpawnTime = 1.5f;
static const float sEnemySpawnTime = 15.0f;
static const float sEnemyDirecTime = 3.0f;



////////////////////////////////////////////////////////////////////////////////////////////////

struct GameState
{
	Player m_player;
	EnemySpaceShip m_Enemy;

	float mAsteroidSpawnCounter = 0.0f;
	float mEnemySpawnCounter = sEnemySpawnTime;
	float mEnemyDirectionChange = sEnemyDirecTime;
	float mBulletSpawnCounter = 0.0f;
	float mEnemyBulletSpawnCounter = 0.0f;


	std::array<Asteroid, 100> asteroids;
	std::array<Particle, MAX_ELEMENTS> particles;
	std::array<Bullet, 5> bullets;


	bool gameRunning = true;

};

void DebugLog(const char* msg, ...)
{
	char temp[4096];

	va_list ap;
	va_start(ap, msg);
	vsnprintf_s(temp, 4096 - 1, msg, ap);
	va_end(ap);

	OutputDebugStringA(temp);
}



float fRand()
{
	return ((float)std::rand() / (float)RAND_MAX);
}

float vecMag(Vec2 vec)
{
	return sqrt(pow(vec.x, 2) + pow(vec.y, 2));
}

Vec2 vecNorm(Vec2 vec)
{
	float magnitude = sqrt(pow(vec.x, 2) + pow(vec.y, 2));
	return Vec2(vec.x / magnitude, vec.y / magnitude);
}

float dotProd(Vec2 vec1, Vec2 vec2)
{
	return vec1.x*vec2.x + vec1.y*vec2.y;
}

bool TestPointToCircle(Vec2 point, Vec2 circleCenter, float circleRadius)
{
	// if the point is within the circle 
	if (vecMag(point - circleCenter) <= circleRadius)
	{
		return true;
	}

	return false;
}

// Test intersection between a line and circle
bool TestLineToCircle(Vec2 p1, Vec2 p2, Vec2 circleCenter, float circleRadius)
{
	Vec2 d = p2 - p1;
	Vec2 f = circleCenter - p1;

	// find the closest point between the line and the circle center
	Vec2 du = vecNorm(d);
	float proj = dotProd(f, du);

	Vec2 closest;

	if (proj < 0.0f)
	{
		closest = p1;
	}
	else if (proj > vecMag(d))
	{
		closest = p2;
	}
	else
	{
		Vec2 projV = du * proj;
		closest = projV + p1;
	}

	Vec2 closestDiff = circleCenter - closest;
	float closestLen = vecMag(closestDiff);

	if (closestLen > circleRadius)
	{
		return false;
	}

	return true;
}


bool inBounds(Vec2 vec, sf::RenderWindow* const window)
{
	if (vec.x > window->getSize().x + 70 || vec.y > window->getSize().y + 70 || vec.x < -70 || vec.y < -70)
		return false;

	return true;
}

Vec2 getPosInBounds(sf::RenderWindow* const window)
{
	return Vec2(rand() % window->getSize().x, rand() % window->getSize().y);

}

Vec2 getPosOutBounds(sf::RenderWindow* const window)
{
	switch (rand() % 9)
	{

	// somewhere on left
	case 1:
		return Vec2(0 - rand() % 70, window->getSize().y*fRand());

	// somewhere on right
	case 2:
		return Vec2(window->getSize().x + rand() % 70, (window->getSize().y+30)*fRand());

	// somewhere below
	case 3:
		return Vec2(window->getSize().x*fRand(), window->getSize().y + rand() % 70);

	// somewhere above
	case 4:
		return Vec2(window->getSize().x*fRand(), 0 - rand() % 70);

	// somewhere on left and below
	case 5:
		return Vec2(0 - rand() % 70, window->getSize().y + rand() % 70);

	// somewhere on left and above
	case 6:
		return Vec2(0 - rand() % 70, 0 - rand() % 70);

	// somewhere on right and below
	case 7:
		return Vec2(window->getSize().x + rand() % 70, window->getSize().y + rand() % 70);

	// somewhere on right and above
	case 8:
		return Vec2(window->getSize().x + rand() % 70, 0 - rand() % 70);

	default:
		return Vec2(0, 0);

	}

}


// returns unit vector pointing vec1 to vec2
Vec2 calculateDirec(Vec2 vec1, Vec2 vec2)
{
	float new_x = vec2.x - vec1.x;
	float new_y = vec2.y - vec2.x;

	float magnitude = sqrt(pow(new_x, 2) + pow(new_y, 2));

	return Vec2(new_x / magnitude, new_y / magnitude);
	
}




bool loadResources(Resources& r)
{
	bool success = true;
	success &= r.mPlayerTex.loadFromFile("Assets/player.png");
	success &= r.mPlayerTexFlame.loadFromFile("Assets/player_flame.png");
	success &= r.mAsteroidTex1.loadFromFile("Assets/asteroid_1.png");
	success &= r.mAsteroidTex2.loadFromFile("Assets/asteroid_2.png");
	success &= r.mAsteroidTex3.loadFromFile("Assets/asteroid_3.png");
	success &= r.mAsteroidTex4.loadFromFile("Assets/asteroid_4.png");
	success &= r.mUfoTex.loadFromFile("Assets/ufo.png");
	success &= r.mBulletTex.loadFromFile("Assets/bullet.png");
	success &= r.mScoreFont.loadFromFile("Assets/font.ttf");
	return success;
}




void updateAndRenderPPFX(sf::RenderTexture* renderTex, sf::RenderWindow* window)
{
	// convert rendertexture into a sprite
	sf::Sprite s = sf::Sprite(renderTex->getTexture());


	// draw sprite to renderwindow
	window->draw(s);



}



void updateAndRenderUI(sf::RenderTexture* renderTex, sf::RenderWindow* window, Resources* resouce, GameState* gamestate)
{

	sf::Text text;
	text.setFont(resouce->mScoreFont);
	text.setFillColor(sf::Color::White);

	// centre of window
	Vec2 windowCentre = Vec2(683, 384);// window->getView().getCenter();



	




	// if game not running
	if (!gamestate->gameRunning)
	{ 
		text.setCharacterSize(40);
		text.setString("GAME OVER\n\nHIGHSCORE: " + std::to_string(gamestate->m_player.mScore) + "\nPRESS 'R' TO RESET\n");
	

		float textWidth = text.getLocalBounds().width;
		float textHeight = text.getLocalBounds().height;

		sf::FloatRect rect = text.getLocalBounds();

		/*DebugLog("\nleft: %f\n", rect.left);
		DebugLog("top: %f\n", rect.top); 
		DebugLog("textWidth: %f\n", rect.width);
		DebugLog("textHeight: %f\n", rect.height);*/


		windowCentre = Vec2(windowCentre.x - textWidth / 2, windowCentre.y - textHeight / 2);
	

		//print "GAME OVER" + score on screen
		text.setPosition(windowCentre);
		
		
	}


	// else is running
	else
	{
		text.setPosition(10, 10);
		text.setCharacterSize(30);
		text.setString("SCORE: " + std::to_string(gamestate->m_player.mScore));
	}
	
	

	renderTex->draw(text);
}







void updateAndRenderParticles(sf::RenderTexture* renderTex, Resources* resource, GameState* gamestate, float dt)
{

	for (unsigned i = 0; i < gamestate->particles.size(); i++)
	{
		sf::Sprite particleSprite = sf::Sprite(resource->mBulletTex);
		
		if (gamestate->particles[i].mLife > 0)
		{
			particleSprite.setScale(gamestate->particles[i].mScale);
			gamestate->particles[i].mLife -= dt * 50 * fRand();		// decay particle life in relation to time, particle life is 1 second
			gamestate->particles[i].mPos = gamestate->particles[i].mPos + gamestate->particles[i].mVel*dt;	// updates each particle new location
			particleSprite.setPosition(gamestate->particles[i].mPos);	// draws particle at specified position
			renderTex->draw(particleSprite);

		}

	}


}



void updateAndRenderEnemy(sf::RenderTexture* renderTex, Resources* resource, GameState* gamestate, sf::RenderWindow* const window, float dt)
{

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// spawns new enemy, if current enemy is dead and sufficient time passed

	// spawn timer only updated when no enemies are currently alive
	if (gamestate->m_Enemy.mHealth <= 0.0)
	{
		gamestate->mEnemySpawnCounter -= dt;		// decrement spawn counter

		if (gamestate->mEnemySpawnCounter <= 0.0f)		// if spawn time has passed
		{
			// resets the enemy spawn counter
			gamestate->mEnemySpawnCounter = sEnemySpawnTime;

			// if enemy space has no health, spawn enemy
			if (gamestate->m_Enemy.mHealth <= 0.0f)
			{
				EnemySpaceShip new_enemy;
				new_enemy.mHealth = 100.0f;
				new_enemy.mPos = getPosOutBounds(window); 
				new_enemy.mVel = vecNorm(getPosInBounds(window) - new_enemy.mPos) * ((100.0f * fRand()) + 90.0f);

				gamestate->m_Enemy = new_enemy;
			}
		}
	}


	



	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// updates enemy attributes and draws on screen if alive


	sf::Sprite enemySprite = sf::Sprite(resource->mUfoTex);
	EnemySpaceShip* enemy_current = &gamestate->m_Enemy;


	

	// if not in window and enemy has life, kill asteroid
	if (!inBounds(enemy_current->mPos, window) && (enemy_current->mHealth > 0.0f))
	{
		enemy_current->mHealth = 0.0f;
	}


	// else if asteroid has health, update location and draw on screen 
	else if (gamestate->m_Enemy.mHealth > 0.0f)
	{
		
		if ((gamestate->mEnemyDirectionChange -= dt) < 0.0f)	// enemy changes direction after short duration
		{
			gamestate->mEnemyDirectionChange = (sEnemyDirecTime * fRand()) + 2.0f;		// updates new duration time			
			enemy_current->mVel = vecNorm(getPosInBounds(window) - enemy_current->mPos) * ((100.0f * fRand()) + 100.0f);	// updates enemy new velocity
		}


		// updates current location
		enemy_current->mPos += enemy_current->mVel * dt;		
		enemySprite.setOrigin((Vec2)resource->mUfoTex.getSize() * 0.5f);
		enemySprite.setPosition(enemy_current->mPos);
		enemySprite.scale(enemy_current->mScale);


		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// check for bullet collision with enemy

		sf::FloatRect enemyBounds = enemySprite.getGlobalBounds();		// draw bounding box around enemy 


		for (unsigned i = 0; i < gamestate->bullets.size(); i++)
		{
			if (enemyBounds.contains(gamestate->bullets[i].mPos) && (gamestate->bullets[i].mLife > 0.0f))
			{
				gamestate->bullets[i].mLife = 0.0f;
				enemy_current->mHealth -= 50.0f;
				gamestate->m_player.mScore += 50;


				if (enemy_current->mHealth <= 0.0f)
				{
					//spawns 25 particles, asteroid explosion
					for (unsigned i = 0; i < 32; i++)
					{
						for (unsigned i = 0; i < gamestate->particles.size(); i++)
						{
							if (gamestate->particles[i].mLife <= 0.0f)
							{
								Particle new_particle;
								new_particle.mLife = 100.0f;
								new_particle.mPos = enemy_current->mPos;
								new_particle.mVel = calculateDirec(new_particle.mPos, getPosInBounds(window)) * fRand() * 250.0f;
								gamestate->particles[i] = new_particle;
								break;
							}

						}
					}
				}



				break;
			}
		}




		if (enemy_current->mHealth > 0.0f)
		{
			///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// updates new enemy bullets fired

			gamestate->mEnemyBulletSpawnCounter += dt;

			if (gamestate->mEnemyBulletSpawnCounter > sEnemyBulletSpawnTime)
			{
				gamestate->mEnemyBulletSpawnCounter = 0.0f;		// resets time elapsed since bullet fired

				Vec2 enemyToPlayer = gamestate->m_player.mPos - gamestate->m_Enemy.mPos;	// calculate vector between enemy and player
				float offset = fRand() * 8.0f;
				enemyToPlayer = vecNorm(Vec2(enemyToPlayer.x + offset, enemyToPlayer.y + offset));	// gives random accuracy to vector



				for (unsigned i = 0; i < gamestate->bullets.size(); i++)	// loops through the bullet array and finds next free space 
				{

					if (gamestate->bullets[i].mLife <= 0)
					{
						Bullet new_bullet;		// spawns a new bullet
						new_bullet.mPos = enemy_current->mPos + (enemyToPlayer * enemySprite.getGlobalBounds().width / 1.5f);			// updates position to just outside enemy
						new_bullet.mRotation = atan2f(enemyToPlayer.y, enemyToPlayer.x) * (float)(180 / 3.14159);
						new_bullet.mVel = enemyToPlayer * 250.0f;
						new_bullet.mLife = 175.0f;
						gamestate->bullets[i] = new_bullet;
						break;
					}
				}
			}


			///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// draws enemy sprite
			renderTex->draw(enemySprite);
		}


	}



	// else if no life, do nothing
	else
	{

	}











}





void updateAndRenderAsteroids(sf::RenderWindow* const window, sf::RenderTexture* renderTex, Resources* resource, GameState* gamestate, float dt)
{


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// loops through array and stores asteroid in next empty space

	// every 3 seconds spawn an asteroid, with random velocity and position
	if ((gamestate->mAsteroidSpawnCounter -= dt) < 0.0f)
	{
		// resets the spawn counter
		gamestate->mAsteroidSpawnCounter = sAsteroidSpawnTime;

		for (unsigned i = 0; i < gamestate->asteroids.size(); i++)
		{
			// if asteroid has no health, empty space
			if(gamestate->asteroids[i].mHealth <= 0)
			{ 
				Asteroid new_asteroid;
				new_asteroid.mHealth = 100.0f;
				new_asteroid.mPos = getPosOutBounds(window);
				new_asteroid.mVel = calculateDirec(new_asteroid.mPos, getPosInBounds(window)) * ((250.0f * fRand()) + 50);


				// selects random asteroid texture
				switch (rand() % 4)
				{
				case 0:
					new_asteroid.mTex = resource->mAsteroidTex1;
					break;

				case 1:
					new_asteroid.mTex = resource->mAsteroidTex2;
					break;

				case 2:
					new_asteroid.mTex = resource->mAsteroidTex3;
					break;

				case 3:
					new_asteroid.mTex = resource->mAsteroidTex4;
					break;

				}	


				float scale_fac = fRand() + (float)0.7;

				new_asteroid.mScale = Vec2(scale_fac, scale_fac);


				gamestate->asteroids[i] = new_asteroid;
				break;
			}
		}

	}


	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// updates the location of each asteroid, if outside of window then health is set to 0 

	for (unsigned i = 0; i < gamestate->asteroids.size(); i++)
	{


		Asteroid* asteroid_current = &gamestate->asteroids[i];
		sf::Sprite asteroidSprite = sf::Sprite(asteroid_current->mTex);

		
		asteroid_current->mPos = asteroid_current->mPos + asteroid_current->mVel*dt;	// updates asteroid new location


		// if not in window and asteroid has life, kill asteroid
		if (!inBounds(asteroid_current->mPos, window) && (asteroid_current->mHealth > 0.0f))
		{
			asteroid_current->mHealth = 0.0f;
		}

		// else if asteroid has health
		else if ((asteroid_current->mHealth > 0.0f))
		{

			bool collision = false;		// collision flag
			float radius = asteroidSprite.getGlobalBounds().width / 2.0f;	// radius of current asteroid

			// checks each bullet for collision with current asteroid
			for (unsigned i = 0; i < gamestate->bullets.size(); i++)
			{
				if (gamestate->bullets[i].mLife > 0)
				{
					
					if (TestPointToCircle(gamestate->bullets[i].mPos, asteroid_current->mPos, radius))
					{
						collision = true;
						gamestate->bullets[i].mLife = 0.0f;
						asteroid_current->mHealth = 0.0f;
						gamestate->m_player.mScore += 10;
						
						//spawns 25 particles, asteroid explosion
						for (unsigned i = 0; i < 32; i++)
						{
							for (unsigned i = 0; i < gamestate->particles.size(); i++)
							{
								if (gamestate->particles[i].mLife <= 0.0f)
								{
									Particle new_particle;
									new_particle.mLife = 100.0f;
									new_particle.mPos = asteroid_current->mPos;
									new_particle.mVel = calculateDirec(new_particle.mPos, getPosInBounds(window)) * fRand() * 250.0f;
									gamestate->particles[i] = new_particle;
									break;
								}

							}
						}


					}
				}

			}




			// p1, gets middle right point of player ship
			Vec2 playerP1 = Vec2(gamestate->m_player.mPos.x + 0.5*gamestate->m_player.mPlayerBounds.width, gamestate->m_player.mPos.y);

			// p2, gets top left point of player ship
			Vec2 playerP2 = Vec2(gamestate->m_player.mPos.x - 0.5*gamestate->m_player.mPlayerBounds.width, gamestate->m_player.mPos.y - 0.5*gamestate->m_player.mPlayerBounds.height);

			// p3, gets bottom left point of player ship
			Vec2 playerP3 = Vec2(gamestate->m_player.mPos.x - 0.5*gamestate->m_player.mPlayerBounds.width, gamestate->m_player.mPos.y + 0.5*gamestate->m_player.mPlayerBounds.height);


			// checks for collision of 

			if (TestLineToCircle(playerP2, playerP1, asteroid_current->mPos, radius) || TestLineToCircle(playerP3, playerP1, asteroid_current->mPos, radius))
			{
				gamestate->gameRunning = false;
			}




			if (!collision)
			{
				// draws asteroid at specified location
				asteroidSprite.setOrigin((Vec2)asteroid_current->mTex.getSize() * 0.5f);
				asteroidSprite.setPosition(asteroid_current->mPos);
				asteroidSprite.scale(asteroid_current->mScale);
				renderTex->draw(asteroidSprite);
			}


		}


		// else, do nothing
		else
		{

		}



	}

		

	




}



void updateAndRenderBullets(sf::RenderTexture* renderTex, Resources* resource, GameState* gamestate, float dt)
{

	for (unsigned i = 0; i < gamestate->bullets.size(); i++)
	{

		// converts the bullet texture into a sprite
		sf::Sprite bulletSprite = sf::Sprite(resource->mBulletTex);
		bulletSprite.setOrigin((Vec2)resource->mBulletTex.getSize() * 0.5f);

		if (gamestate->bullets[i].mLife > 0)
		{
			
			bulletSprite.rotate(gamestate->bullets[i].mRotation);	// rotate the player in relation to mouse cursor
			gamestate->bullets[i].mLife -= dt * 100;		// decay bullet life in relation to time, bullet life is 1 second
			gamestate->bullets[i].mPos = gamestate->bullets[i].mPos + gamestate->bullets[i].mVel*dt;	// updates each bullet new location
			bulletSprite.setPosition(gamestate->bullets[i].mPos);	// draws bullet at specified position
			
			renderTex->draw(bulletSprite);	

		}

		else
		{
			gamestate->bullets[i].mLife = 0.0f;
		}

	}
	
}



void updateAndRenderPlayer(sf::RenderTexture* renderTex, Resources* resource, GameState* gamestate, sf::RenderWindow* window, float dt)
{
	// if game not running
	if (!gamestate->gameRunning)
	{
		// if "r" is pressed, reset game
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::R))
		{
			// resets player
			gamestate->m_player.mPos = Vec2(683, 384);
			gamestate->m_player.mVel = Vec2(0, 0);
			gamestate->m_player.mDirec = Vec2(0, 0);
			gamestate->m_player.mScore = 0;

			// current asteroids are destroyed
			for (unsigned i = 0; i < gamestate->asteroids.size(); i++)
			{
				gamestate->asteroids[i].mHealth = 0;
			}

			// resets enemy spawn time
			gamestate->mEnemySpawnCounter = sEnemySpawnTime;

			gamestate->gameRunning = true;
		}


		return;
	}
		



	sf::Sprite playerSprite;	// sprite representing player


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// checks and stores user input

	Vec2 playerToMouse = vecNorm((Vec2)sf::Mouse::getPosition(*window) - gamestate->m_player.mPos);	// gets the direction from player to mouse
	bool keyPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Space);	// gets the key press state
	bool leftMousePressed = sf::Mouse::isButtonPressed(sf::Mouse::Left);	// gets left mouse press state
	

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// updates the location, orientation, player bounds and player texture

	// load player flame texture if "spacebar" pressed
	if (keyPressed)
	{
		playerSprite = sf::Sprite(resource->mPlayerTexFlame); 
		playerSprite.setOrigin((Vec2)resource->mPlayerTexFlame.getSize() * 0.5f);	// defines the center point for all transformations, relative to top left corner of sprite 
		gamestate->m_player.mVel += playerToMouse * 5.0f;	// increases velocity linearly
	}

	// otherwise load normal player texture 
	else
	{
		playerSprite = sf::Sprite(resource->mPlayerTex); 
		playerSprite.setOrigin((Vec2)resource->mPlayerTex.getSize() * 0.5f);	// defines the center point for all transformations, relative to top left corner of sprite 
		gamestate->m_player.mVel = gamestate->m_player.mVel*0.99f;	// velocity decays exponentially
	}

	gamestate->m_player.mPos = gamestate->m_player.mPos + gamestate->m_player.mVel * dt;	// new position is combination of previous position + velocity * change in time
	playerSprite.setPosition(gamestate->m_player.mPos);										// updates new position
	playerSprite.rotate(atan2f(playerToMouse.y, playerToMouse.x) * (float)(180 / 3.14159));	// rotate the player in relation to mouse cursor
	gamestate->m_player.mPlayerBounds = playerSprite.getGlobalBounds();		// updates the players current bounds

	/*DebugLog("\nPlayerleft: %f\n", gamestate->m_player.mPlayerBounds.left);
	DebugLog("Playertop: %f\n", gamestate->m_player.mPlayerBounds.top);
	DebugLog("PlayerWidth: %f\n", gamestate->m_player.mPlayerBounds.width);
	DebugLog("PlayerHeight: %f\n", gamestate->m_player.mPlayerBounds.height);*/

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// updates bullet fired
	
	gamestate->mBulletSpawnCounter += dt;//0.016f;		// updates time since last bullet spawn

	if (leftMousePressed && gamestate->mBulletSpawnCounter > sBulletSpawnTime)
	{
			gamestate->mBulletSpawnCounter = 0.0f; // resets the spawn counter

			// loops through the bullet array and finds next free space 
			for (unsigned i = 0; i < gamestate->bullets.size(); i++)
			{
				// if bullet has no life, empty space
				if (gamestate->bullets[i].mLife <= 0)
				{
					Bullet new_bullet;		// spawns a new bullet


					new_bullet.mPos = gamestate->m_player.mPos + playerToMouse * (playerSprite.getLocalBounds().width*0.5f);	// updates position to just in front of space craft
					new_bullet.mRotation = atan2f(playerToMouse.y, playerToMouse.x) * (float)(180 / 3.14159);
					new_bullet.mVel = playerToMouse * 500.0f;
					new_bullet.mLife = 100.0f;
					gamestate->bullets[i] = new_bullet;
					break;
				}
			}

		

	}
	

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// checks for bullet collision with player



	// p1, gets middle right point of player ship
	Vec2 playerP1 = Vec2(gamestate->m_player.mPos.x + 0.5*gamestate->m_player.mPlayerBounds.width, gamestate->m_player.mPos.y);

	// p2, gets top left point of player ship
	Vec2 playerP2 = Vec2(gamestate->m_player.mPos.x - 0.5*gamestate->m_player.mPlayerBounds.width, gamestate->m_player.mPos.y - 0.5*gamestate->m_player.mPlayerBounds.height);

	// p3, gets bottom left point of player ship
	Vec2 playerP3 = Vec2(gamestate->m_player.mPos.x - 0.5*gamestate->m_player.mPlayerBounds.width, gamestate->m_player.mPos.y + 0.5*gamestate->m_player.mPlayerBounds.height);



	for (unsigned i = 0; i < gamestate->bullets.size(); i++)
	{


		if (gamestate->bullets[i].mLife > 0.0f)		// check if bullet live
		{

			if (TestLineToCircle(playerP2, playerP1, gamestate->bullets[i].mPos, 0) || TestLineToCircle(playerP3, playerP1, gamestate->bullets[i].mPos, 0))
			{
				gamestate->gameRunning = false;
			}
		}

	}


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// draws player in window

	renderTex->draw(playerSprite);
	renderTex->display();


}





void updateAndRenderLevel(sf::RenderTexture* renderTex)
{
	renderTex->clear(sf::Color::Black);

	// updates rendertexture with new assets
	renderTex->display();
}



void updateAndRender(sf::RenderTexture* renderTex, sf::RenderWindow* window, Resources* resource, GameState* gamestate, float time)
{
	updateAndRenderLevel(renderTex);
	updateAndRenderPlayer(renderTex, resource, gamestate, window, time);
	updateAndRenderEnemy(renderTex, resource, gamestate, window, time);
	updateAndRenderBullets(renderTex, resource, gamestate, time);
	updateAndRenderAsteroids(window, renderTex, resource, gamestate, time);
	updateAndRenderParticles(renderTex, resource, gamestate, time);
	updateAndRenderUI(renderTex, window, resource, gamestate);
	updateAndRenderPPFX(renderTex, window);
}







int main()
{
	// canvas that all assets are drawn to 
	sf::RenderTexture renderTex;

	sf::Clock clock;
	GameState gamestate;
	Resources resource;
	std::srand(static_cast<unsigned int>(std::time(0)));

	if (!loadResources(resource))
	{
		exit(1);
	}


	// create the window
	sf::RenderWindow window(sf::VideoMode(1366, 768), "Space Wars");


	window.setVerticalSyncEnabled(true);

	// updates the size of the rendertexture
	renderTex.create(window.getSize().x, window.getSize().y);




	while (window.isOpen())
	{

		sf::Event event;
		while (window.pollEvent(event))
		{

			if (event.type == sf::Event::Closed)
				window.close();
		}

		sf::Time frameTime = clock.getElapsedTime();

		clock.restart();


		window.clear(sf::Color::Black);

		updateAndRender(&renderTex, &window, &resource, &gamestate, frameTime.asSeconds());

		
		window.display();
	}

	return 0;
}
