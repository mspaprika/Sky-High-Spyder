#define PLAY_IMPLEMENTATION
#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Play.h"

int DISPLAY_WIDTH = 1280;
int DISPLAY_HEIGHT = 720;
int DISPLAY_SCALE = 1;

enum Agent8State
{
	STATE_APPEAR = 0,
	STATE_HALT,
	STATE_PLAY,
	STATE_DEAD,
	STATE_DESTROY,
	STATE_WON,
};

struct GameState
{
	//float timer = 0;
	//int spriteId = 0;

	int score = 0;
	Agent8State agent8State = STATE_APPEAR;

	int gameCount = 1;
	int totalScore = 0;
	int wins = 0;
	int diamonds = 0;
};

GameState gameState;

enum gameObjectType
{
	TYPE_NULL = -1,
	TYPE_AGENT8,
	TYPE_FAN,
	TYPE_TOOL,
	TYPE_COIN,
	TYPE_STAR,
	TYPE_LASER,
	TYPE_DESTROYED,
	TYPE_DIAMOND,
};

void HandlePlayerControls();
void UpdateFan();
void UpdateTools();
void UpdateCoinsAndStars();
void UpdateLasers();
void UpdateDestroyed();
void UpdateAgent8();

void Destroy();
void UpdateDiamonds();
void DestroyAllItems();

// The entry point for a PlayBuffer program
void MainGameEntry( PLAY_IGNORE_COMMAND_LINE )
{
	Play::CreateManager( DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_SCALE );
	Play::CentreAllSpriteOrigins();
	Play::LoadBackground("Data//Backgrounds//background.png");
	Play::StartAudioLoop("music");
	Play::CreateGameObject(TYPE_AGENT8, { 115, 0 }, 50, "agent8");

	int id_fan = Play::CreateGameObject(TYPE_FAN, { 1140, 217 }, 0, "fan");
	Play::GetGameObject(id_fan).velocity = { 0, 3 };
	Play::GetGameObject(id_fan).animSpeed = 1.0f;
}

// Called by PlayBuffer every frame (60 times a second!)
bool MainGameUpdate( float elapsedTime )
{
	/*
	gameState.timer += elapsedTime;
	Play::ClearDrawingBuffer( Play::cOrange );

	Play::DrawDebugText( { DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2 },
		Play::GetSpriteName(gameState.spriteId),
		Play::cWhite );

	Play::DrawSprite(gameState.spriteId, Play::GetMousePos(), gameState.timer);

	if (Play::KeyPressed(VK_SPACE))
	{
		gameState.spriteId++;
	}
	*/

	Play::DrawBackground();

	//HandlePlayerControls();
	UpdateAgent8();

	UpdateFan();
	UpdateTools();
	UpdateCoinsAndStars();
	UpdateLasers();
	UpdateDestroyed();

	UpdateDiamonds();

	Play::DrawFontText("64px", "ARROW KEYS TO MOVE UP AND DOWN AND SPACE TO FIRE",
		{ DISPLAY_WIDTH / 2, DISPLAY_HEIGHT - 80 },
		Play::CENTRE);
	Play::DrawFontText("64px", "LEFT TO FIRE UP, RIGHT TO FIRE DOWN, TAB FOR DIAGONAL, SHIFT FOR SHIELD",
		{ DISPLAY_WIDTH / 2, DISPLAY_HEIGHT - 30 },
		Play::CENTRE);
	Play::DrawFontText("132px", "SCORE: " + std::to_string(gameState.score),
		{ DISPLAY_WIDTH / 2, 50 },
		Play::CENTRE);
	Play::DrawFontText("64px", "Game: " + std::to_string(gameState.gameCount),
		{ DISPLAY_WIDTH - 100, 50 },
		Play::CENTRE);
	Play::DrawFontText("64px", "Wins: " + std::to_string(gameState.wins),
		{ DISPLAY_WIDTH - 300, 50 },
		Play::CENTRE);
	Play::DrawFontText("64px", "Total: " + std::to_string(gameState.totalScore),
		{ 100, 50 },
		Play::CENTRE);
	Play::DrawFontText("64px", "Gems: " + std::to_string(gameState.diamonds),
		{ 300, 50 },
		Play::CENTRE);

	Play::PresentDrawingBuffer();
	return Play::KeyDown( VK_ESCAPE );
}


