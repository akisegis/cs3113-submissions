#include "LevelC.h"
#include "Utility.h"

#define LEVEL_WIDTH 14
#define LEVEL_HEIGHT 12

constexpr char SPRITESHEET_FILEPATH[] = "assets/images/MouseSpritesheet.png",
               ENEMY_FILEPATH[]       = "assets/images/cheese.png",
               COLLECT_FILEPATH[]     = "assets/images/diamond.png";


unsigned int LEVELC_DATA[] =
{
    2,  0, 0, 20,  4, 4,  5,  11,  3, 4, 4, 4, 4, 5,
    12, 0, 0, 54,  0, 0, 20, 21, 22, 0, 0, 0, 0, 16,
    12, 0, 0, 54, 0, 0, 0, 0, 0, 0, 0, 0, 0, 16,
    12, 0, 0, 54, 0, 0, 36, 32, 32, 33, 0, 0, 0, 16,
    12, 0, 0, 55, 0, 0, 54, 0, 0, 0, 0, 0, 31, 16,
    12, 0, 0, 0,  0, 0, 54, 0, 0, 0, 0, 0, 0, 16,
    12, 0, 0, 53, 0, 0, 54, 0, 0, 53, 0, 0, 0, 16,
    12, 0, 0, 54, 31, 32, 33, 0, 0, 2, 33, 0, 0, 16,
    31,32, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 16,
    12, 0, 0, 0, 0, 31,32, 33,0, 0, 0, 0, 0, 16,
    12, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 16,
    23, 1, 1, 54, 1, 1, 1, 1, 1, 45, 0, 0, 35, 25
};

LevelC::~LevelC()
{
    delete [] m_game_state.npcs;
    delete    m_game_state.player;
    delete    m_game_state.map;
    Mix_FreeChunk(m_game_state.stab_sfx);
    Mix_FreeChunk(m_game_state.coin_sfx);
    Mix_FreeChunk(m_game_state.hp_sfx);
    Mix_FreeMusic(m_game_state.bgm);
}

