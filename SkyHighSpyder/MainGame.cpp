#define PLAY_IMPLEMENTATION
#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Play.h"

int DISPLAY_WIDTH  = 1280;
int DISPLAY_HEIGHT = 720;
int DISPLAY_SCALE = 1;

const Vector2D METEOR_VELOCITY_DEFAULT{ 1.0f, 0.f }; 
const Vector2D AGENT8_VELOCITY_DEFAULT{ 2.f, 0.f };

const float AGENT8_SPEED{ 3.0f };
const float SPECIAL_ASTEROID_SPEED{ 1.0f };
const float ASTEROID_SPEED{ 2.0f };
const float METEOR_SPEED{ 1.0f };
const float GEM_SPEED{ 0.5f };
const float ASTEROID_PIECE_SPEED{ 15.f };
const float ROCKET_SPEED{ 6.f };

const float RESISTANCE{ 0.7f };

const Vector2D ASTEROID_PIECE_VELOCITY_1{ 0.f, -ASTEROID_PIECE_SPEED };
const Vector2D ASTEROID_PIECE_VELOCITY_2{ 3.f, ASTEROID_PIECE_SPEED };
const Vector2D ASTEROID_PIECE_VELOCITY_3{ -ASTEROID_PIECE_SPEED, 0.f };

const Vector2D LASER_VELOCITY_DEFAULT{ 0.f, 0.f };
const Vector2D LASER_ACCELERATION{ 0.f, 0.f };

int MAX_GEMS{ 2 };
float timePassed = 0.f;
float particleOpacity{ 1.f };
const int MAX_ASTEROIDS{ 3 };
const int SHIELD_DURATION{ 300 };

const Vector2D METEOR_AABB{ 50.f, 60.f };
const Vector2D ASTEROID_AABB{ 50.f, 50.f };
const Vector2D PIECES_AABB{ 20.f, 20.f };
const Vector2D AGENT8_AABB{ 30.f, 30.f };
const Vector2D PARTICLE_AABB{ 20.f, 20.f };
const Vector2D GEM_AABB{ 20.f, 20.f };
const Vector2D LASER_AABB{ 5.f, 20.f };
const Vector2D ROCKET_AABB{ 10.f, 30.f };

float scale{ 1.0f };

enum Agent8State
{
	STATE_ATTACHED = 0,
	STATE_FLY,
	STATE_DEAD,
	STATE_WON,
	STATE_ROCKET,
};

enum GameFlow
{
	STATE_LOBBY = 0,
	STATE_GAME_PLAY,
	STATE_GAME_PAUSED,
	STATE_GAME_OVER,
	STATE_VICTORY,
};

struct GameState
{
	int score{ 0 };
	int level{ 1 };
	int highScore{ 0 };
	int gems{ MAX_GEMS };
	int gemsCollected{ 0 };
	bool playMode{ true };
	bool paused{ false };
	bool sound{ false };
	bool music{ false };
	bool powerActivated{ false };
	int collisionCount{ 0 };
	int meteorCollisions{ 0 };
	int asteroidCollisions{ 0 };
	bool asteroidDestroyed{ false };
	bool levelPassed{ false };
	bool levelInfo{ true };
	bool isAnimation{ false };

	Agent8State agent8State = STATE_ATTACHED;
	GameFlow state = STATE_LOBBY;
};

GameState gameState;

enum gameObjectType
{
	TYPE_NULL = -1,
	TYPE_AGENT8,
	TYPE_ROCKET,
	TYPE_ASTEROID,
	TYPE_METEOR,
	TYPE_GEM,
	TYPE_DIAMOND,
	TYPE_PARTICLE,
	TYPE_PIECES,
	TYPE_RING,
	TYPE_LASER,
	TYPE_SPECIAL,
	TYPE_SHIELD,
	TYPE_DESTROYED,
};

void HandlePlayerControls();
void DestroyAllItems();
void ResetGame();
void AttachedPlayerControls();
void RocketPlayerControls();

void UpdateAgent8();
void UpdateRocket();
void UpdateLasers();
void UpdateMeteors();
void UpdateAsteroids();
void UpdateAsteroidPieces();
void UpdateGemsAndDiamonds();
void UpdateParticles();
void UpdateRings();
void UpdateDestroyed();

void CreateGameObjects();
void CreateParticles();
void CreateRocketParticles();
void LoopObject(GameObject& object);
bool IsColliding(const GameObject& object);
bool IsCollidingLaser(const GameObject& laser, const GameObject& object);
void AsteroidCollision();
void AsteroidExplosion(GameObject& asteroid);

void SoundControl();
float Randomize(int range, float multiplier);
void FloatDirectionObject(GameObject& obj, float speed);
bool DisplayAreaTest(GameObject& object, Vector2D AABB);

void DrawGameStats();
void DrawGamePlay();
void DrawLobby();
void DrawSoundControl();

// The entry point for a PlayBuffer program
void MainGameEntry( PLAY_IGNORE_COMMAND_LINE )
{
	Play::CreateManager( DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_SCALE );
	Play::CentreAllSpriteOrigins();
	Play::LoadBackground("Data//Backgrounds//background.png");
	
}

