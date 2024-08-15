#include "Menu_Screen.h"
#include "Utility.h"
#define LEVEL_WIDTH 14
#define LEVEL_HEIGHT 8

constexpr char SPRITESHEET_FILEPATH[] = "assets/images/MouseSpritesheet.png",
               ENEMY_FILEPATH[]       = "assets/images/cheese.png",
               COLLECT_FILEPATH[]     = "assets/images/diamond.png";


unsigned int START_DATA[] =
{
    3,  4, 4, 4,  4, 4,  4,  4,  4, 4, 4, 4, 4, 5,
    12, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 16,
    12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 16,
    12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 16,
    12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 16,
    12, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 16,
    12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 16,
    23, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 25
};


Menu_Screen::~Menu_Screen()
{
    delete [] m_game_state.npcs;
    delete    m_game_state.player;
    delete    m_game_state.map;
    Mix_FreeChunk(m_game_state.stab_sfx);
    Mix_FreeChunk(m_game_state.coin_sfx);
    Mix_FreeChunk(m_game_state.hp_sfx);
    Mix_FreeMusic(m_game_state.bgm);
}

void Menu_Screen::initialise()
{
    m_game_state.next_scene_id = -1;

    GLuint map_texture_id = Utility::load_texture("assets/images/Tileset.png");
    m_game_state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, START_DATA, map_texture_id, 1.0f, 10, 6);
    
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
        
    m_game_state.player->set_position(glm::vec3(6.75f, -4.0f, 0.0f));
    
    /* npcs' stuff */
    
    GLuint collect_texture_id = Utility::load_texture(COLLECT_FILEPATH);
    m_game_state.npcs = new Entity[NPC_COUNT];
    
    m_game_state.npcs[0] =  Entity(collect_texture_id, 0.05f, 1.0f, 1.0f, COLLECT, COLLECTABLE, IDLE);
    m_game_state.npcs[0].set_position(glm::vec3(10.5f, -5.0f, 0.0f));
    m_game_state.npcs[0].set_movement(glm::vec3(0.0f));
    
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

    m_game_state.bgm = Mix_LoadMUS("assets/audio/Level 1 Dreamwave.mp3");
    Mix_PlayMusic(m_game_state.bgm, -1);
    Mix_VolumeMusic(MIX_MAX_VOLUME / 2.0f);
    
    m_game_state.coin_sfx = Mix_LoadWAV("assets/audio/coin.wav");
}

void Menu_Screen::update(float delta_time)
{
    m_game_state.player->update(delta_time, m_game_state.player, m_game_state.npcs, NPC_COUNT, m_game_state.map);
    
    std::cout << "x: " << m_game_state.player->get_position().x << ", y: " << m_game_state.player->get_position().y << "\n";
    
    for (int i = 0; i < NPC_COUNT; i++) {
        m_game_state.npcs[i].update(delta_time, m_game_state.player, m_game_state.npcs, NPC_COUNT, m_game_state.map);
    }
    
    for (int i = 0; i < NPC_COUNT; i++){
        float dist = glm::distance(m_game_state.npcs[i].get_position(), m_game_state.player->get_position());
        
        if (dist < 1.5f && m_game_state.npcs[i].get_active()) {
            m_game_state.npcs[i].deactivate();
            m_game_state.next_scene_id = 1;
            //collectible++
            Mix_PlayChannel(-1, m_game_state.coin_sfx, 0);
        }
    }
}

void Menu_Screen::render(ShaderProgram* program)
{
    m_game_state.map->render(program);
    for (int i = 0; i < NPC_COUNT; i++) { m_game_state.npcs[i].render(program); }
    m_game_state.player->render(program);
    GLuint font_texture_id_1 = Utility::load_texture("assets/fonts/font1.png");
    Utility::draw_text(program, font_texture_id_1, "The Great Mouse Escape: The Hidden Heist", 0.5f, -0.25f, glm::vec3(1.7f, -1.5f, 0.0f));
    Utility::draw_text(program, font_texture_id_1, "Move with the Arrow Keys to get the Diamond", 0.5f, -0.25f, glm::vec3(1.3f, -2.5f, 0.0f));
    Utility::draw_text(program, font_texture_id_1, "Watch out for the Cheese Trapper!", 0.5f, -0.25f, glm::vec3(2.5f, -3.5f, 0.0f));
}