void LevelC::initialise()
{
    m_game_state.next_scene_id = -1;
    
    GLuint map_texture_id = Utility::load_texture("assets/images/Tileset.png");
    m_game_state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LEVELC_DATA, map_texture_id, 1.0f, 10, 6);
    
    // Code from main.cpp's initialise()
    /**
     George's Stuff
     */
    // Existing
    int player_walking_animation[7][4] =
    {
        { 0, 1, 2, 3 }, // for mouse move left
        { 0, 1, 2, 3 }, // for mouse move right
        { 8, 9, 10, 11 }, // for mouse move up
        { 4, 5, 6, 7  },  // for mouse move down
        { 25, 25, 25, 25  },  // for mouse slash right
        { 29, 29, 29, 29  },  // for mouse slash down
        { 33, 33, 33, 33  }  // for mouse slash up
    };

    glm::vec3 acceleration = glm::vec3(0.0f, -4.81f, 0.0f);
    
    GLuint player_texture_id = Utility::load_texture(SPRITESHEET_FILEPATH);
    
    m_game_state.player = new Entity(
        player_texture_id,         // texture id
        5.0f,                      // speed
        acceleration,              // acceleration
        5.0f,                      // jumping power
        player_walking_animation,  // animation index sets
        0.0f,                      // animation time
        4,                         // animation frame amount
        0,                         // current animation index
        4,                         // animation column amount
        12,                         // animation row amount
        1.75f,                      // width
        1.0f,                       // height
        PLAYER
    );
        
    m_game_state.player->set_position(glm::vec3(10.5f, -10.0f, 0.0f));
    
    /* npcs' stuff */
    GLuint enemy_texture_id = Utility::load_texture(ENEMY_FILEPATH);
    GLuint collect_texture_id = Utility::load_texture(COLLECT_FILEPATH);
    m_game_state.npcs = new Entity[NPC_COUNT];
    
    m_game_state.npcs[0] =  Entity(enemy_texture_id, 0.05f, 1.0f, 1.0f, ENEMY, GUARD, IDLE);
    m_game_state.npcs[0].set_position(glm::vec3(1.375f, -6.0f, 0.0f));
    m_game_state.npcs[0].set_movement(glm::vec3(0.0f));
    
    m_game_state.npcs[1] =  Entity(collect_texture_id, 0.0f, 1.0f, 1.0f, COLLECT, COLLECTABLE, IDLE);
    m_game_state.npcs[1].set_position(glm::vec3(10.91f, -1.41f, 0.0f));
    m_game_state.npcs[1].set_movement(glm::vec3(0.0f));
    
    m_game_state.npcs[2] =  Entity(collect_texture_id, 0.0f, 1.0f, 1.0f, COLLECT, COLLECTABLE, IDLE);
    m_game_state.npcs[2].set_position(glm::vec3(7.375f, -4.2f, 0.0f));
    m_game_state.npcs[2].set_movement(glm::vec3(0.0f));
    
    m_game_state.npcs[3] =  Entity(collect_texture_id, 0.0f, 1.0f, 1.0f, COLLECT, COLLECTABLE, IDLE);
    m_game_state.npcs[3].set_position(glm::vec3(4.54f, -5.33f, 0.0f));
    m_game_state.npcs[3].set_movement(glm::vec3(0.0f));
    
    m_game_state.npcs[4] =  Entity(collect_texture_id, 0.0f, 1.0f, 1.0f, COLLECT, COLLECTABLE, IDLE);
    m_game_state.npcs[4].set_position(glm::vec3(1.584f, -9.583f, 0.0f));
    m_game_state.npcs[4].set_movement(glm::vec3(0.0f));
    
    m_game_state.npcs[5] =  Entity(collect_texture_id, 0.0f, 1.0f, 1.0f, COLLECT, COLLECTABLE, IDLE);
    m_game_state.npcs[5].set_position(glm::vec3(7.625f, -6.42f, 0.0f));
    m_game_state.npcs[5].set_movement(glm::vec3(0.0f));
    
    m_game_state.npcs[6] =  Entity(collect_texture_id, 0.0f, 1.0f, 1.0f, COLLECT, COLLECTABLE, IDLE);
    m_game_state.npcs[6].set_position(glm::vec3(10.25f, -4.2f, 0.0f));
    m_game_state.npcs[6].set_movement(glm::vec3(0.0f));
    
    m_game_state.npcs[6] =  Entity(collect_texture_id, 0.0f, 1.0f, 1.0f, COLLECT, COLLECTABLE, IDLE);
    m_game_state.npcs[6].set_position(glm::vec3(4.54f, -1.584, 0.0f));
    m_game_state.npcs[6].set_movement(glm::vec3(0.0f));
    
    /**
     BGM and SFX
     */
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    
    m_game_state.bgm = Mix_LoadMUS("assets/audio/Level 1 Dreamwave.mp3");
    Mix_PlayMusic(m_game_state.bgm, -1);
    Mix_VolumeMusic(MIX_MAX_VOLUME / 2.0f);
    
    m_game_state.stab_sfx = Mix_LoadWAV("assets/audio/hurt.wav");
    m_game_state.coin_sfx = Mix_LoadWAV("assets/audio/coin.wav");
    m_game_state.hp_sfx = Mix_LoadWAV("assets/audio/hp_up.wav");
}

void LevelC::update(float delta_time)
{
    
    m_game_state.player->update(delta_time, m_game_state.player, m_game_state.npcs, NPC_COUNT, m_game_state.map);
    
    std::cout << "x: " << m_game_state.player->get_position().x << ", y: " << m_game_state.player->get_position().y << "\n";
    
    for (int i = 0; i < NPC_COUNT; i++) {
        m_game_state.npcs[i].update(delta_time, m_game_state.player, m_game_state.npcs, NPC_COUNT, m_game_state.map);
    }
    
    for (int i = 1; i < NPC_COUNT; i++){
        float dist = glm::distance(m_game_state.npcs[i].get_position(), m_game_state.player->get_position());
        
        if (dist < 1.5f && m_game_state.npcs[i].get_active()) {
            m_game_state.npcs[i].deactivate();
            //collectible++
            Mix_PlayChannel(-1, m_game_state.coin_sfx, 0);
        }
    }
    
    if (m_game_state.player->get_collided_top()|| m_game_state.player->get_collided_bottom()||
        m_game_state.player->get_collided_left()||m_game_state.player->get_collided_right()){
        Mix_PlayChannel(-1, m_game_state.stab_sfx, 0);
        m_game_state.next_scene_id = 4;
        // minus hp
    }
    
    if (m_game_state.player->get_position().x < 2.375f && m_game_state.player->get_position().y > 0.75f)
    { m_game_state.next_scene_id = 5;
        Mix_PlayChannel(-1, m_game_state.hp_sfx, 0);
    }
    //x: -1.16088, y: -1.58333 end game
}

void LevelC::render(ShaderProgram *program)
{
    m_game_state.map->render(program);
    for (int i = 0; i < NPC_COUNT; i++) { m_game_state.npcs[i].render(program); }
    m_game_state.player->render(program);
}