// Called by PlayBuffer every frame (60 times a second!)
bool MainGameUpdate( float elapsedTime )
{
	
	timePassed += elapsedTime;
	if (timePassed > 5.f)
	{
		timePassed = 0.f;
		gameState.levelInfo = false;
	}

	Play::DrawBackground();

	switch (gameState.state)
	{
		case STATE_LOBBY:
		{
			DrawLobby();
			if (Play::KeyPressed(VK_RETURN))
			{
				CreateGameObjects();
				gameState.state = STATE_GAME_PLAY;
			}
			break;
		}
		case STATE_GAME_PLAY:
		{			
			if (Play::KeyPressed(VK_SHIFT))
			{
				gameState.paused = true;
				gameState.state = STATE_GAME_PAUSED;
			}
			UpdateAgent8();
			UpdateLasers();
			UpdateAsteroids();
			UpdateMeteors();
			UpdateRocket();
			UpdateParticles();
			UpdateGemsAndDiamonds();
			UpdateRings();
			UpdateAsteroidPieces();
			DrawGamePlay();
			break;
		}
		case STATE_GAME_PAUSED:
		{
			DrawLobby();
			if (Play::KeyPressed(VK_RETURN))
			{
				gameState.paused = false;
				gameState.state = STATE_GAME_PLAY;
			}
			break;
		}
	}

	UpdateDestroyed();
	SoundControl();

	if (gameState.levelPassed && !gameState.isAnimation)
	{
		gameState.level++;
		MAX_GEMS++;
		ResetGame();
		CreateGameObjects();
		DrawGamePlay();
		gameState.state = STATE_GAME_PLAY;
		gameState.levelPassed = false;
	}

	if (Play::KeyPressed(VK_TAB))
	{
		gameState.level = { 1 };
		gameState.score = { 0 };
		MAX_GEMS = { 2 };
		ResetGame();
	}
	
	return Play::KeyDown( VK_ESCAPE );
}

// Gets called once when the player quits the game 
int MainGameExit(void)
{
	Play::DestroyManager();
	return PLAY_OK;
}

void SoundControl()
{
	if (Play::KeyPressed(VK_F2))
		gameState.sound = !gameState.sound;

	if (Play::KeyPressed(VK_F3))
	{
		gameState.music = !gameState.music;

		(gameState.music) ? Play::StartAudioLoop("music") : Play::StopAudioLoop("music");
	}
}

void CreateGameObjects()
{
	int idAgent8 = Play::CreateGameObject(TYPE_AGENT8, { 115, 0 }, 50, "agent8_left");
	Play::MoveSpriteOrigin("agent8_left", 0, 55);
	Play::MoveSpriteOrigin("agent8_right", 0, 55);

	GameObject& objAgent8 = Play::GetGameObject(idAgent8);

	int objSpecialAsteroidID = Play::CreateGameObject(TYPE_SPECIAL, { DISPLAY_WIDTH / 2, 400 }, 50, "asteroid");
	GameObject& objSpecialAsteroid = Play::GetGameObject(objSpecialAsteroidID);
	Play::MoveSpriteOrigin("asteroid", 0, -20);
	objSpecialAsteroid.rotation = Randomize(630, 0.01);
	objSpecialAsteroid.animSpeed = 2.5f;

	objAgent8.rotSpeed = objSpecialAsteroid.rotSpeed;

	for (int i = 0; i < MAX_ASTEROIDS; i++)
	{
		int id = Play::CreateGameObject(TYPE_ASTEROID, { Play::RandomRoll(DISPLAY_WIDTH), Play::RandomRoll(DISPLAY_WIDTH) }, 50, "asteroid");
		GameObject& objAsteroid = Play::GetGameObject(id);
		objAsteroid.rotation = Randomize(630, 0.01);
		objAsteroid.animSpeed = 2.5f;
	}

	int id = Play::CreateGameObject(TYPE_METEOR, { Play::RandomRoll(DISPLAY_WIDTH), Play::RandomRoll(DISPLAY_WIDTH) }, 50, "meteor");
	GameObject& objMeteor = Play::GetGameObject(id);
	Play::SetSprite(objMeteor, "meteor", 0.25f);
	objMeteor.rotation = Play::DegToRad(70);

	if (gameState.level > 1)
	{
		id = Play::CreateGameObject(TYPE_ROCKET, { Play::RandomRoll(DISPLAY_WIDTH), Play::RandomRoll(DISPLAY_HEIGHT) }, 30, "rocket");
		GameObject& objRocket = Play::GetGameObject(id);
		objRocket.rotation = Play::DegToRad(90);
	}
}

void DrawSoundControl()
{
	Play::DrawFontText("64px", (gameState.sound) ? "SOUND: ON" : "SOUND: OFF", { 100, 50 }, Play::CENTRE);
	Play::DrawFontText("64px", (gameState.music) ? "MUSIC: ON" : "MUSIC: OFF", { 100, 100 }, Play::CENTRE);
}

