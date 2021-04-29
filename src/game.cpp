#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <SFML/Audio.hpp>
#include <sstream>
#include <iostream>
#include <algorithm>
#include "../include/game.hpp"
#include "../include/ball_object.hpp"
#include "../include/text_renderer.hpp"
#include "../include/post_processor.hpp"
#include "../include/sprite_renderer.hpp"
#include "../include/resource_manager.hpp"
#include "../include/particle_generator.hpp"


// player
const float PLAYER_VELOCITY(500.0f);
const glm::vec2 PLAYER_SIZE(100.0f, 20.0f);

// ball
const float BALL_RADIUS = 12.5f;
const glm::vec2 INITIAL_BALL_VELOCITY(100.0f, -350.0f);

GameObject *player;
BallObject *ball;
ParticleGenerator *particles;

// text
TextRenderer *text;

// music
sf::Music gameMusic;
sf::Music bleepBlockMusic;
sf::Music powerupMusic;
sf::Music solidMusic;
sf::Music bleepPaddleMusic;

// post processing
PostProcessor *effects;
float shakeTime = 0.0f; 

// Game-related State data
SpriteRenderer *renderer;

Game::Game(unsigned int width, unsigned int height) 
    : state(GAME_MENU), keys(), width(width), height(height)
{ 

}

Game::~Game()
{
    delete ball;
    delete text;
    delete player;
    delete effects;
    delete renderer;
    delete particles;
}


bool LoadMusic(sf::Music &music, std::string path)
{
    if (!music.openFromFile(path))
    {
        std::cout << ("could not open file " + path) << std::endl; // error
        return false;
    }
    return true;
}


void Game::Init()
{
    this->lives = 3;
    // load shaders
    ResourceManager::LoadShader("shaders/vertex/transform.vert", 
                                "shaders/fragment/fragment.frag",
                                nullptr, "sprite");
    ResourceManager::LoadShader("shaders/vertex/particle.vert", 
                                "shaders/fragment/particle.frag", 
                                nullptr, "particle"); 
    ResourceManager::LoadShader("shaders/vertex/post.vert", 
                                "shaders/fragment/post.frag", 
                                nullptr, "postprocessing");
    // configure music
    LoadMusic(solidMusic, "audios/solid.ogg");
    LoadMusic(gameMusic, "audios/breakout.ogg");
    LoadMusic(powerupMusic, "audios/powerup.ogg");
    LoadMusic(bleepBlockMusic, "audios/bleep_block.ogg");
    LoadMusic(bleepPaddleMusic, "audios/bleep_paddle.ogg");
    // play level music
    gameMusic.play();
    gameMusic.setLoop(true);
    // configure shaders
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(this->width), 
        static_cast<float>(this->height), 0.0f, -1.0f, 1.0f);
    ResourceManager::GetShader("sprite").Use().SetInteger("image", 0);
    ResourceManager::GetShader("sprite").SetMatrix4("projection", projection);
    // set render-specific controls
    Shader shader = ResourceManager::GetShader("sprite");
    particles = new ParticleGenerator(
        ResourceManager::GetShader("particle"), ResourceManager::GetTexture("particle"), 500
    );
    renderer = new SpriteRenderer(shader);
    effects = new PostProcessor(
        ResourceManager::GetShader("postprocessing"), this->width, this->height
    );
    // load textures
    ResourceManager::LoadTexture("imgs/awesomeface.png", true, "face");
    ResourceManager::LoadTexture("imgs/background.jpg", false, "background");
    ResourceManager::LoadTexture("imgs/block.png", false, "block");
    ResourceManager::LoadTexture("imgs/block_solid.png", false, "block_solid");
    ResourceManager::LoadTexture("imgs/paddle.png", true, "paddle");
    ResourceManager::LoadTexture("imgs/particle.png", true, "particle"); 
    ResourceManager::LoadTexture("imgs/powerup_chaos.png", true, "chaos"); 
    ResourceManager::LoadTexture("imgs/powerup_confuse.png", true, "confuse"); 
    ResourceManager::LoadTexture("imgs/powerup_increase.png", true, "increase"); 
    ResourceManager::LoadTexture("imgs/powerup_passthrough.png", true, "passthrough");
    ResourceManager::LoadTexture("imgs/powerup_speed.png", true, "speed");
    ResourceManager::LoadTexture("imgs/powerup_sticky.png", true, "sticky"); 
    ResourceManager::GetShader("particle").Use().SetMatrix4("projection", projection);
    // load levels
    GameLevel one; one.Load("levels/one.lvl", this->width, this->height / 2);
    GameLevel two; two.Load("levels/two.lvl", this->width, this->height / 2);
    GameLevel three; three.Load("levels/three.lvl", this->width, this->height / 2);
    GameLevel four; four.Load("levels/four.lvl", this->width, this->height / 2);
    this->levels.push_back(one);
    this->levels.push_back(two);
    this->levels.push_back(three);
    this->levels.push_back(four);
    this->level = 0;
    // load player
    glm::vec2 playerPos = glm::vec2(
        (this->width - PLAYER_SIZE.x) / 2.0f, this->height - PLAYER_SIZE.y
    );
    player = new GameObject(playerPos, PLAYER_SIZE, ResourceManager::GetTexture("paddle"));

    // load ball
    glm::vec2 ballPos = playerPos + glm::vec2(PLAYER_SIZE.x / 2.0f - BALL_RADIUS, 
                                              -BALL_RADIUS * 2.0f);
    ball = new BallObject(ballPos, BALL_RADIUS, INITIAL_BALL_VELOCITY,
        ResourceManager::GetTexture("face"));
    
    // text
    text = new TextRenderer(this->width, this->height);
    text->Load("fonts/OCRAEXT.TTF", 24);
}