// Gets called once when the player quits the game 
int MainGameExit( void )
{
	Play::DestroyManager();
	return PLAY_OK;
}

void HandlePlayerControls()
{
	GameObject& obj_agent8 = Play::GetGameObjectByType(TYPE_AGENT8);
	if (Play::KeyDown(VK_UP))
	{
		obj_agent8.velocity = { 0, -4 };
		Play::SetSprite(obj_agent8, "agent8_climb", 0.25f);
	}
	else if (Play::KeyDown(VK_DOWN))
	{
		obj_agent8.velocity = { 0, 2 };
		Play::SetSprite(obj_agent8, "agent8_fall", 0);
	}
	
	else
	{
		if (obj_agent8.velocity.y > 5)
		{
			gameState.agent8State = STATE_HALT;	
			Play::SetSprite(obj_agent8, "agent8_halt", 0.333f);
			obj_agent8.acceleration = { 0, 0 };
		}
		else
		{
			Play::SetSprite(obj_agent8, "agent8_hang", 0.02f);
			obj_agent8.velocity *= 0.5f;
			obj_agent8.acceleration = { 0, 0 };
		}
	}

	if (Play::KeyPressed(VK_SPACE))
	{
		Vector2D firePos = obj_agent8.pos + Vector2D(155, -75);
		int id = Play::CreateGameObject(TYPE_LASER, firePos, 50, "laser");
		Play::GetGameObject(id).velocity = { 52, 0 };
		Play::PlayAudio("shoot");
	}

	if (Play::KeyPressed(VK_SHIFT))
	{
		gameState.agent8State = STATE_DESTROY;;
	}

	if (Play::KeyPressed(VK_LEFT))
	{
		Vector2D firePos = obj_agent8.pos + Vector2D(10, -75);
		int id = Play::CreateGameObject(TYPE_LASER, firePos, 50, "star");
		Play::GetGameObject(id).velocity = { 0, -52 };
		Play::PlayAudio("shoot");
	}

	if (Play::KeyPressed(VK_RIGHT))
	{
		Vector2D firePos = obj_agent8.pos + Vector2D(10, -75);
		int id = Play::CreateGameObject(TYPE_LASER, firePos, 50, "star");
		Play::GetGameObject(id).velocity = { 0, 52 };
		Play::PlayAudio("shoot");
	}

	if (Play::KeyPressed(VK_TAB))
	{
		Vector2D firePos = obj_agent8.pos + Vector2D(10, -75);
		int id = Play::CreateGameObject(TYPE_LASER, firePos, 50, "star");
		Play::GetGameObject(id).velocity = { 50, -20 };
		int id2 = Play::CreateGameObject(TYPE_LASER, firePos, 50, "star");
		Play::GetGameObject(id2).velocity = { 50, 20 };
		Play::PlayAudio("shoot");
	}
}

void UpdateFan()
{
	GameObject& obj_fan = Play::GetGameObjectByType(TYPE_FAN);

	if (gameState.agent8State == STATE_PLAY) {

		if (Play::RandomRoll(100) == 50)
		{
			int id = Play::CreateGameObject(TYPE_TOOL, obj_fan.pos, 50, "driver");
			GameObject& obj_tool = Play::GetGameObject(id);
			obj_tool.velocity = Point2f(-8, Play::RandomRollRange(-1, 1) * 6);

			if (Play::RandomRoll(2) == 1)
			{
				Play::SetSprite(obj_tool, "spanner", 0);
				obj_tool.radius = 100;
				obj_tool.velocity.x = -2;
				obj_tool.rotSpeed = 0.1f;
			}
			Play::PlayAudio("tool");
		}

		if (Play::RandomRoll(150) == 1)
		{
			int id = Play::CreateGameObject(TYPE_COIN, obj_fan.pos, 40, "coin");
			GameObject& obj_coin = Play::GetGameObject(id);
			obj_coin.velocity = { -3, 0 };
			obj_coin.rotSpeed = 0.1f;
		}

		if (Play::RandomRoll(200) == 1)
		{
			int id = Play::CreateGameObject(TYPE_DIAMOND, obj_fan.pos, 40, "diamond");
			GameObject& obj_diamond = Play::GetGameObject(id);
			obj_diamond.velocity = { -3, 0 };
			obj_diamond.rotSpeed = 0.1f;
		}
	}

	Play::UpdateGameObject(obj_fan);

	if (Play::IsLeavingDisplayArea(obj_fan))
	{
		obj_fan.pos = obj_fan.oldPos;
		obj_fan.velocity.y *= -1;
	}

	Play::DrawObject(obj_fan);
}