void DrawGamePlay()
{
	GameObject& objAgent8 = Play::GetGameObjectByType(TYPE_AGENT8);
	GameObject& objSpecialAsteroid = Play::GetGameObjectByType(TYPE_SPECIAL);
	
	std::vector<int> vMeteors = Play::CollectGameObjectIDsByType(TYPE_METEOR);

	for (int idMeteor : vMeteors)
	{
		GameObject& objMeteor = Play::GetGameObject(idMeteor);

		Play::DrawObjectRotated(objMeteor);
	}

	std::vector<int> vAsteroids = Play::CollectGameObjectIDsByType(TYPE_ASTEROID);
	for (int idAsteroid : vAsteroids)
	{
		GameObject& objAsteroid = Play::GetGameObject(idAsteroid);

		Play::DrawObjectRotated(objAsteroid);
	}

	std::vector<int> vPieces = Play::CollectGameObjectIDsByType(TYPE_PIECES);
	for (int idPiece : vPieces)
	{
		GameObject& objPiece = Play::GetGameObject(idPiece);
		Play::DrawObject(objPiece);
	}

	std::vector<int> vParticles = Play::CollectGameObjectIDsByType(TYPE_PARTICLE);
	for (int idParticle : vParticles)
	{
		
		GameObject& objParticle = Play::GetGameObject(idParticle);

		if (objParticle.frame >= 10)
			particleOpacity = 0.5f;

		Play::DrawObjectRotated(objParticle, particleOpacity);
	}

	std::vector<int> vGems = Play::CollectGameObjectIDsByType(TYPE_GEM);
	for (int idGem : vGems)
	{
		GameObject& objGem = Play::GetGameObject(idGem);
		Play::DrawObjectRotated(objGem);
	}

	std::vector<int> vRings = Play::CollectGameObjectIDsByType(TYPE_RING);
	for (int idRing : vRings)
	{
		scale += 0.1f;
		GameObject& objRing = Play::GetGameObject(idRing);

		Play::DrawSpriteRotated("ring", objRing.pos, 1,  objRing.rotation, scale + 1.5f, .05f);
		Play::DrawSpriteRotated("ring", objRing.pos, 1,  objRing.rotation, scale + 1.25f, .1f);
		Play::DrawSpriteRotated("ring", objRing.pos, 2,  objRing.rotation, scale + 1.0f, .2f);
		Play::DrawSpriteRotated("ring", objRing.pos, 3,  objRing.rotation, scale + 0.75f, .3f);
	}

	std::vector<int> vLasers = Play::CollectGameObjectIDsByType(TYPE_LASER);
	for (int idLaser : vLasers)
	{
		GameObject& objLaser = Play::GetGameObject(idLaser);
		Play::DrawObjectRotated(objLaser);
	}

	std::vector<int> vDiamonds = Play::CollectGameObjectIDsByType(TYPE_DIAMOND);
	for (int idDiamond : vDiamonds)
	{
		GameObject& objDiamond = Play::GetGameObject(idDiamond);
		Play::DrawObjectRotated(objDiamond);
		objDiamond.scale = 0.5f;
	}

	std::vector<int> vShields = Play::CollectGameObjectIDsByType(TYPE_SHIELD);
	for (int idShield : vShields)
	{
		GameObject& objShield = Play::GetGameObject(idShield);

		if (gameState.playMode)
		{
			Play::DrawSpriteRotated("ring", objShield.pos, 1, objShield.rotation, 2.8f, .1f);
			Play::DrawSpriteRotated("ring", objShield.pos, 1, objShield.rotation, 2.5f, .4f);
		}
	}

	Play::DrawObjectRotated(objSpecialAsteroid);
	
	std::vector<int> vRockets = Play::CollectGameObjectIDsByType(TYPE_ROCKET);
	for (int idRocket : vRockets)
	{
		GameObject& objRocket = Play::GetGameObject(idRocket);
		Play::DrawObjectRotated(objRocket);
	}
		
	Play::DrawObjectRotated(objAgent8);

	DrawGameStats();
	DrawSoundControl();

	if (!gameState.playMode)
	{
		Play::DrawFontText("151px", "GAME OVER", { DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2 }, Play::CENTRE);
		Play::DrawFontText("64px", (gameState.agent8State == STATE_DEAD) ? "" : "YOU DESTROYED TOO MANY ASTEROIDS AND LOST GEMS", { DISPLAY_WIDTH / 2, (DISPLAY_HEIGHT / 2) + 100 }, Play::CENTRE);
		Play::DrawFontText("64px", "PRESS TAB TO RESTART", {DISPLAY_WIDTH / 2, DISPLAY_HEIGHT - 100}, Play::CENTRE);
	}

	Play::PresentDrawingBuffer();
}

void DrawLobby()
{
	Play::DrawFontText("64px", (gameState.paused) ? "PAUSED" : "WELCOME TO SKY HIGH SPYDER !", { DISPLAY_WIDTH / 2, 300 }, Play::CENTRE);
	Play::DrawFontText("64px", (gameState.paused) ? "PRESS ENTER TO CONTINUE" : "PRESS ENTER TO START || SHIFT TO PAUSE" , { DISPLAY_WIDTH / 2, DISPLAY_HEIGHT - 300 }, Play::CENTRE);
	Play::DrawFontText("64px", (gameState.paused) ? "" : "ARROW KEYS TO MOVE LEFT / RIGHT || DOWN FOR LASERS || SPACE TO FLY", { DISPLAY_WIDTH / 2, DISPLAY_HEIGHT - 200 }, Play::CENTRE);
	Play::DrawFontText("64px", "F2 FOR SOUND || F3 FOR MUSIC", { DISPLAY_WIDTH / 2, DISPLAY_HEIGHT - 100 }, Play::CENTRE);

	Play::PresentDrawingBuffer();
}