void Game::Update(float dt)
{
    ball->Move(dt, this->width);
    // check for collisions
    this->DoCollisions();
    // update particles
    particles->Update(dt, *ball, 2, glm::vec2(ball->radius / 2.0f));
    // update power ups
    UpdatePowerUps(dt);
    if (ball->Position.y >= this->height) // did ball reach bottom edge?
    {
        if (--this->lives == 0)
        {
            this->ResetLevel();
            this->state = GAME_MENU;
        }
        this->ResetPlayer();
    }

    if (shakeTime > 0.0f)
    {
        shakeTime -= dt;
        if (shakeTime <= 0.0f) effects->Shake = false;
    }

    if (this->state == GAME_ACTIVE && this->levels[this->level].IsCompleted())
    {
        this->ResetLevel();
        this->ResetPlayer();
        effects->Chaos = true;
        this->state = GAME_WIN;
    }
}


void Game::ProcessInput(float dt)
{
    if (this->state == GAME_MENU)
    {
        if (this->keys[GLFW_KEY_ENTER] && !this->keysProcessed[GLFW_KEY_ENTER])
        {
            this->state = GAME_ACTIVE;
            this->keysProcessed[GLFW_KEY_ENTER] = true;
        }
        if (this->keys[GLFW_KEY_W] && !this->keysProcessed[GLFW_KEY_W])
        {
            this->level = (this->level + 1) % 4;
            this->keysProcessed[GLFW_KEY_W] = true;
        }
        if (this->keys[GLFW_KEY_S] && !this->keysProcessed[GLFW_KEY_S])
        {
            if (this->level > 0) --this->level;
            else this->level = 3;
            this->keysProcessed[GLFW_KEY_S] = true;
        }
    }

    if (this->state == GAME_ACTIVE)
    {
        float velocity = PLAYER_VELOCITY * dt;
        // move
        if (this->keys[GLFW_KEY_A])
        {
            if (player->Position.x >= 0.0f)
            {
                player->Position.x -= velocity;
                if (ball->stuck)
                    ball->Position.x -= velocity;
            }
        }
        if (this->keys[GLFW_KEY_D])
        {
            if (player->Position.x <= this->width - player->Size.x)
            {
                player->Position.x += velocity;
                if (ball->stuck)
                    ball->Position.x += velocity;
            }
        }
        if (this->keys[GLFW_KEY_SPACE])
        {
            ball->stuck = false;
        }
    }

    if (this->state == GAME_WIN && this->keys[GLFW_KEY_ENTER])
    {
        this->keysProcessed[GLFW_KEY_ENTER] = true;
        effects->Chaos = false;
        this->state = GAME_MENU;
    }
}


