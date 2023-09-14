#define PLAY_IMPLEMENTATION
#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Play.h"

int DISPLAY_WIDTH  = 1280;
int DISPLAY_HEIGHT = 720;
int DISPLAY_SCALE = 1;

const Vector2D METEOR_VELOCITY_DEFAULT{ 1.0f, 0.f }; 
const Vector2D AGENT8_VELOCITY_DEFAULT{ 2.f, 0.f };

const float AGENT8_SPEED{ 5.0f };
const float SPECIAL_ASTEROID_SPEED{ 1.0f };
const float ASTEROID_SPEED{ 2.0f };
const float METEOR_SPEED{ 1.0f };
const float GEM_SPEED{ 0.5f };
const float ASTEROID_PIECE_SPEED{ 15.f };

const Vector2D ASTEROID_PIECE_VELOCITY_1{ 0.f, -ASTEROID_PIECE_SPEED };
const Vector2D ASTEROID_PIECE_VELOCITY_2{ 3.f, ASTEROID_PIECE_SPEED };
const Vector2D ASTEROID_PIECE_VELOCITY_3{ -ASTEROID_PIECE_SPEED, 0.f };

const Vector2D LASER_VELOCITY_DEFAULT{ 0.f, 0.f };
const Vector2D LASER_ACCELERATION{ 0.f, 0.f };

const int MAX_GEMS{ 5 };
const int MAX_ASTEROIDS{ 3 };
const int SHIELD_DURATION{ 300 };

const Vector2D METEOR_AABB{ 50.f, 50.f };
const Vector2D ASTEROID_AABB{ 50.f, 50.f };
const Vector2D PIECES_AABB{ 20.f, 20.f };
const Vector2D AGENT8_AABB{ 30.f, 30.f };
const Vector2D PARTICLE_AABB{ 20.f, 20.f };
const Vector2D GEM_AABB{ 20.f, 20.f };
const Vector2D LASER_AABB{ 5.f, 20.f };

enum Agent8State
{
	STATE_ATTACHED = 0,
	STATE_FLY,
	STATE_DEAD,
	STATE_WON,
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
	bool playMode{ true };
	bool paused{ false };
	bool sound{ false };
	bool music{ false };
	bool powerActivated{ false };
	int collisionCount{ 0 };
	int meteorCollisions{ 0 };

	Agent8State agent8State = STATE_ATTACHED;
	GameFlow state = STATE_LOBBY;
};

GameState gameState;

enum gameObjectType
{
	TYPE_NULL = -1,
	TYPE_AGENT8,
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

void UpdateAgent8();
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
void LoopObject(GameObject& object);
bool IsColliding(const GameObject& object);
bool IsCollidingLaser(const GameObject& object);
void AsteroidCollision();
void AsteroidExplosion();

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
	Play::DrawBackground();
	DrawSoundControl();

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
			if (Play::KeyPressed(VK_SPACE))
			{
				gameState.paused = false;
				gameState.state = STATE_GAME_PLAY;
			}
			break;
		}
	}

	UpdateDestroyed();
	SoundControl();

	if (Play::KeyPressed(VK_TAB))
		ResetGame();
	
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

	objAgent8.rotSpeed = objSpecialAsteroid.rotSpeed;

	for (int i = 0; i < MAX_ASTEROIDS; i++)
	{
		int id = Play::CreateGameObject(TYPE_ASTEROID, { Play::RandomRoll(DISPLAY_WIDTH), Play::RandomRoll(DISPLAY_WIDTH) }, 50, "asteroid");
		GameObject& objAsteroid = Play::GetGameObject(id);
		objAsteroid.rotation = Randomize(630, 0.01);
	}

	int id = Play::CreateGameObject(TYPE_METEOR, { Play::RandomRoll(DISPLAY_WIDTH), Play::RandomRoll(DISPLAY_WIDTH) }, 50, "meteor");
	GameObject& objMeteor = Play::GetGameObject(id);
	Play::SetSprite(objMeteor, "meteor", 0.25f);
	objMeteor.rotation = Play::DegToRad(90);
}