void DrawGameStats()
{	
	Play::DrawFontText("64px", "SCORE: " + std::to_string(gameState.score), { DISPLAY_WIDTH - 200, 50 }, Play::CENTRE);
	//Play::DrawFontText("64px", "HIGHSCORE: " + std::to_string(gameState.highScore), { DISPLAY_WIDTH - 200, 100 }, Play::CENTRE);
	//Play::DrawFontText("64px", "Spy state: " + std::to_string(gameState.agent8State), { DISPLAY_WIDTH - 400, 100 }, Play::CENTRE);
	//Play::DrawFontText("64px", "Asteroid rotation: " + std::to_string(objSpecialAsteroid.rotation), { DISPLAY_WIDTH - 400, 200 }, Play::CENTRE);
	Play::DrawFontText("105px", (gameState.powerActivated) ? "SHIELD ACTIVATED" : "", {DISPLAY_WIDTH / 2 , 150}, Play::CENTRE);
	Play::DrawFontText("132px", "Level: " + std::to_string(gameState.level), { DISPLAY_WIDTH / 2, 50 }, Play::CENTRE);
	Play::DrawFontText("132px", (gameState.levelInfo) ?  "LEVEL " + std::to_string(gameState.level) : "", {DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2}, Play::CENTRE);
	//Play::DrawFontText("64px", "Collisions: " + std::to_string(gameState.asteroidCollisions), { 300, 100 }, Play::CENTRE);
	Play::DrawFontText("64px", "Gems: " + std::to_string(gameState.gemsCollected) + " of " + std::to_string(MAX_GEMS), {350, 50}, Play::CENTRE);
}

void FloatDirectionObject(GameObject& obj, float speed)
{
	float x = sin(obj.rotation);
	float y = cos(obj.rotation);

	obj.velocity.x = x * speed;
	obj.velocity.y = -y * speed;
}

float Randomize(int range, float multiplier = 1.0f)
{
	return (float)(rand() % range) * multiplier;
}

void AttachedPlayerControls() 
{
	GameObject& objAgent8 = Play::GetGameObjectByType(TYPE_AGENT8);
	GameObject& objSpecialAsteroid = Play::GetGameObjectByType(TYPE_SPECIAL);

	if (Play::KeyDown(VK_LEFT))
	{
		objAgent8.rotation -= 0.1f;
		Play::SetSprite(objAgent8, "agent8_left", 0);
		objAgent8.animSpeed = 0.5f;
	}
	else if (Play::KeyDown(VK_RIGHT))
	{
		objAgent8.rotation += 0.1f;
		Play::SetSprite(objAgent8, "agent8_right", 0);
		objAgent8.animSpeed = 0.5f;
	}
	else if (Play::KeyDown(VK_DOWN))
	{
		if (gameState.sound)
			Play::PlayAudio("laser");

		int id = Play::CreateGameObject(TYPE_LASER, objAgent8.pos, 50, "laser");
		GameObject& objLaser = Play::GetGameObject(id);
		objLaser.rotation = objAgent8.rotation;
	}
	else if (Play::KeyPressed(VK_SPACE))
	{
		if (gameState.sound)
			Play::PlayAudio("explode");

		AsteroidExplosion(objSpecialAsteroid);
		objAgent8.pos.x += sin(objAgent8.rotation) * 100;
		objAgent8.pos.y -= cos(objAgent8.rotation) * 100;
		gameState.agent8State = STATE_FLY;
	}
	else
	{
		Play::SetSprite(objAgent8, "agent8_left", 0);
		objAgent8.animSpeed = 0.f;
	}

	Play::UpdateGameObject(objAgent8);
}

void RocketPlayerControls()
{
	GameObject& objAgent8 = Play::GetGameObjectByType(TYPE_AGENT8);
	GameObject& objRocket = Play::GetGameObjectByType(TYPE_ROCKET);

	if (Play::KeyDown(VK_LEFT))
		objRocket.rotation -= 0.05f;

	else if (Play::KeyDown(VK_RIGHT))
		objRocket.rotation += 0.05f;

	else if (Play::KeyPressed(VK_SPACE))
	{
		objAgent8.pos.x += sin(objAgent8.rotation) * 100;
		objAgent8.pos.y -= cos(objAgent8.rotation) * 100;
		objAgent8.velocity = { 0.f, -5.0f };
		Play::MoveSpriteOrigin("agent8_right", 0, 35);
		Play::StopAudioLoop("fan");
		gameState.agent8State = STATE_FLY;
	} 

	Play::UpdateGameObject(objAgent8);
}

void HandlePlayerControls()
{
	GameObject& objAgent8 = Play::GetGameObjectByType(TYPE_AGENT8);
	GameObject& objRocket = Play::GetGameObjectByType(TYPE_ROCKET);
	

	if (Play::KeyDown(VK_LEFT))
		objAgent8.rotation -= 0.05f; 

	else if (Play::KeyDown(VK_RIGHT))
		objAgent8.rotation += 0.05f;		
	
	else if (Play::KeyDown(VK_DOWN))
	{
		if (gameState.sound)
			Play::PlayAudio("laser");

		int id = Play::CreateGameObject(TYPE_LASER, objAgent8.pos, 50, "laser");
		GameObject& objLaser = Play::GetGameObject(id);
		objLaser.rotation = objAgent8.rotation;
	}

	Play::UpdateGameObject(objAgent8);
}

void UpdateAgent8()
{
	GameObject& objAgent8 = Play::GetGameObjectByType(TYPE_AGENT8);
	GameObject& objSpecialAsteroid = Play::GetGameObjectByType(TYPE_SPECIAL);
	GameObject& objRocket = Play::GetGameObjectByType(TYPE_ROCKET);

	switch (gameState.agent8State)
	{
		case STATE_ATTACHED:
		{	
			objAgent8.velocity = { 0.f, 0.f };
			objAgent8.acceleration = { 0.f, 0.f };
			objAgent8.pos = objSpecialAsteroid.pos;

			if (gameState.playMode)
				AttachedPlayerControls();
			
			break;
		}
		case STATE_ROCKET:
		{
			CreateRocketParticles();
			objAgent8.velocity = { 0.f, 0.f };
			objAgent8.acceleration = { 0.f, 0.f };
			objAgent8.pos = objRocket.pos;
			objAgent8.rotation = objRocket.rotation - Play::DegToRad(90);
			objAgent8.animSpeed = 0.f;
			Play::SetSprite( objAgent8, "agent8_right", 0);

			if (gameState.playMode)
				RocketPlayerControls();
			
			break;
		}
		case STATE_FLY:
		{
			CreateParticles();
			FloatDirectionObject(objAgent8, AGENT8_SPEED);
			AsteroidCollision();
			objAgent8.velocity *= RESISTANCE;
			Play::SetSprite( objAgent8, "agent8_fly", 0);

			if (gameState.playMode)
				HandlePlayerControls();

			break;
		}
		case STATE_DEAD:
		{
			FloatDirectionObject(objAgent8, 0.5f);
			break;
		}
	}

	LoopObject(objAgent8);

	Play::UpdateGameObject(objAgent8);
}