void Game::Render()
{
    if (this->state == GAME_ACTIVE || this->state == GAME_MENU)
    {
        effects->BeginRender();
        // draw background
        Texture2D background = ResourceManager::GetTexture("background");
        renderer->DrawSprite(background, glm::vec2(0.0f, 0.0f), 
            glm::vec2(this->width, this->height), 0.0f
        );
        // draw level
        this->levels[this->level].Draw(*renderer);
        // draw player
        player->Draw(*renderer);
        // draw particles	
        particles->Draw();
        // draw power ups
        for (PowerUp &powerUp : powerUps)
        {
            if (!powerUp.Destroyed) powerUp.Draw(*renderer);
        }
        // draw ball
        ball->Draw(*renderer);
        effects->EndRender();
        effects->Render(glfwGetTime());
        // draw text
        std::stringstream ss;
        ss << this->lives;
        text->RenderText("Lives: " + ss.str(), 5.0f, 5.0f, 1.0f);
    }

    if (this->state == GAME_MENU)
    {
        text->RenderText("Press ENTER to start", 250.0f, this->height / 2.0f, 1.0f);
        text->RenderText("Press W or S to select level", 245.0f, this->height / 2 + 20.0f, 0.75f);
    }

    if (this->state == GAME_WIN)
    {
        text->RenderText(
            "You WIN!!!", 320.0, this->height / 2 -20.0f, 1.0, glm::vec3(0.0f, 1.0f, 0.0f)
        );
        text->RenderText(
            "Press ENTER to retry or ESC to quit", 130, this->height / 2.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f)
        );
    }
}


void Game::ResetLevel()
{
    if (this->level == 0)
        this->levels[0].Load("levels/one.lvl", this->width, this->height / 2);
    else if (this->level == 1)
        this->levels[1].Load("levels/two.lvl", this->width, this->height / 2);
    else if (this->level == 2)
        this->levels[2].Load("levels/three.lvl", this->width, this->height / 2);
    else if (this->level == 3)
        this->levels[3].Load("levels/four.lvl", this->width, this->height / 2);
    
    this->lives = 3;
}


void Game::ResetPlayer()
{
    // reset player/ball stats
    player->Size = PLAYER_SIZE;
    player->Position = glm::vec2(this->width / 2.0f - PLAYER_SIZE.x / 2.0f, 
        this->height - PLAYER_SIZE.y);
    ball->Reset(player->Position + glm::vec2(PLAYER_SIZE.x / 2.0f - BALL_RADIUS,
         -(BALL_RADIUS * 2.0f)), INITIAL_BALL_VELOCITY);
}


bool ShouldSpawn(unsigned int chance)
{
    unsigned int random = rand() % chance;
    return random == 0;
}

void Game::SpawnPowerUp(GameObject &block)
{
    // duration of 0 means infinity
    if (ShouldSpawn(20)) // 1 in 75 chance
        this->powerUps.push_back(
             PowerUp("speed", glm::vec3(0.5f, 0.5f, 1.0f), 0.0f, block.Position, 
             ResourceManager::GetTexture("speed")
         ));
    if (ShouldSpawn(20))
        this->powerUps.push_back(
            PowerUp("sticky", glm::vec3(1.0f, 0.5f, 1.0f), 10.0f, block.Position, 
            ResourceManager::GetTexture("sticky") 
        ));
    if (ShouldSpawn(20))
        this->powerUps.push_back(
            PowerUp("pass-through", glm::vec3(0.5f, 1.0f, 0.5f), 20.0f, block.Position, 
            ResourceManager::GetTexture("passthrough")
        ));
    if (ShouldSpawn(20))
        this->powerUps.push_back(
            PowerUp("pad-size-increase", glm::vec3(1.0f, 0.6f, 0.4), 0.0f, block.Position, 
            ResourceManager::GetTexture("increase")    
        ));
    if (ShouldSpawn(15)) // negative powerups should spawn more often
        this->powerUps.push_back(
            PowerUp("confuse", glm::vec3(1.0f, 0.3f, 0.3f), 1.0f, block.Position, 
            ResourceManager::GetTexture("confuse")
        ));
    if (ShouldSpawn(15))
        this->powerUps.push_back(
            PowerUp("chaos", glm::vec3(0.9f, 0.25f, 0.25f), 3.0f, block.Position, 
            ResourceManager::GetTexture("chaos")
        ));
}