void UpdateTools()
{
	GameObject& obj_agent8 = Play::GetGameObjectByType(TYPE_AGENT8);
	std::vector<int> vTools = Play::CollectGameObjectIDsByType(TYPE_TOOL);

	for (int id : vTools)
	{
		GameObject& obj_tool = Play::GetGameObject(id);

		if (gameState.agent8State != STATE_DEAD && Play::IsColliding(obj_tool, obj_agent8))
		{
			Play::StopAudioLoop("music");
			Play::PlayAudio("die");
			gameState.agent8State = STATE_DEAD;
			//obj_agent8.pos = { -100, 100 };
			
		}
		Play::UpdateGameObject(obj_tool);

		if (Play::IsLeavingDisplayArea(obj_tool, Play::VERTICAL))
		{
			obj_tool.pos = obj_tool.oldPos;
			obj_tool.velocity.y *= -1;
		}
		Play::DrawObjectRotated(obj_tool);

		if (!Play::IsVisible(obj_tool))
		{
			Play::DestroyGameObject(id);
		}
	}
}

void UpdateCoinsAndStars()
{
	GameObject& obj_agent8 = Play::GetGameObjectByType(TYPE_AGENT8);
	std::vector<int> vCoins = Play::CollectGameObjectIDsByType(TYPE_COIN);

	for (int id_coin : vCoins)
	{
		GameObject& obj_coin = Play::GetGameObject(id_coin);
		bool hasCollided = false;

		if (Play::IsColliding(obj_coin, obj_agent8) && gameState.agent8State != STATE_DEAD && gameState.agent8State != STATE_WON)
		{
			for (float rad{ 0.25f }; rad < 2.0f; rad += 0.5f)
			{
				int id = Play::CreateGameObject(TYPE_STAR, obj_agent8.pos, 0, "star");
				GameObject& obj_star = Play::GetGameObject(id);
				obj_star.rotSpeed = 0.1f;
				obj_star.acceleration = { 0.0f, 0.5f };
				Play::SetGameObjectDirection(obj_star, 16, rad * PLAY_PI);
			}

			hasCollided = true;
			gameState.score += 500;
			Play::PlayAudio("collect");

		}

		Play::UpdateGameObject(obj_coin);
		Play::DrawObjectRotated(obj_coin);

		if (!Play::IsVisible(obj_coin) || hasCollided)
		{
			Play::DestroyGameObject(id_coin);
		}
	}

	std::vector<int> vStars = Play::CollectGameObjectIDsByType(TYPE_STAR);

	for (int id_star : vStars)
	{
		GameObject& obj_star = Play::GetGameObject(id_star);

		Play::UpdateGameObject(obj_star);
		Play::DrawObjectRotated(obj_star);

		if (Play::IsVisible(obj_star))
		{
			Play::DestroyGameObject(id_star);
		}
	}
}