void UpdateAsteroidPieces()
{
	std::vector<int> vPieces = Play::CollectGameObjectIDsByType(TYPE_PIECES);

	for (int idPiece : vPieces)
	{
		GameObject& objPiece = Play::GetGameObject(idPiece);
		//FloatDirectionObject(objPiece, ASTEROID_SPEED);
		Play::UpdateGameObject(objPiece);
		
		if (!DisplayAreaTest(objPiece, PIECES_AABB))
			objPiece.type = TYPE_DESTROYED;
	}
}

void UpdateLasers()
{
	std::vector<int> vLasers = Play::CollectGameObjectIDsByType(TYPE_LASER);
	std::vector<int> vMeteors = Play::CollectGameObjectIDsByType(TYPE_METEOR);
	std::vector<int> vAsteroids = Play::CollectGameObjectIDsByType(TYPE_ASTEROID);
	
	for (int idLaser : vLasers)
	{
		GameObject& objLaser = Play::GetGameObject(idLaser);
		FloatDirectionObject(objLaser, 10.f);

		for (int meteorId : vMeteors)
		{
			GameObject& objMeteor = Play::GetGameObject(meteorId);
			if (IsCollidingLaser(objLaser, objMeteor))
			{
				if (gameState.sound)
					Play::PlayAudio("clang");

				objLaser.type = TYPE_DESTROYED;
				gameState.meteorCollisions++;

				if (gameState.meteorCollisions > 25)
				{
					objMeteor.type = TYPE_DESTROYED;
					gameState.score += 300;
				}
			}
		}

		for (int asteroidId : vAsteroids)
		{
			GameObject& objAsteroid = Play::GetGameObject(asteroidId);
			if (IsCollidingLaser(objLaser, objAsteroid))
			{
				if (gameState.sound)
					Play::PlayAudio("explosion");

				objLaser.type = TYPE_DESTROYED;
				gameState.asteroidCollisions++;

				if (gameState.asteroidCollisions > 10)
				{
					AsteroidExplosion(objAsteroid);
					objAsteroid.type = TYPE_DESTROYED;
					gameState.asteroidDestroyed = true;
				}			
			}
		}
		Play::UpdateGameObject(objLaser);
	}
}

void UpdateMeteors()
{
	std::vector<int> vMeteors = Play::CollectGameObjectIDsByType(TYPE_METEOR);
	GameObject& objRocket = Play::GetGameObjectByType(TYPE_ROCKET);

	bool collision = false;

	if (gameState.agent8State == STATE_FLY || gameState.agent8State == STATE_ROCKET)
		collision = true;

	for (int idMeteor : vMeteors)
	{
		GameObject& objMeteor = Play::GetGameObject(idMeteor);

		if (collision && IsColliding(objMeteor) && gameState.playMode && !gameState.powerActivated)
		{
			if(gameState.sound)
				Play::PlayAudio("combust");

			if (gameState.agent8State == STATE_ROCKET)
			{
				gameState.agent8State = STATE_DEAD;
				objRocket.type = TYPE_DESTROYED;
			}

			GameObject& objAgent8 = Play::GetGameObjectByType(TYPE_AGENT8);
			Play::SetSprite(objAgent8, "agent8_dead", 0.25f);
			objAgent8.rotation = objMeteor.rotation;
			gameState.agent8State = STATE_DEAD;
			objMeteor.type = TYPE_DESTROYED;
			gameState.levelInfo = false;
			gameState.playMode = false;
		}

		objMeteor.rotation += Play::DegToRad(0.1);
		FloatDirectionObject(objMeteor, METEOR_SPEED);
		LoopObject(objMeteor);
		Play::UpdateGameObject(objMeteor);
	}
}

void UpdateAsteroids()
{
	std::vector<int> vAsteroids = Play::CollectGameObjectIDsByType(TYPE_ASTEROID);

	if (gameState.asteroidDestroyed)
	{
		gameState.asteroidCollisions = 0;
		gameState.asteroidDestroyed = false;
	}

	if ( vAsteroids.size() == 0 && gameState.gems > 0 )
		gameState.playMode = false;

	for (int idAsteroid : vAsteroids)
	{
		GameObject& objAsteroid = Play::GetGameObject(idAsteroid);
		FloatDirectionObject(objAsteroid, ASTEROID_SPEED);
		LoopObject(objAsteroid);
		Play::UpdateGameObject(objAsteroid);
	}

	GameObject& objSpecialAsteroid = Play::GetGameObjectByType(TYPE_SPECIAL);
	FloatDirectionObject(objSpecialAsteroid, SPECIAL_ASTEROID_SPEED);
	LoopObject(objSpecialAsteroid);
	Play::UpdateGameObject(objSpecialAsteroid);

}

