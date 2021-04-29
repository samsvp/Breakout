#pragma once

#include <vector>
#include <tuple>
#include "power_up.hpp"
#include "game_level.hpp"
#include "ball_object.hpp"

// Represents the current state of the game
enum GameState {
    GAME_ACTIVE,
    GAME_MENU,
    GAME_WIN
};

enum Direction {
	UP,
	RIGHT,
	DOWN,
	LEFT
};

typedef std::tuple<bool, Direction, glm::vec2> Collision; 

// Game holds all game-related state and functionality.
// Combines all game-related data into a single class for
// easy access to each of the components and manageability.
class Game
{
public:
    // constructor/destructor
    Game(unsigned int width, unsigned int height);
    ~Game();
    // game state
    GameState state;	
    bool keys[1024];
    bool keysProcessed[1024];
    unsigned int width, height;
    unsigned int lives;
    // game levels
    std::vector<GameLevel> levels;
    unsigned int level;
    // power ups
    std::vector<PowerUp> powerUps;
    void SpawnPowerUp(GameObject &block);
    void UpdatePowerUps(float dt);
    // initialize game state (load all shaders/textures/levels)
    void Init();
    // game loop
    void ProcessInput(float dt);
    void Update(float dt);
    void Render();
    // reset
    void ResetLevel();
    void ResetPlayer();
    // Collisions
    Direction VectorDirection(glm::vec2 target);
    bool CheckCollision(GameObject &one, GameObject &two);
    Collision CheckCollision(BallObject &one, GameObject &two);
    void DoCollisions();
};
