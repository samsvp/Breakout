#include <vector>

#include "game_object.hpp"

class GameLevel
{
public:
    GameLevel();
    std::vector<GameObject> bricks;
    // load level from text file
    void Load(const char* file, unsigned int levelWidth, unsigned int levelHeight);
    // render
    void Draw(SpriteRenderer &renderer);
    // check if level is completed
    bool IsCompleted();

private:
    // init level
    void Init(std::vector<std::vector<unsigned int>> tileData,
              unsigned int levelWidth, unsigned int levelHeight);
};