void UpdateRocket()
{
	std::vector<int> vRockets = Play::CollectGameObjectIDsByType(TYPE_ROCKET);
	float rocketSpeed = 1.f;

	for (int idRocket : vRockets)
	{
		GameObject& objRocket = Play::GetGameObject(idRocket);
		if (gameState.agent8State == STATE_ROCKET)
			rocketSpeed = ROCKET_SPEED;

		if (IsColliding(objRocket) && gameState.agent8State == STATE_FLY)
		{
			GameObject& objAgent8 = Play::GetGameObjectByType(TYPE_AGENT8);
			Play::MoveSpriteOrigin("agent8_right", 0, -35);
			gameState.agent8State = STATE_ROCKET;

			if (gameState.sound)
				Play::StartAudioLoop("fan");
		}

		objRocket.rotation += Play::DegToRad(0.1);
		FloatDirectionObject(objRocket, rocketSpeed);
		LoopObject(objRocket);
		Play::UpdateGameObject(objRocket);
	}
}

void UpdateGemsAndDiamonds()
{
	std::vector<int> vGems = Play::CollectGameObjectIDsByType(TYPE_GEM);
	std::vector<int> vDiamonds = Play::CollectGameObjectIDsByType(TYPE_DIAMOND);

	bool collision = false;

	if (gameState.agent8State == STATE_FLY || gameState.agent8State == STATE_ROCKET)
		collision = true;

	if (vGems.size() == 0 && vDiamonds.size() == 0 && gameState.gemsCollected == MAX_GEMS)
		gameState.levelPassed = true;

	for (int idGem : vGems)
	{
		GameObject& objGem = Play::GetGameObject(idGem);
		objGem.rotation += Play::DegToRad(1);

		if (IsColliding(objGem) && collision)
		{
			if (gameState.sound)
				Play::PlayAudio("reward");

			gameState.gemsCollected++;
			gameState.score += 500;
			int id = Play::CreateGameObject(TYPE_RING, objGem.pos, 50, "ring");
			GameObject& objRing = Play::GetGameObject(id);

			gameState.isAnimation = true;
			objGem.type = TYPE_DESTROYED;
		}

		FloatDirectionObject(objGem, GEM_SPEED);
		LoopObject(objGem);

		Play::UpdateGameObject(objGem);
	}

	for (int idDiamonds : vDiamonds)
	{
		GameObject& objDiamond = Play::GetGameObject(idDiamonds);
		objDiamond.rotation += Play::DegToRad(1);

		if (IsColliding(objDiamond) && collision)
		{
			if (gameState.sound)
				Play::PlayAudio("reward");

			gameState.gemsCollected++;
			gameState.powerActivated = true;
			gameState.score += 1000;
			GameObject& objAgent8 = Play::GetGameObjectByType(TYPE_AGENT8);

			int id = Play::CreateGameObject(TYPE_SHIELD, objAgent8.pos, 50, "ring");
			GameObject& objShield = Play::GetGameObject(id);
			objShield.rotation = objAgent8.rotation;
			gameState.powerActivated = true;
			gameState.isAnimation = true;
			
			objDiamond.type = TYPE_DESTROYED;
		}

		FloatDirectionObject(objDiamond, GEM_SPEED);
		LoopObject(objDiamond);
	}
}

void CreateParticles()
{
	GameObject& objAgent8 = Play::GetGameObjectByType(TYPE_AGENT8);

	Play::CreateGameObject(TYPE_PARTICLE, { objAgent8.pos.x + Play::RandomRollRange(-20, 20), objAgent8.pos.y + Play::RandomRollRange(-15, 15)}, 50, "particle");
	Play::CreateGameObject(TYPE_PARTICLE, { objAgent8.pos.x + Play::RandomRollRange(-20, 20), objAgent8.pos.y + Play::RandomRollRange(-15, 15)}, 50, "particle");
}

void CreateRocketParticles()
{
	GameObject& objRocket = Play::GetGameObjectByType(TYPE_ROCKET);

	Play::CreateGameObject(TYPE_PARTICLE, { objRocket.pos.x + Play::RandomRollRange(-20, 20), objRocket.pos.y + Play::RandomRollRange(-25, 25)}, 50, "particle");
	Play::CreateGameObject(TYPE_PARTICLE, { objRocket.pos.x + Play::RandomRollRange(-20, 20), objRocket.pos.y + Play::RandomRollRange(-25, 25)}, 50, "particle");
	Play::CreateGameObject(TYPE_PARTICLE, { objRocket.pos.x + Play::RandomRollRange(-20, 20), objRocket.pos.y + Play::RandomRollRange(-25, 25)}, 50, "particle");

}

void UpdateParticles()
{
	std::vector<int> vParticles = Play::CollectGameObjectIDsByType(TYPE_PARTICLE);

	for (int idParticle : vParticles)
	{
		GameObject& objParticle = Play::GetGameObject(idParticle);
		objParticle.frame++;

		if (objParticle.frame >= 20 || !DisplayAreaTest(objParticle, PARTICLE_AABB)) 
			objParticle.type = TYPE_DESTROYED;

		Play::UpdateGameObject(objParticle);
	}
}