void UpdateLasers()
{
	std::vector<int> vLasers = Play::CollectGameObjectIDsByType(TYPE_LASER);
	std::vector<int> vTools = Play::CollectGameObjectIDsByType(TYPE_TOOL);
	std::vector<int> vCoins = Play::CollectGameObjectIDsByType(TYPE_COIN);
	std::vector<int> vDiamonds = Play::CollectGameObjectIDsByType(TYPE_DIAMOND);

	for (int id_laser : vLasers)
	{
		GameObject& obj_laser = Play::GetGameObject(id_laser);
		bool hasCollided = false;

		for (int id_tool : vTools)
		{
			GameObject& obj_tool = Play::GetGameObject(id_tool);

			if (Play::IsColliding(obj_laser, obj_tool))
			{
				hasCollided = true;
				obj_tool.type = TYPE_DESTROYED;
				gameState.score += 100;
			}
		}

		for (int id_diamond : vDiamonds)
		{
			GameObject& obj_diamond = Play::GetGameObject(id_diamond);

			if (Play::IsColliding(obj_laser, obj_diamond))
			{
				hasCollided = true;
				obj_diamond.type = TYPE_DESTROYED;
				Play::PlayAudio("error");
			}
		}

		for (int id_coin : vCoins)
		{
			GameObject& obj_coin = Play::GetGameObject(id_coin);

			if (Play::IsColliding(obj_laser, obj_coin))
			{
				hasCollided = true;
				obj_coin.type = TYPE_DESTROYED;
				Play::PlayAudio("error");
				gameState.score -= 300;
			}
		}

		if (gameState.score < 0)
		{
			gameState.score = 0;
		}

		Play::UpdateGameObject(obj_laser);
		Play::DrawObject(obj_laser);

		if (!Play::IsVisible(obj_laser) || hasCollided)
		{
			Play::DestroyGameObject(id_laser);
		}
	}
}

void UpdateDestroyed()
{
	std::vector<int> vDead = Play::CollectGameObjectIDsByType(TYPE_DESTROYED);

	for (int id_dead : vDead)
	{
		GameObject& obj_dead = Play::GetGameObject(id_dead);
		obj_dead.animSpeed = 0.2f;
		Play::UpdateGameObject(obj_dead);

		if (obj_dead.frame % 2)
		{
			Play::DrawObjectRotated(obj_dead, (10 - obj_dead.frame) / 10.0f);
		}

		if (!Play::IsVisible(obj_dead) || obj_dead.frame >= 10)
		{
			Play::DestroyGameObject(id_dead);
		}
	}
}

void UpdateAgent8()
{
	GameObject& obj_agent8 = Play::GetGameObjectByType(TYPE_AGENT8);

	switch (gameState.agent8State)
	{
	case STATE_APPEAR:
		obj_agent8.velocity = { 0, 12 };
		obj_agent8.acceleration = { 0, 0.9f };
		Play::SetSprite(obj_agent8, "agent8_fall", 0);
		obj_agent8.rotation = 0;

		if (obj_agent8.pos.y >= DISPLAY_HEIGHT / 3)
		{
			gameState.agent8State = STATE_PLAY;
		}
		break;

	case STATE_HALT:
		obj_agent8.velocity *= 0.9f;
		obj_agent8.rotation = 0.2f;

		if (Play::IsAnimationComplete(obj_agent8))
		{
			gameState.agent8State = STATE_PLAY;
		}
		break;

	case STATE_PLAY:
		HandlePlayerControls();
		break;
	
	case STATE_DESTROY:
		Destroy();
		break;

	case STATE_WON:

		obj_agent8.acceleration = { 0, -0.1f };

		Play::DrawFontText("132px", "VICTORY !!!",
			{ DISPLAY_WIDTH / 2, 200 },
			Play::CENTRE);

		Play::DrawFontText("64px", "Press SPACE for a new game",
			{ DISPLAY_WIDTH / 2, 300 },
			Play::CENTRE);
		
		DestroyAllItems();

		if (Play::KeyDown(VK_SPACE) == true)
		{
			gameState.agent8State = STATE_APPEAR;
			obj_agent8.pos = { 115, 0 };
			obj_agent8.velocity = { 0, 0 };
			obj_agent8.frame = 0;
			Play::StartAudioLoop("music");

			gameState.score = 0;
			gameState.diamonds = 0;
			gameState.gameCount++;
		}
		break;

	case STATE_DEAD:
		obj_agent8.acceleration = { 0.3f, 0.05f };
		obj_agent8.rotation += 0.5f;
		
		Play::DrawFontText("132px", "GAME OVER" ,
			{ DISPLAY_WIDTH / 2, 200 },
			Play::CENTRE);
		
		Play::DrawFontText("64px", "Press SPACE for a new game",
			{ DISPLAY_WIDTH / 2, 300 },
			Play::CENTRE);

		gameState.score = 0;
		gameState.diamonds = 0;

		DestroyAllItems(); 

		if (Play::KeyDown(VK_SPACE) == true)
		{
			gameState.agent8State = STATE_APPEAR;
			obj_agent8.pos = { 115, 0 };
			obj_agent8.velocity = { 0, 0 };
			obj_agent8.frame = 0;
			Play::StartAudioLoop("music");
			
			gameState.score = 0;
			gameState.gameCount++;
		}
		break;
	}

	Play::UpdateGameObject(obj_agent8);

	if (Play::IsLeavingDisplayArea(obj_agent8) && gameState.agent8State != STATE_DEAD)
	{
		obj_agent8.pos = obj_agent8.oldPos;
	}

	Play::DrawLine({ obj_agent8.pos.x, 0 }, obj_agent8.pos, Play::cYellow);
	Play::DrawObjectRotated(obj_agent8);
}

