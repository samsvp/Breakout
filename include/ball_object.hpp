#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "game_object.hpp"
#include "texture.hpp"


class BallObject: public GameObject
{
public:
    BallObject();
    BallObject(glm::vec2 pos, float radius, glm::vec2 velocity, Texture2D sprite);
    
    float radius;
    bool stuck;
    bool sticky, passThrough;

    glm::vec2 Move(float dt, unsigned int window_width);
    void Reset(glm::vec2 postion, glm::vec2 velocity);
};