void UpdateRings()                                                                                                      
{
	std::vector<int> vShields = Play::CollectGameObjectIDsByType(TYPE_SHIELD);

	for (int idShield : vShields)
	{
		GameObject& objShield = Play::GetGameObject(idShield);
		objShield.pos = Play::GetGameObjectByType(TYPE_AGENT8).pos;
		objShield.frame++;

		if (objShield.frame >= SHIELD_DURATION)
		{
			objShield.type = TYPE_DESTROYED;
			gameState.isAnimation = false;
			gameState.powerActivated = false;
		}
	}

	std::vector<int> vRings = Play::CollectGameObjectIDsByType(TYPE_RING);
	for (int idRing : vRings)
	{
		GameObject& objRing = Play::GetGameObject(idRing);
		objRing.frame++;

		if (objRing.frame >= 30 || !DisplayAreaTest(objRing, PARTICLE_AABB) || !Play::IsVisible(objRing) )
			objRing.type = TYPE_DESTROYED;
	}	
	if (vRings.size() == 0)
		scale = { 1.f };

	if (vShields.size() == 0 && vRings.size() == 0)
		gameState.isAnimation = false;
}

bool IsColliding(const GameObject& object)
{
	Vector2D AABB = { 0.f, 0.f };
	GameObject& agent8 = Play::GetGameObjectByType(TYPE_AGENT8);

	switch (object.type)
	{
		case TYPE_ASTEROID:
			AABB = ASTEROID_AABB;
			break;
		case TYPE_METEOR:
			AABB = METEOR_AABB;
			break;
		case TYPE_GEM:
			AABB = GEM_AABB;
			break;
		case TYPE_DIAMOND:
			AABB = GEM_AABB;
			break;
		case TYPE_LASER:
			AABB = LASER_AABB;
			break;
		case TYPE_ROCKET:
			AABB = ROCKET_AABB;
			break;
	}

	if (agent8.pos.y - AGENT8_AABB.y < object.pos.y + AABB.y &&
		agent8.pos.y + AGENT8_AABB.y > object.pos.y - AABB.y)
	{
		if (agent8.pos.x + AGENT8_AABB.x > object.pos.x - AABB.x &&
			agent8.pos.x - AGENT8_AABB.x < object.pos.x + AABB.x)
		{
			gameState.collisionCount++;
			return true;
		}	
	}
	return false;
}

bool IsCollidingLaser(const GameObject& laser, const GameObject& object)
{
	Vector2D AABB = { 0.f, 0.f };

	switch (object.type)
	{
		case TYPE_ASTEROID:
			AABB = ASTEROID_AABB;
			break;
		case TYPE_METEOR:
			AABB = METEOR_AABB;
			break;
	}

	if (object.pos.y - AABB.y < laser.pos.y + LASER_AABB.y &&
		object.pos.y + AABB.y > laser.pos.y - LASER_AABB.y)
	{
		if (object.pos.x + AABB.x > laser.pos.x - LASER_AABB.x &&
			object.pos.x - AABB.x < laser.pos.x + LASER_AABB.x)
		{
			return true;
		}
	}
	return false;
}

void AsteroidCollision()
{
	GameObject& objSpecialAsteroid = Play::GetGameObjectByType(TYPE_SPECIAL);
	GameObject& objAgent8 = Play::GetGameObjectByType(TYPE_AGENT8);

	

	if (IsColliding(objSpecialAsteroid) && gameState.agent8State == STATE_FLY && gameState.playMode)
	{
		Play::SetSprite(objAgent8, "agent8_left", 0);
		//Play::MoveSpriteOrigin("agent8_right", 0, 35);
		gameState.agent8State = STATE_ATTACHED;
		objAgent8.rotation = objSpecialAsteroid.rotation;
	}

	std::vector<int> vAsteroids = Play::CollectGameObjectIDsByType(TYPE_ASTEROID);

	for (int idAsteroid : vAsteroids)
	{
		GameObject& objAsteroid = Play::GetGameObject(idAsteroid);

		if (gameState.agent8State == STATE_FLY  && IsColliding(objAsteroid) && gameState.playMode)
		{
			Vector2D oldPos = objSpecialAsteroid.pos;
			float oldRotaton = objSpecialAsteroid.rotation;
			objSpecialAsteroid.pos = objAsteroid.pos;
			objSpecialAsteroid.rotation = objAsteroid.rotation;
			objAsteroid.pos = oldPos;
			objAsteroid.rotation = oldRotaton;
			objAgent8.rotation = objSpecialAsteroid.rotation;

			Play::SetSprite(objAgent8, "agent8_left", 0);
			//Play::MoveSpriteOrigin("agent8_right", 0, 35);
			gameState.agent8State = STATE_ATTACHED;
		}
	}
}

void AsteroidExplosion(GameObject& asteroid)
{
	int id = Play::CreateGameObject(TYPE_PIECES, asteroid.pos, 50, "asteroid_pieces");
	GameObject& objAsteroidPieces1 = Play::GetGameObject(id);
	Play::SetSprite(objAsteroidPieces1, "asteroid_pieces", 0.f);
	objAsteroidPieces1.frame = 1;
	objAsteroidPieces1.velocity = ASTEROID_PIECE_VELOCITY_3;

	id = Play::CreateGameObject(TYPE_PIECES, asteroid.pos, 50, "asteroid_pieces");
	GameObject& objAsteroidPieces2 = Play::GetGameObject(id);
	Play::SetSprite(objAsteroidPieces2, "asteroid_pieces", 0.f);
	objAsteroidPieces2.frame = 2;
	objAsteroidPieces2.velocity = ASTEROID_PIECE_VELOCITY_2;

	id = Play::CreateGameObject(TYPE_PIECES, asteroid.pos, 50, "asteroid_pieces");
	GameObject& objAsteroidPieces3 = Play::GetGameObject(id);
	Play::SetSprite(objAsteroidPieces3, "asteroid_pieces", 0.f);
	objAsteroidPieces3.frame = 3;
	objAsteroidPieces3.velocity = ASTEROID_PIECE_VELOCITY_1;

	if (asteroid.type == TYPE_SPECIAL)
	{
		if (gameState.gems > 0)
		{
			int posX = asteroid.pos.x- 200;
			int posY = asteroid.pos.y -200;

			if (posX < 100 || posX > DISPLAY_WIDTH - 100)
				posX = DISPLAY_WIDTH / 2;

			if (posY < 100 || posY > DISPLAY_HEIGHT - 100)
				posY = DISPLAY_HEIGHT / 2;

			if (Play::RandomRoll(6) == 2)
				id = Play::CreateGameObject(TYPE_DIAMOND, { posX, posY }, 50, "diamond");
			else
				id = Play::CreateGameObject(TYPE_GEM, { posX, posY }, 50, "gem");

			gameState.gems--;
			GameObject& objGem = Play::GetGameObject(id);
			objGem.rotation = Randomize(630, 0.01);
		}
		asteroid.pos = { DISPLAY_WIDTH + 100.f, -100.f };
	}	
}

