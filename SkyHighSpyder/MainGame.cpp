#define PLAY_IMPLEMENTATION
#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Play.h"

int DISPLAY_WIDTH = 1280;
int DISPLAY_HEIGHT = 720;
int DISPLAY_SCALE = 1;

const Vector2D METEOR_VELOCITY_DEFAULT = { -5, 0 };
const Vector2D METEOR_ACCELERATION = { 0, -0.5f }; 

const Vector2D ASTEROID_VELOCITY_DEFAULT = { 5, 0 };
const Vector2D ASTEROID_ACCELERATION = { 0, 0.5f }; 

const Vector2D AGENT8_VELOCITY_DEFAULT = { 2, 0 };
const Vector2D AGENT8_ACCELERATION = { 0, 0.1f };

enum Agent8State
{
	STATE_APPEAR = 0,
	STATE_FLY,
	STATE_ATTACHED,
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

	Agent8State agent8State = STATE_APPEAR;
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
	TYPE_DESTROYED,
};

void HandlePlayerControls();
void SoundControl();

void UpdateLasers();
void UpdateDestroyed();
void UpdateAgent8();
void UpdateAsteroids();
void UpdateMeteors();

void CreateGameObjects();

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
			if (Play::KeyPressed(VK_SPACE))
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
	int idAgent8 = Play::CreateGameObject(TYPE_AGENT8, { 115, 0 }, 50, "agent8");

	Play::GetGameObject(idAgent8).acceleration = AGENT8_ACCELERATION;
	Play::GetGameObject(idAgent8).velocity = AGENT8_VELOCITY_DEFAULT;

	int objAsteroid = Play::CreateGameObject(TYPE_ASTEROID, { DISPLAY_WIDTH / 2, 100 }, 50, "asteroid");

	Play::GetGameObject(objAsteroid).acceleration = ASTEROID_ACCELERATION;
	Play::GetGameObject(objAsteroid).velocity = ASTEROID_VELOCITY_DEFAULT;

	int objMeteor = Play::CreateGameObject(TYPE_METEOR, { DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2 }, 50, "meteor");

	Play::GetGameObject(objMeteor).acceleration = METEOR_ACCELERATION;
	Play::GetGameObject(objMeteor).velocity = METEOR_VELOCITY_DEFAULT;
}

void DrawSoundControl()
{
	Play::DrawFontText("64px", (gameState.sound) ? "SOUND: ON" : "SOUND: OFF", Point2D(100, 150), Play::CENTRE);
	Play::DrawFontText("64px", (gameState.music) ? "MUSIC: ON" : "MUSIC: OFF", Point2D(100, 200), Play::CENTRE);
}

void DrawGamePlay()
{
	GameObject& objAgent8 = Play::GetGameObjectByType(TYPE_AGENT8);
	GameObject& objAsteroid = Play::GetGameObjectByType(TYPE_ASTEROID);
	GameObject& objMeteor = Play::GetGameObjectByType(TYPE_METEOR);

	Play::DrawObject(objAgent8);
	Play::DrawObject(objAsteroid);
	Play::DrawObject(objMeteor);

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
	Play::DrawFontText("132px", "SCORE: " + std::to_string(gameState.score), { DISPLAY_WIDTH / 2, 50 }, Play::CENTRE);
	Play::DrawFontText("64px", "Game: " + std::to_string(gameState.gameCount), { DISPLAY_WIDTH - 100, 50 }, Play::CENTRE);
	Play::DrawFontText("64px", "Wins: " + std::to_string(gameState.wins), { DISPLAY_WIDTH - 300, 50 }, Play::CENTRE);
	Play::DrawFontText("64px", "Total: " + std::to_string(gameState.totalScore), { 100, 50 }, Play::CENTRE);
}

void HandlePlayerControls()
{
	GameObject& obj_agent8 = Play::GetGameObjectByType(TYPE_AGENT8); 

}


void UpdateLasers()
{
	std::vector<int> vLasers = Play::CollectGameObjectIDsByType(TYPE_LASER);

	for (int id_laser : vLasers)
	{
		
	}
}

void UpdateAgent8()
{
	GameObject& obj_agent8 = Play::GetGameObjectByType(TYPE_AGENT8);

	Play::UpdateGameObject(obj_agent8);
}

void UpdateMeteors()
{
	std::vector<int> vMeteors = Play::CollectGameObjectIDsByType(TYPE_METEOR);

	for (int id_meteor : vMeteors)
	{
		GameObject& obj_meteor = Play::GetGameObject(id_meteor);

		Play::UpdateGameObject(obj_meteor);
	}
}

void UpdateAsteroids()
{
	std::vector<int> vAsteroids = Play::CollectGameObjectIDsByType(TYPE_ASTEROID);

	for (int id_asteroid : vAsteroids)
	{
		GameObject& obj_asteroid = Play::GetGameObject(id_asteroid);

		Play::UpdateGameObject(obj_asteroid);
	}
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