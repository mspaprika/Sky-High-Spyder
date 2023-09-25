#define PLAY_IMPLEMENTATION
#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Play.h"

constexpr int DISPLAY_WIDTH = 1280;
constexpr int DISPLAY_HEIGHT = 720;
constexpr int DISPLAY_SCALE = 1;

void Draw();

enum GameObjectType
{
	TYPE_PLAYER,
	TYPE_ENEMY,
	TYPE_PLATFORM,
};

// The entry point for a PlayBuffer program
void MainGameEntry(PLAY_IGNORE_COMMAND_LINE)
{
	Play::CreateManager(DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_SCALE);
	Play::CentreAllSpriteOrigins();
	Play::LoadBackground("Data//Backgrounds//background.png");

	int id = Play::CreateGameObject(TYPE_PLAYER, { DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2 }, 20, "spr_pod_stand_right");
	GameObject& objPlayer = Play::GetGameObject(id);

	//objPlayer.rotation = Play::DegToRad(180);

}

// Called by PlayBuffer every frame (60 times a second!)
bool MainGameUpdate( float elapsedTime )
{
	Draw();

	return Play::KeyDown( VK_ESCAPE );
}


// Gets called once when the player quits the game 
int MainGameExit( void )
{
	Play::DestroyManager();
	return PLAY_OK;
}

void Draw()
{
	Play::ClearDrawingBuffer(Play::cWhite);
	Play::DrawBackground();

	GameObject& objplayer = Play::GetGameObjectByType(TYPE_PLAYER);
	Play::DrawObjectRotated(objplayer);
	Play::PresentDrawingBuffer();
}