bool IsOtherPowerUpActive(std::vector<PowerUp> &powerUps, std::string type)
{
    for (const PowerUp &powerUp : powerUps)
    {
        if (powerUp.Activated && powerUp.Type == type) return true;
    }
    return false;
}


void Game::UpdatePowerUps(float dt)
{
    for (PowerUp &powerUp: powerUps)
    {
        powerUp.Position += powerUp.Velocity * dt;
        
        if (!powerUp.Activated) continue;

        powerUp.Duration -= dt;

        if (powerUp.Duration > 0.0f) continue;
        
        // remove powerup from list (will later be removed)
        powerUp.Activated = false;
        // deactive effects
        if (powerUp.Type == "sticky")
        {
            if (!IsOtherPowerUpActive(this->powerUps, "sticky"))
            {	// only reset if no other PowerUp of type sticky is active
                ball->sticky = false;
                player->Color = glm::vec3(1.0f);
            }
        }
        else if (powerUp.Type == "pass-through")
        {
            if (!IsOtherPowerUpActive(this->powerUps, "pass-through"))
            {	// only reset if no other PowerUp of type pass-through is active
                ball->passThrough = false;
                ball->Color = glm::vec3(1.0f);
            }
        }
        else if (powerUp.Type == "confuse")
        {
            if (!IsOtherPowerUpActive(this->powerUps, "confuse"))
            {	// only reset if no other PowerUp of type confuse is active
                effects->Confuse = false;
            }
        }
        else if (powerUp.Type == "chaos")
        {
            if (!IsOtherPowerUpActive(this->powerUps, "chaos"))
            {	// only reset if no other PowerUp of type chaos is active
                effects->Chaos = false;
            }
        }     
    }

    this->powerUps.erase(std::remove_if(this->powerUps.begin(), this->powerUps.end(),
        [](const PowerUp &powerUp) {return powerUp.Destroyed && !powerUp.Activated;}
    ), this->powerUps.end());
}


Direction Game::VectorDirection(glm::vec2 target)
{
    glm::vec2 compass[] = {
        glm::vec2(0.0f, 1.0f),	// up
        glm::vec2(1.0f, 0.0f),	// right
        glm::vec2(0.0f, -1.0f),	// down
        glm::vec2(-1.0f, 0.0f)	// left
    };

    float max = 0.0f;
    unsigned int bestMatch = 0;
    for (unsigned int i = 0; i < 4; i++)
    {
        float dotProduct = glm::dot(glm::normalize(target), compass[i]);
        if (dotProduct > max)
        {
            max = dotProduct;
            bestMatch = i;
        }
    }
    return (Direction)bestMatch;
}


bool Game::CheckCollision(GameObject &one, GameObject &two)
{
    bool collisionX = one.Position.x + one.Size.x >= two.Position.x &&
        two.Position.x + two.Size.x >= one.Position.x;

    bool collisionY = one.Position.y + one.Size.y >= two.Position.y &&
        two.Position.y + two.Size.y >= one.Position.y;

    return collisionX && collisionY;
}


