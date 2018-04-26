#define _USE_MATH_DEFINES
#include "GameState.h"
#include "Helper.h"

//Loads the required resources for the entities
void GameState::loadResources() {
	TextureID = LoadTexture(RESOURCE_FOLDER"spritesheet_rgba.png");
	map.Load(levelFILE);
	for (int i = 0; i < map.entities.size(); i++) {
		PlaceEntity(map.entities[i].type, map.entities[i].x * tileSize, map.entities[i].y * -tileSize);
	}
	start = player.Position;
	viewMatrix.Translate(-player.Position.x, -player.Position.y, 0);
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
	bgm = Mix_LoadMUS("Running.mp3");
	ghost = Mix_LoadWAV("ghost.wav");
	jump = Mix_LoadWAV("boing_spring.wav");
}

//Plays the background music
void GameState::playBackgroundMusic() const{
	Mix_VolumeMusic(20);
	Mix_PlayMusic(bgm, -1);
}

//Updates the GameState based on the time elapsed
void GameState::updateGameState(float elapsed) {
	player.Update(elapsed, map.mapData, solidTiles);
	for (int i = 0; i < entities.size(); ++i) {
		entities[i].Update(elapsed, map.mapData, solidTiles);
		//Reverses the movement of NPCs if there is collision against tiles
		if (entities[i].collidedLeft) {
			entities[i].acceleration.x = 0.0;
			entities[i].forward = false;
		}
		if (entities[i].collidedRight) {
			entities[i].acceleration.x = -0.0;
			entities[i].forward = true;
		}
	}
	//Player restarts when touches an enemy
	for (int i = 0; i < entities.size(); ++i) {
		if (player.SATCollidesWith(entities[i])) {
			player.Position = start;
			Mix_PlayChannel(-1, ghost, 0);
		}
	}
	//Player restarts when touches water
	int gridX, gridY;
	worldToTileCoordinates(player.Position.x, player.Position.y, &gridX, &gridY);
	if (map.mapData[gridY][gridX] == 11 || map.mapData[gridY][gridX] == 40) {
		player.Position = start;
	}
	//Translate the view matrix by the player's position
	viewMatrix.Identity();
	viewMatrix.Translate(-player.Position.x, -player.Position.y, 0);
}

void GameState::processKeys(const Uint8 * keys)
{
	if (keys[SDL_SCANCODE_A]) {
		player.acceleration.x = -1.2;
		player.forward = false;
	}
	if (keys[SDL_SCANCODE_D]) {
		player.acceleration.x = 1.2;
		player.forward = true;
	}
	if (keys[SDL_SCANCODE_SPACE]) {
		if (player.collidedBottom) {
			player.velocity.y = 2.5;
			Mix_PlayChannel(-1, jump, 0);
		}
		if (player.collidedLeft || player.collidedRight) {
			player.velocity.y = 1.25;
			player.velocity.x = player.forward ? -1.5 : 1.5;
		}
	}
	if (!(keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_SPACE])) {
		player.acceleration.x = 0.0;
		player.acceleration.y = 0.0;
	}
}

//Creates entities based on the string type
void GameState::PlaceEntity(std::string type, float x, float y)
{
	if (type == "Player") {
		player = Entity(x, y, std::vector<SheetSprite>({ createSheetSpriteBySpriteIndex(TextureID, 109, tileSize), createSheetSpriteBySpriteIndex(TextureID, 119, tileSize), createSheetSpriteBySpriteIndex(TextureID, 118, tileSize) }), Player, false);
	}
	else if (type == "Enemy") {
		Entity enemy = Entity(x, y, std::vector<SheetSprite>({createSheetSpriteBySpriteIndex(TextureID, 445, tileSize) }), Enemy, false);
		enemy.acceleration.x = 0.0;
		entities.emplace_back(enemy);
	}
}

//Draws the game state (tilemap and entities)
void GameState::Render(ShaderProgram & program)
{
	DrawLevel(program, TextureID, map, viewMatrix, 0.0, 0.0);
	player.Render(program, viewMatrix);
	for (int i = 0; i < entities.size(); ++i) {
		entities[i].Render(program, viewMatrix);
	}
}

// Destructor for GameState
GameState::~GameState() {
	Mix_FreeChunk(ghost);
	Mix_FreeChunk(jump);
	Mix_FreeMusic(bgm);
	Mix_CloseAudio();
}
