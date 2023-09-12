#define PLAY_IMPLEMENTATION
#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Play.h"

int DISPLAY_WIDTH = 1280;
int DISPLAY_HEIGHT = 720;
int DISPLAY_SCALE = 1;

const Vector2D METEOR_VELOCITY_DEFAULT = { 1.0f, 0.f }; 
const Vector2D ASTEROID_VELOCITY_DEFAULT = { 1.f, 0.f };
const Vector2D AGENT8_VELOCITY_DEFAULT = { 2.f, 0.f };

const float AGENT8_SPEED{ 6.5f };
const float SPECIAL_ASTEROID_SPEED{ 1.0f };
const float ASTEROID_SPEED{ 2.0f };

const Vector2D LASER_VELOCITY_DEFAULT = { 0.f, 0.f };
const Vector2D LASER_ACCELERATION = { 0.f, 0.f };

const Vector2D METEOR_AABB{ 50.f, 50.f };
const Vector2D ASTEROID_AABB{ 50.f, 50.f };
const Vector2D AGENT8_AABB{ 30.f, 30.f };

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
	int gameCount{ 1 };
	int totalScore{ 0 };
	int wins{ 0 };
	bool paused{ false };
	bool sound{ false };
	bool music{ false };
	int collisionCount{ 0 };

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
	TYPE_PARTICLE,
	TYPE_LASER,
	TYPE_SPECIAL,
	TYPE_DESTROYED,
};

void HandlePlayerControls();
void SoundControl();
float Randomize(int range, float multiplier);

void UpdateLasers();
void UpdateDestroyed();
void UpdateAgent8();
void UpdateAsteroids();
void UpdateMeteors();

void CreateGameObjects();
void LoopObject(GameObject& object);
bool IsColliding(const GameObject& object);
void AsteroidCollision();
void FloatDirectionObject(GameObject& obj, float speed);

void DrawGameStats();
void DrawGamePlay();
void DrawLobby();
void DrawSoundControl();

void DestroyAllItems();

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
			HandlePlayerControls();
			DrawGameStats();
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
		case STATE_GAME_OVER:
		{
			break;
		}
		case STATE_VICTORY:
		{
			break;
		}
	}

	UpdateDestroyed();
	SoundControl();
	
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
	Play::MoveSpriteOrigin("agent8_left", 0, 60);
	Play::MoveSpriteOrigin("agent8_right", 0, 60);

	GameObject& objAgent8 = Play::GetGameObject(idAgent8);

	int objSpecialAsteroidID = Play::CreateGameObject(TYPE_SPECIAL, { DISPLAY_WIDTH / 2, 400 }, 50, "asteroid");
	GameObject& objSpecialAsteroid = Play::GetGameObject(objSpecialAsteroidID);
	//objSpecialAsteroid.velocity = ASTEROID_VELOCITY_DEFAULT;
	objSpecialAsteroid.rotation = Randomize(630, 0.01);

	objAgent8.rotSpeed = objSpecialAsteroid.rotSpeed;

	for (int i = 0; i < 3; i++)
	{
		int id = Play::CreateGameObject(TYPE_ASTEROID, { Play::RandomRoll(DISPLAY_WIDTH), Play::RandomRoll(DISPLAY_WIDTH) }, 50, "asteroid");
		GameObject& objAsteroid = Play::GetGameObject(id);
		objAsteroid.rotation = Randomize(630, 0.01);
	}

	//int objMeteor = Play::CreateGameObject(TYPE_METEOR, { (DISPLAY_WIDTH / 2) - (i * 500), (DISPLAY_HEIGHT / 2) - (i * 500) }, 50, "meteor");
	//Play::GetGameObject(objMeteor).rotation = Randomize(630, 0.01);
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

	for (int id_meteor : vMeteors)
	{
		GameObject& obj_meteor = Play::GetGameObject(id_meteor);

		Play::DrawObject(obj_meteor);
	}

	std::vector<int> vAsteroids = Play::CollectGameObjectIDsByType(TYPE_ASTEROID);

	for (int id_asteroid : vAsteroids)
	{
		GameObject& obj_asteroid = Play::GetGameObject(id_asteroid);

		Play::DrawObjectRotated(obj_asteroid);
	}

	Play::DrawObjectRotated(objSpecialAsteroid);
	Play::DrawObjectRotated(objAgent8);


	Play::PresentDrawingBuffer();
}

void DrawLobby()
{
	Play::DrawFontText("64px", (gameState.paused) ? "PAUSED" : "WELCOME TO SKY HIGH SPYDER !", Point2D(DISPLAY_WIDTH / 2, 300), Play::CENTRE);
	Play::DrawFontText("64px", (gameState.paused) ? "PRESS SPACE TO CONTINUE" : "PRESS SPACE TO START AND SHIFT TO PAUSE" , { DISPLAY_WIDTH / 2, DISPLAY_HEIGHT - 300 }, Play::CENTRE);
	Play::DrawFontText("64px", (gameState.paused) ? "" : "ARROW KEYS TO MOVE UP AND DOWN AND SPACE TO FIRE", { DISPLAY_WIDTH / 2, DISPLAY_HEIGHT - 200 }, Play::CENTRE);
	Play::DrawFontText("64px", "F2 FOR SOUND || F3 FOR MUSIC", { DISPLAY_WIDTH / 2, DISPLAY_HEIGHT - 100 }, Play::CENTRE);

	Play::PresentDrawingBuffer();
}

