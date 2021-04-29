#include <string>
#include <fstream>
#include <sstream>
#include "../include/game_level.hpp"
#include "../include/resource_manager.hpp"

GameLevel::GameLevel()
{
    
}

void GameLevel::Load(const char* file, unsigned int levelWidth, unsigned int levelHeight)
{
    // clear level data
    this->bricks.clear();

    // load from file
    unsigned int tileCode;
    GameLevel level;
    std::string line;
    std::ifstream fstream(file);
    std::vector<std::vector<unsigned int>> tileData;
    if (fstream)
    {
        while (std::getline(fstream, line))
        {
            std::istringstream sstream(line);
            std::vector<unsigned int> row;
            while (sstream >> tileCode) // read each word
            {
                row.push_back(tileCode);
            }
            tileData.push_back(row);
        }
        if (tileData.size() > 0)
        {
            this->Init(tileData, levelWidth, levelHeight);
        }
    }
}

void GameLevel::Draw(SpriteRenderer &renderer)
{
    for (GameObject &tile : this->bricks)
    {
        if (!tile.Destroyed)
        {
            tile.Draw(renderer);
        }   
    }
}

bool GameLevel::IsCompleted()
{
    for (GameObject &tile : this->bricks)
    {
        if (!tile.IsSolid && !tile.Destroyed)
        {
            return false;
        }   
    }
    return true;
}

void GameLevel::Init(std::vector<std::vector<unsigned int>> tileData, 
                     unsigned int levelWidth, unsigned int levelHeight)
{
    // calculate dimensions
    unsigned int height = tileData.size();
    unsigned int width  = tileData[0].size();
    float unit_width    = levelWidth / static_cast<float>(width);
    float unit_height   = levelHeight / height;
    // initialize level tiles based on tileData		
    for (unsigned int y = 0; y < height; ++y)
    {
        for (unsigned int x = 0; x < width; ++x)
        {
            // check block type from level data (2D level array)
            if (tileData[y][x] == 1) // solid
            {
                glm::vec2 pos(unit_width * x, unit_height * y);
                glm::vec2 size(unit_width, unit_height);
                GameObject obj(pos, size, 
                    ResourceManager::GetTexture("block_solid"), 
                    glm::vec3(0.8f, 0.8f, 0.7f)
                );
                obj.IsSolid = true;
                this->bricks.push_back(obj);
            }
            else if (tileData[y][x] > 1)	
            {
                glm::vec3 color = glm::vec3(1.0f); // original: white
                if (tileData[y][x] == 2)
                    color = glm::vec3(0.2f, 0.6f, 1.0f);
                else if (tileData[y][x] == 3)
                    color = glm::vec3(0.0f, 0.7f, 0.0f);
                else if (tileData[y][x] == 4)
                    color = glm::vec3(0.8f, 0.8f, 0.4f);
                else if (tileData[y][x] == 5)
                    color = glm::vec3(1.0f, 0.5f, 0.0f);

                glm::vec2 pos(unit_width * x, unit_height * y);
                glm::vec2 size(unit_width, unit_height);
                this->bricks.push_back(
                    GameObject(pos, size, ResourceManager::GetTexture("block"), color)
                );
            }
        }
    }  
}