void LoopObject(GameObject& object)
{
	Vector2D AABB = { 0.f, 0.f };

	switch (object.type)
	{
		case TYPE_ASTEROID:
			AABB = ASTEROID_AABB;
			break;
		case TYPE_SPECIAL:
			AABB = ASTEROID_AABB;
			break;
		case TYPE_METEOR:
			AABB = METEOR_AABB;
			break;
		case TYPE_GEM:
			AABB = GEM_AABB;
			break;
		case TYPE_AGENT8:
			AABB = AGENT8_AABB;
			break;
	}

	if (object.pos.x > (DISPLAY_WIDTH + AABB.x))
		object.pos.x = -AABB.x;
	if (object.pos.x < -AABB.x)
		object.pos.x = (DISPLAY_WIDTH + AABB.x);
	if (object.pos.y > (DISPLAY_HEIGHT + AABB.y))
		object.pos.y = -AABB.y;
	if (object.pos.y < -AABB.y)
		object.pos.y = (DISPLAY_HEIGHT + AABB.y);
}

bool DisplayAreaTest(GameObject& object, Vector2D AABB)
{
	if (object.pos.x > (DISPLAY_WIDTH + AABB.x) || object.pos.x < -AABB.x 
		|| object.pos.y >(DISPLAY_HEIGHT + AABB.y) || object.pos.y < -AABB.y )
		return false;

	return true;
}

void DestroyAllItems()
{
	Play::GetGameObjectByType(TYPE_AGENT8).type = TYPE_DESTROYED;
	Play::GetGameObjectByType(TYPE_SPECIAL).type = TYPE_DESTROYED;
	Play::GetGameObjectByType(TYPE_ROCKET).type = TYPE_DESTROYED;

	std::vector<int> vAsteroids = Play::CollectGameObjectIDsByType(TYPE_ASTEROID);
	for (int idAsteroid : vAsteroids)
	{
		Play::GetGameObject(idAsteroid).type = TYPE_DESTROYED;
	}

	std::vector<int> vMeteors = Play::CollectGameObjectIDsByType(TYPE_METEOR);
	for (int idMeteor : vMeteors)
	{
		Play::GetGameObject(idMeteor).type = TYPE_DESTROYED;
	}

	std::vector<int> vGems = Play::CollectGameObjectIDsByType(TYPE_GEM);
	for (int idGem : vGems)
	{
		Play::GetGameObject(idGem).type = TYPE_DESTROYED;
	}

	std::vector<int> vPieces = Play::CollectGameObjectIDsByType(TYPE_PIECES);
	for (int idPiece : vPieces)
	{
		Play::GetGameObject(idPiece).type = TYPE_DESTROYED;
	}

	std::vector<int> vParticles = Play::CollectGameObjectIDsByType(TYPE_PARTICLE);
	for (int idParticle : vParticles)
	{
		Play::GetGameObject(idParticle).type = TYPE_DESTROYED;
	}

	std::vector<int> vDiamonds = Play::CollectGameObjectIDsByType(TYPE_DIAMOND);
	for (int idDiamond : vDiamonds)
	{
		Play::GetGameObject(idDiamond).type = TYPE_DESTROYED;
	}

	std::vector<int> vRings = Play::CollectGameObjectIDsByType(TYPE_RING);
	for (int idRing : vRings)
	{
		Play::GetGameObject(idRing).type = TYPE_DESTROYED;
	}

	std::vector <int> vShields = Play::CollectGameObjectIDsByType(TYPE_SHIELD);
	for (int idShield : vShields)
	{
		Play::GetGameObject(idShield).type = TYPE_DESTROYED;
	}
}

void UpdateDestroyed()
{
	std::vector<int> vDead = Play::CollectGameObjectIDsByType(TYPE_DESTROYED);

	for (int id_dead : vDead)
	{
		Play::DestroyGameObject(id_dead);
	}
}

void ResetGame()
{
	DestroyAllItems();

	if (gameState.highScore < gameState.score)
		gameState.highScore = gameState.score;

	gameState.paused = { false };
	gameState.collisionCount = { 0 };
	gameState.gems = { MAX_GEMS };
	gameState.gemsCollected = { 0 };
	gameState.playMode = { true };
	gameState.meteorCollisions = { 0 };
	gameState.asteroidCollisions = { 0 };
	gameState.asteroidDestroyed = { false };
	gameState.powerActivated = { false };
	gameState.levelInfo = { true };

	Play::CentreAllSpriteOrigins(); 

	gameState.agent8State =   STATE_ATTACHED;
	gameState.state = STATE_LOBBY;
}