void DrawGameStats()
{	
	GameObject& objAgent8 = Play::GetGameObjectByType(TYPE_AGENT8);
	Play::DrawFontText("132px", "SCORE: " + std::to_string(gameState.score), { DISPLAY_WIDTH / 2, 50 }, Play::CENTRE);
	Play::DrawFontText("64px", "Game: " + std::to_string(gameState.gameCount), { DISPLAY_WIDTH - 100, 50 }, Play::CENTRE);
	Play::DrawFontText("64px", "Wins: " + std::to_string(gameState.wins), { DISPLAY_WIDTH - 300, 50 }, Play::CENTRE);
	Play::DrawFontText("64px", "Total: " + std::to_string(gameState.totalScore), { 100, 50 }, Play::CENTRE);
	Play::DrawFontText("64px", "Collisions: " + std::to_string(gameState.collisionCount), { 300, 50 }, Play::CENTRE);
	Play::DrawFontText("64px", "Agent State: " + std::to_string(gameState.agent8State), { 300, 100 }, Play::CENTRE);
	//Play::DrawFontText("64px", "Agent Pos y: " + std::to_string(objAgent8.pos.y), { 300, 200 }, Play::CENTRE);
}

void HandlePlayerControls()
{
	GameObject& objAgent8 = Play::GetGameObjectByType(TYPE_AGENT8); 

	if (Play::KeyDown(VK_LEFT))
	{
		objAgent8.rotation -= 0.1f;
		Play::SetSprite( objAgent8, "agent8_left", 0.25f );
	}
	if (Play::KeyDown(VK_RIGHT))
	{
		objAgent8.rotation += 0.1f;
		Play::SetSprite( objAgent8, "agent8_right", 0.25f );
	}
	if (Play::KeyDown(VK_SPACE))
	{
		Play::SetSprite( objAgent8, "agent8_fly", 0.0f );
		gameState.agent8State = STATE_FLY;
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
		FloatDirectionObject(objAgent8, AGENT8_SPEED);
		AsteroidCollision();
		break;
	}
	case STATE_DEAD:
	{
		break;
	}
	}

	LoopObject(objAgent8);

	Play::UpdateGameObject(objAgent8);

}

void UpdateLasers()
{
	std::vector<int> vLasers = Play::CollectGameObjectIDsByType(TYPE_LASER);

	for (int id_laser : vLasers)
	{

	}
}

void UpdateMeteors()
{
	std::vector<int> vMeteors = Play::CollectGameObjectIDsByType(TYPE_METEOR);

	for (int id_meteor : vMeteors)
	{
		GameObject& obj_meteor = Play::GetGameObject(id_meteor);
		obj_meteor.velocity.y = std::clamp(obj_meteor.velocity.y, .0f, 2.0f);
		Play::UpdateGameObject(obj_meteor);
		LoopObject(obj_meteor);
	}
}

void UpdateAsteroids()
{
	std::vector<int> vAsteroids = Play::CollectGameObjectIDsByType(TYPE_ASTEROID);

	for (int id_asteroid : vAsteroids)
	{
		GameObject& objAsteroid = Play::GetGameObject(id_asteroid);
		FloatDirectionObject(objAsteroid, ASTEROID_SPEED);
		LoopObject(objAsteroid);
		Play::UpdateGameObject(objAsteroid);
	}

	GameObject& objSpecialAsteroid = Play::GetGameObjectByType(TYPE_SPECIAL);
	FloatDirectionObject(objSpecialAsteroid, SPECIAL_ASTEROID_SPEED);
	LoopObject(objSpecialAsteroid);
	Play::UpdateGameObject(objSpecialAsteroid);

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

void AsteroidCollision()
{
	GameObject& objSpecialAsteroid = Play::GetGameObjectByType(TYPE_SPECIAL);

	if (IsColliding(objSpecialAsteroid) && gameState.agent8State == STATE_FLY)
		gameState.agent8State = STATE_ATTACHED;

	std::vector<int> vAsteroids = Play::CollectGameObjectIDsByType(TYPE_ASTEROID);

	for (int id_asteroid : vAsteroids)
	{
		GameObject& objAsteroid = Play::GetGameObject(id_asteroid);

		if (gameState.agent8State == STATE_FLY && IsColliding(objAsteroid))
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

void DestroyAllItems()
{
	
}

void UpdateDestroyed()
{
	std::vector<int> vDead = Play::CollectGameObjectIDsByType(TYPE_DESTROYED);

	for (int id_dead : vDead)
	{
		Play::DestroyGameObject(id_dead);
	}
}