Collision Game::CheckCollision(BallObject &one, GameObject &two)
{
    // ball circle
    glm::vec2 center(one.Position + one.radius);
    // calculate AABB
    glm::vec2 aabb_half_extents(two.Size.x / 2.0f, two.Size.y / 2.0f);
    glm::vec2 aabb_center(
        two.Position.x + aabb_half_extents.x, 
        two.Position.y + aabb_half_extents.y
    );
    // difference vector
    glm::vec2 difference = center - aabb_center;
    glm::vec2 clampedDifference = glm::clamp(difference, -aabb_half_extents, aabb_half_extents);
    // add clamped value to AABB_center and we get the value of box closest to circle
    glm::vec2 closest = aabb_center + clampedDifference;
    // retrieve vector between center circle and closest point AABB and check if length <= radius
    difference = closest - center;

    if (glm::length(difference) <= one.radius)
        return std::make_tuple(true, VectorDirection(difference), difference);
    else
        return std::make_tuple(false, UP, glm::vec2(0.0f, 0.0f));
}


void ActivatePowerUp(PowerUp &powerUp)
{
    if (powerUp.Type == "speed")
    {
        ball->Velocity *= 1.2;
    }
    else if (powerUp.Type == "sticky")
    {
        ball->sticky = true;
        player->Color = glm::vec3(1.0f, 0.5f, 1.0f);
    }
    else if (powerUp.Type == "pass-through")
    {
        ball->passThrough = true;
        ball->Color = glm::vec3(1.0f, 0.5f, 0.5f);
    }
    else if (powerUp.Type == "pad-size-increase")
    {
        player->Size.x += 50;
    }
    else if (powerUp.Type == "confuse")
    {
        if (!effects->Chaos)
            effects->Confuse = true; // only activate if chaos wasn't already active
    }
    else if (powerUp.Type == "chaos")
    {
        if (!effects->Confuse)
            effects->Chaos = true;
    }
}

 
void Game::DoCollisions()
{
    for (GameObject &box: this->levels[this->level].bricks)
    {
        if (box.Destroyed) continue;
        Collision collision = CheckCollision(*ball, box);
        if (!std::get<0>(collision)) continue;
        // destroy block if not solid
        if (!box.IsSolid) 
        {
            box.Destroyed = true;
            this->SpawnPowerUp(box);
            bleepPaddleMusic.play();
        }
        else
        {
            shakeTime = 0.05f;
            effects->Shake = true;
            solidMusic.play();
        } 
        // collision resolution
        Direction dir = std::get<1>(collision);
        glm::vec2 diffVector = std::get<2>(collision);
        
        // pass through power up
        if (ball->passThrough && !box.IsSolid) continue;

        if (dir == LEFT || dir == RIGHT)
        {
            ball->Velocity.x = -ball->Velocity.x;
            // relocate
            float penetration = ball->radius - std::abs(diffVector.x);
            ball->Position.x += (dir == LEFT) ? penetration : -penetration;
        }
        else
        {
            ball->Velocity.y = -ball->Velocity.y;
            // relocate
            float penetration = ball->radius - std::abs(diffVector.y);
            ball->Position.y += (dir == UP) ? penetration : -penetration;
        }
    }
    
    Collision result = CheckCollision(*ball, *player);

    // and finally check collisions for player pad (unless stuck)
    if (!ball->stuck && std::get<0>(result))
    {
        // check where it hit the board, and change velocity based on where it hit the board
        float centerBoard = player->Position.x + player->Size.x / 2.0f;
        float distance = (ball->Position.x + ball->radius) - centerBoard;
        float percentage = distance / (player->Size.x / 2.0f);
        // then move accordingly
        float strength = 2.0f;
        glm::vec2 oldVelocity = ball->Velocity;
        ball->Velocity.x = INITIAL_BALL_VELOCITY.x * percentage * strength; 
        ball->Velocity.y = -1.0f * abs(ball->Velocity.y);
        ball->Velocity = glm::normalize(ball->Velocity) * glm::length(oldVelocity);
        // sticky power up
        ball->stuck = ball->sticky;
        bleepBlockMusic.play();
    }

    for (PowerUp &powerUp : this->powerUps)
    {
        if (powerUp.Destroyed) continue;
        if (powerUp.Position.y >= this->height) powerUp.Destroyed = true;
        if (CheckCollision(*player, powerUp))
        {
            ActivatePowerUp(powerUp);
            powerUp.Destroyed = true;
            powerUp.Activated = true;
            powerupMusic.play();
        }
    }
    
}