void Destroy()
{
	GameObject& obj_agent8 = Play::GetGameObjectByType(TYPE_AGENT8);
	std::vector<int> vTools = Play::CollectGameObjectIDsByType(TYPE_TOOL);

	for (int id_tool : vTools)
	{
		GameObject& obj_tool = Play::GetGameObject(id_tool);

		if (Play::IsColliding(obj_agent8, obj_tool))
		{
			obj_tool.type = TYPE_DESTROYED;
			gameState.agent8State = STATE_PLAY;
		}
		if (Play::KeyPressed(VK_UP) || Play::KeyPressed(VK_DOWN) || Play::KeyPressed(VK_SPACE))
		{
			gameState.agent8State = STATE_PLAY;
		}
	}
}

void DestroyAllItems()
{
	for (int id_tool : Play::CollectGameObjectIDsByType(TYPE_TOOL))
	{
		Play::GetGameObject(id_tool).type = TYPE_DESTROYED;
	}

	for (int id_coin : Play::CollectGameObjectIDsByType(TYPE_COIN))
	{
		Play::GetGameObject(id_coin).type = TYPE_DESTROYED;
	}

	for (int id_diamond : Play::CollectGameObjectIDsByType(TYPE_DIAMOND))
	{
		Play::GetGameObject(id_diamond).type = TYPE_DESTROYED;
	}
}

void UpdateDiamonds()
{
	GameObject& obj_agent8 = Play::GetGameObjectByType(TYPE_AGENT8);
	std::vector<int> vDiamonds = Play::CollectGameObjectIDsByType(TYPE_DIAMOND);

	for (int id_diamond : vDiamonds)
	{
		GameObject& obj_diamond = Play::GetGameObject(id_diamond);
		bool hasCollided = false;

		if (Play::IsColliding(obj_diamond, obj_agent8) && gameState.agent8State != STATE_DEAD  && gameState.agent8State != STATE_WON)
		{
			for (float rad{ 0.25f }; rad < 2.0f; rad += 0.5f)
			{
				int id = Play::CreateGameObject(TYPE_STAR, obj_agent8.pos, 0, "star");
				GameObject& obj_star = Play::GetGameObject(id);
				obj_star.rotSpeed = 0.1f;
				obj_star.acceleration = { 0.0f, 0.5f };
				Play::SetGameObjectDirection(obj_star, 16, rad * PLAY_PI);
			}

			hasCollided = true;
			Play::PlayAudio("collect");
			gameState.diamonds += 1;

			if (gameState.diamonds == 5)
			{
				gameState.agent8State = STATE_WON;
				gameState.totalScore += gameState.score;
				gameState.wins++;
			}
		}

		Play::UpdateGameObject(obj_diamond);
		Play::DrawObjectRotated(obj_diamond);

		if (!Play::IsVisible(obj_diamond) || hasCollided)
		{
			Play::DestroyGameObject(id_diamond);
		}
	}

	std::vector<int> vStars = Play::CollectGameObjectIDsByType(TYPE_STAR);

	for (int id_star : vStars)
	{
		GameObject& obj_star = Play::GetGameObject(id_star);

		Play::UpdateGameObject(obj_star);
		Play::DrawObjectRotated(obj_star);

		if (Play::IsVisible(obj_star))
		{
			Play::DestroyGameObject(id_star);
		}
	}
}