void DrawSoundControl()
{
	Play::DrawFontText("64px", (gameState.sound) ? "SOUND: ON" : "SOUND: OFF", Point2D(100, 150), Play::CENTRE);
	Play::DrawFontText("64px", (gameState.music) ? "MUSIC: ON" : "MUSIC: OFF", Point2D(100, 200), Play::CENTRE);
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
		Play::DrawObject(objParticle);
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
		GameObject& objRing = Play::GetGameObject(idRing);
		Play::DrawSpriteRotated("ring", objRing.pos, 1,  objRing.rotation, 3.0f, .1f);
		Play::DrawSpriteRotated("ring", objRing.pos, 1,  objRing.rotation, 2.5f, .2f);
		Play::DrawSpriteRotated("ring", objRing.pos, 1,  objRing.rotation, 1.8f, .3f);
	}

	std::vector<int> vLasers = Play::CollectGameObjectIDsByType(TYPE_LASER);
	for (int id_laser : vLasers)
	{
		GameObject& objLaser = Play::GetGameObject(id_laser);
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
	Play::DrawObjectRotated(objAgent8);

	DrawGameStats();
	DrawSoundControl();

	if (!gameState.playMode)
	{
		Play::DrawFontText("151px", (gameState.agent8State == STATE_DEAD) ? "GAME OVER" : "VICTORY !!!", {DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2}, Play::CENTRE);
		Play::DrawFontText("64px", "PRESS TAB TO RESTART", { DISPLAY_WIDTH / 2, DISPLAY_HEIGHT - 100 }, Play::CENTRE);
	}

	Play::PresentDrawingBuffer();
}

void DrawLobby()
{
	Play::DrawFontText("64px", (gameState.paused) ? "PAUSED" : "WELCOME TO SKY HIGH SPYDER !", { DISPLAY_WIDTH / 2, 300 }, Play::CENTRE);
	Play::DrawFontText("64px", (gameState.paused) ? "PRESS SPACE TO CONTINUE" : "PRESS ENTER TO START || SHIFT TO PAUSE" , { DISPLAY_WIDTH / 2, DISPLAY_HEIGHT - 300 }, Play::CENTRE);
	Play::DrawFontText("64px", (gameState.paused) ? "" : "ARROW KEYS TO MOVE LEFT / RIGHT || SHIFT FOR LASERS || SPACE TO FLY", { DISPLAY_WIDTH / 2, DISPLAY_HEIGHT - 200 }, Play::CENTRE);
	Play::DrawFontText("64px", "F2 FOR SOUND || F3 FOR MUSIC", { DISPLAY_WIDTH / 2, DISPLAY_HEIGHT - 100 }, Play::CENTRE);

	Play::PresentDrawingBuffer();
}

void DrawGameStats()
{	
	GameObject& objAgent8 = Play::GetGameObjectByType(TYPE_AGENT8);
	GameObject& objMeteor = Play::GetGameObjectByType(TYPE_METEOR);
	Play::DrawFontText("132px", "SCORE: " + std::to_string(gameState.score), { DISPLAY_WIDTH / 2, 50 }, Play::CENTRE);
	//Play::DrawFontText("64px", "PlayMode: " + std::to_string(gameState.playMode), { DISPLAY_WIDTH - 100, 50 }, Play::CENTRE);
	Play::DrawFontText("64px", "Level: " + std::to_string(gameState.level), { DISPLAY_WIDTH - 300, 50 }, Play::CENTRE);
	//Play::DrawFontText("64px", "Total: " + std::to_string(gameState.highScore), { 100, 50 }, Play::CENTRE);
	Play::DrawFontText("64px", "Collisions: " + std::to_string(gameState.meteorCollisions), { 300, 50 }, Play::CENTRE);
	Play::DrawFontText("64px", "Agent State: " + std::to_string(gameState.agent8State), { 300, 100 }, Play::CENTRE);
	Play::DrawFontText("64px", "Gems: " + std::to_string(gameState.gems), { 300, 200 }, Play::CENTRE);
}

void HandlePlayerControls()
{
	GameObject& objAgent8 = Play::GetGameObjectByType(TYPE_AGENT8); 
	//Play::SetSprite(objAgent8, "agent8_left", 0.f);
	 
	if (Play::KeyDown(VK_LEFT))
	{
		objAgent8.rotation -= 0.1f;

		if (gameState.agent8State == STATE_ATTACHED)
			Play::SetSprite( objAgent8, "agent8_left", 0.5f );
	}
	if (Play::KeyDown(VK_RIGHT))
	{
		objAgent8.rotation += 0.1f;

		if (gameState.agent8State == STATE_ATTACHED)
			Play::SetSprite( objAgent8, "agent8_right", 0.5f );
	}
	if (Play::KeyPressed(VK_SPACE))
	{
		if (gameState.sound)
			Play::PlayAudio("explode");

		Play::SetSprite( objAgent8, "agent8_fly", 0.0f );
		AsteroidExplosion();
		gameState.agent8State = STATE_FLY;
	}
	if (Play::KeyDown(VK_SHIFT))
	{
		if (gameState.sound)
			Play::PlayAudio("laser");

		int id = Play::CreateGameObject(TYPE_LASER, objAgent8.pos, 50, "laser");
		GameObject& objLaser = Play::GetGameObject(id);
		objLaser.rotation = objAgent8.rotation;
	}
	Play::UpdateGameObject(objAgent8);
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

void UpdateAgent8()
{
	GameObject& objAgent8 = Play::GetGameObjectByType(TYPE_AGENT8);
	GameObject& objSpecialAsteroid = Play::GetGameObjectByType(TYPE_SPECIAL);


	switch (gameState.agent8State)
	{
		case STATE_ATTACHED:
		{	
			objAgent8.velocity = { 0.f, 0.f };
			objAgent8.acceleration = { 0.f, 0.f };
			objAgent8.pos = objSpecialAsteroid.pos;
			break;
		}
		case STATE_FLY:
		{
			CreateParticles();
			FloatDirectionObject(objAgent8, AGENT8_SPEED);
			AsteroidCollision();
			break;
		}
		case STATE_DEAD:
		{
			FloatDirectionObject(objAgent8, 0.5f);
			break;
		}
	}

	LoopObject(objAgent8);

	if (gameState.playMode)
		HandlePlayerControls();

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

	for (int idLaser : vLasers)
	{
		GameObject& objLaser = Play::GetGameObject(idLaser);
		FloatDirectionObject(objLaser, 10.f);

		if (IsCollidingLaser(objLaser))
		{
			if (gameState.sound)
				Play::PlayAudio("clang");

			objLaser.type = TYPE_DESTROYED;
			gameState.meteorCollisions++;

			if (gameState.meteorCollisions > 25)
			{
				GameObject& objMeteor = Play::GetGameObjectByType(TYPE_METEOR);
				objMeteor.type = TYPE_DESTROYED;
				gameState.score += 300;
			}
		}

		Play::UpdateGameObject(objLaser);
	}
}

void UpdateMeteors()
{
	std::vector<int> vMeteors = Play::CollectGameObjectIDsByType(TYPE_METEOR);

	for (int idMeteor : vMeteors)
	{
		GameObject& objMeteor = Play::GetGameObject(idMeteor);

		if (IsColliding(objMeteor) && gameState.playMode && gameState.agent8State == STATE_FLY && !gameState.powerActivated)
		{
			if(gameState.sound)
				Play::PlayAudio("combust");

			GameObject& objAgent8 = Play::GetGameObjectByType(TYPE_AGENT8);
			Play::SetSprite(objAgent8, "agent8_dead", 0.25f);
			objAgent8.rotation = objMeteor.rotation;
			gameState.agent8State = STATE_DEAD;
			objMeteor.type = TYPE_DESTROYED;
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

void UpdateGemsAndDiamonds()
{
	std::vector<int> vGems = Play::CollectGameObjectIDsByType(TYPE_GEM);
	std::vector<int> vDiamonds = Play::CollectGameObjectIDsByType(TYPE_DIAMOND);

	if (vGems.size() == 0 && vDiamonds.size() == 0 && gameState.gems == 0)
		gameState.playMode = false;	

	for (int idGem : vGems)
	{
		GameObject& objGem = Play::GetGameObject(idGem);
		objGem.rotation += Play::DegToRad(1);

		if (IsColliding(objGem) && gameState.agent8State == STATE_FLY)
		{
			if (gameState.sound)
				Play::PlayAudio("reward");

			gameState.gems--;
			gameState.score += 500;
			int id = Play::CreateGameObject(TYPE_RING, objGem.pos, 50, "ring");
			GameObject& objRing = Play::GetGameObject(id);
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

		if (IsColliding(objDiamond) && gameState.agent8State == STATE_FLY)
		{
			if (gameState.sound)
				Play::PlayAudio("reward");

			gameState.gems--;
			gameState.powerActivated = true;
			gameState.score += 1000;
			GameObject& objAgent8 = Play::GetGameObjectByType(TYPE_AGENT8);

			int id = Play::CreateGameObject(TYPE_SHIELD, objAgent8.pos, 50, "ring");
			GameObject& objShield = Play::GetGameObject(id);
			objShield.rotation = objAgent8.rotation;
			gameState.powerActivated = true;
			
			objDiamond.type = TYPE_DESTROYED;
		}

		FloatDirectionObject(objDiamond, GEM_SPEED);
		LoopObject(objDiamond);
	}
}

void CreateParticles()
{
	GameObject& objAgent8 = Play::GetGameObjectByType(TYPE_AGENT8);

	Play::CreateGameObject(TYPE_PARTICLE, objAgent8.pos, 50, "particle");
}

void UpdateParticles()
{
	std::vector<int> vParticles = Play::CollectGameObjectIDsByType(TYPE_PARTICLE);

	for (int idParticle : vParticles)
	{
		GameObject& objParticle = Play::GetGameObject(idParticle);
		objParticle.frame++;
		

		if (objParticle.frame >= 10 || !DisplayAreaTest(objParticle, PARTICLE_AABB)) 
			objParticle.type = TYPE_DESTROYED;
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
			gameState.powerActivated = false;
		}
	}

	std::vector<int> vRings = Play::CollectGameObjectIDsByType(TYPE_RING);

	for (int idRing : vRings)
	{
		GameObject& objRing = Play::GetGameObject(idRing);
		objRing.frame++;

		if (objRing.frame >= 10 || !DisplayAreaTest(objRing, PARTICLE_AABB) || !Play::IsVisible(objRing) )
			objRing.type = TYPE_DESTROYED;
	}	
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

bool IsCollidingLaser(const GameObject& object)
{
	GameObject& objMeteor = Play::GetGameObjectByType(TYPE_METEOR);

	if (objMeteor.pos.y - METEOR_AABB.y < object.pos.y + LASER_AABB.y &&
		objMeteor.pos.y + METEOR_AABB.y > object.pos.y - LASER_AABB.y)
	{
		if (objMeteor.pos.x + METEOR_AABB.x > object.pos.x - LASER_AABB.x &&
			objMeteor.pos.x - METEOR_AABB.x < object.pos.x + LASER_AABB.x)
		{
			return true;
		}
	}	
	return false;
}

void AsteroidCollision()
{
	GameObject& objSpecialAsteroid = Play::GetGameObjectByType(TYPE_SPECIAL);

	if (IsColliding(objSpecialAsteroid) && gameState.agent8State == STATE_FLY && gameState.playMode)
		gameState.agent8State = STATE_ATTACHED;

	std::vector<int> vAsteroids = Play::CollectGameObjectIDsByType(TYPE_ASTEROID);

	for (int id_asteroid : vAsteroids)
	{
		GameObject& objAsteroid = Play::GetGameObject(id_asteroid);

		if (gameState.agent8State == STATE_FLY && IsColliding(objAsteroid) && gameState.playMode)
		{
			Vector2D oldPos = objSpecialAsteroid.pos;
			objSpecialAsteroid.pos = objAsteroid.pos;
			objAsteroid.pos = oldPos;

			GameObject& objAgent8 = Play::GetGameObjectByType(TYPE_AGENT8);
			Play::SetSprite(objAgent8, "agent8_left", 0.25f);
			gameState.agent8State = STATE_ATTACHED;
		}
	}
}

void AsteroidExplosion()
{
	GameObject& objSpecialAsteroid = Play::GetGameObjectByType(TYPE_SPECIAL);

	int id = Play::CreateGameObject(TYPE_PIECES, objSpecialAsteroid.pos, 50, "asteroid_pieces");
	GameObject& objAsteroidPieces1 = Play::GetGameObject(id);
	Play::SetSprite(objAsteroidPieces1, "asteroid_pieces", 0.f);
	objAsteroidPieces1.frame = 1;
	objAsteroidPieces1.velocity = ASTEROID_PIECE_VELOCITY_3;

	id = Play::CreateGameObject(TYPE_PIECES, objSpecialAsteroid.pos, 50, "asteroid_pieces");
	GameObject& objAsteroidPieces2 = Play::GetGameObject(id);
	Play::SetSprite(objAsteroidPieces2, "asteroid_pieces", 0.f);
	objAsteroidPieces2.frame = 2;
	objAsteroidPieces2.velocity = ASTEROID_PIECE_VELOCITY_2;

	id = Play::CreateGameObject(TYPE_PIECES, objSpecialAsteroid.pos, 50, "asteroid_pieces");
	GameObject& objAsteroidPieces3 = Play::GetGameObject(id);
	Play::SetSprite(objAsteroidPieces3, "asteroid_pieces", 0.f);
	objAsteroidPieces3.frame = 3;
	objAsteroidPieces3.velocity = ASTEROID_PIECE_VELOCITY_1;

	if (gameState.gems > 0)
	{
		if (Play::RandomRoll(4) == 2)
			id = Play::CreateGameObject(TYPE_DIAMOND, { objSpecialAsteroid.oldPos.x - 100, objSpecialAsteroid.oldPos.y - 100 }, 50, "diamond");
		else 
			id = Play::CreateGameObject(TYPE_GEM, { objSpecialAsteroid.oldPos.x - 100, objSpecialAsteroid.oldPos.y - 100 }, 50, "gem");

		GameObject& objGem = Play::GetGameObject(id);
		objGem.rotation = Randomize(630, 0.01);
	}

	objSpecialAsteroid.pos = { DISPLAY_WIDTH + 100.f, -100.f };
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
	Play::GetGameObjectByType(TYPE_SHIELD).type = TYPE_DESTROYED;

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

	gameState.score = { 0 };
	gameState.paused = { false };
	gameState.collisionCount = { 0 };
	gameState.gems = { MAX_GEMS };
	gameState.playMode = { true };
	gameState.meteorCollisions = { 0 };
	gameState.powerActivated = { false };

	Play::CentreAllSpriteOrigins(); 

	gameState.agent8State =   STATE_ATTACHED;
	gameState.state = STATE_LOBBY;
}