/**
* Author: Aki Segismundo
* Assignment: Lunar Lander
* Date due: 2024-07-13, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

// okokok i know the "sushi" is actually ramen i just thought i used a different sprite... 

#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define LOG(argument) std::cout << argument << '\n'
#define GL_GLEXT_PROTOTYPES 1

#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL2/SDL.h>

#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"

#include "Entity.h"
#include <vector>
#include <ctime>
#include "cmath"

// ————— CONSTANTS ————— //

constexpr int WINDOW_WIDTH  = 640,
              WINDOW_HEIGHT = 480;

constexpr float BG_RED     = 0.9765625f,
                BG_GREEN   = 0.97265625f,
                BG_BLUE    = 0.9609375f,
                BG_OPACITY = 1.0f,

                Y_BOUND         = 4.0f, // floor/ceiling
                X_BOUND         = 5.5f, // left/right walls
                ACC_OF_GRAVITY  = -9.81f / 2,
                FIXED_TIMESTEP  = 1.0f / 60.0f;

constexpr int VIEWPORT_X      = 0,
              VIEWPORT_Y      = 0,
              VIEWPORT_WIDTH  = WINDOW_WIDTH,
              VIEWPORT_HEIGHT = WINDOW_HEIGHT,
              PLATFORM_COUNT  = 6;

constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
               F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

constexpr float MILLISECONDS_IN_SECOND      = 1000.0,
                MINIMUM_COLLISION_DISTANCE  = 0.05f;

constexpr GLint NUMBER_OF_TEXTURES = 1, // to be generated, that is
                LEVEL_OF_DETAIL    = 0, // mipmap reduction image level
                TEXTURE_BORDER     = 0; // this value MUST be zero

/* 
   player   source: george sprite sheet
   platform source: platform open source
   food     source: https://opengameart.org/content/noodles
*/

constexpr char  GEORGE_SPRITE_FILEPATH[]    = "assets/george_0.png",
                PLATFORM_SPRITE_FILEPATH[]  = "assets/snowform.png",
                SUSHI_SPRITE_FILEPATH[]     = "assets/food.png",
                WIN_SPRITE_FILEPATH[]       = "assets/sushi_win.png",
                LOSE_SPRITE_FILEPATH[]      = "assets/sushi_lose.png";

constexpr glm::vec3 END_SCALE     = glm::vec3(10.0f, 8.0f, 0.0f);

// ————— STRUCTS AND ENUMS —————//
enum AppStatus { RUNNING, TERMINATED };

struct GameState {
    Entity* player;
    Entity* platform;
};

// ————— VARIABLES ————— //

GameState     g_game_state;
SDL_Window*   g_display_window;
AppStatus     g_app_status = RUNNING;
ShaderProgram g_shader_program = ShaderProgram();

glm::mat4 g_view_matrix,
          g_projection_matrix,
        
          g_win_matrix,
          g_lose_matrix,
          g_sushi_matrix;

float     g_previous_ticks   = 0.0f,
          g_time_accumulator = 0.0f,
          movement           = 1.0f;

bool      endgame = false,
          win     = false;

GLuint    g_george_texture_id,
          g_platform_texture_id,
          g_sushi_texture_id,
          g_win_texture_id,
          g_lose_texture_id;

// ———— FUNCTIONS ———— //

          void initialise();
          void process_input();
          void update();
          void render();
          void shutdown();

          GLuint load_texture(const char* filepath);
          void draw_object(glm::mat4 &object_model_matrix, GLuint &object_texture_id);

GLuint load_texture(const char* filepath) {
    // STEP 1: Loading the image file
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);
    
    if (image == NULL) {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }
    
    // STEP 2: Generating and binding a texture ID to our image
    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);
    
    // STEP 3: Setting our texture filter parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    // STEP 4: Releasing our file from memory and returning our texture id
    stbi_image_free(image);
    
    return textureID;
}

void draw_object(glm::mat4 &object_g_model_matrix, GLuint &object_texture_id) {
    g_shader_program.set_model_matrix(object_g_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6); // we are now drawing 2 triangles, so use 6, not 3
}


void initialise() {
    // Initialise video and joystick subsystems
    SDL_Init(SDL_INIT_VIDEO);
    
    g_display_window = SDL_CreateWindow("Segismundo Lunar Lander",
                                      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                      WINDOW_WIDTH, WINDOW_HEIGHT,
                                      SDL_WINDOW_OPENGL);
    
    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);

    if (g_display_window == nullptr)
    {
        std::cerr << "Error: SDL window could not be created.\n";
        SDL_Quit();
        exit(1);
    }
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    
    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);
    
    g_view_matrix       = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    
    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);
    
    g_win_matrix        = glm::mat4(1.0f);
    g_lose_matrix       = glm::mat4(1.0f);
    g_sushi_matrix      = glm::mat4(1.0f);
    
    glUseProgram(g_shader_program.get_program_id());
    
    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
    
    g_win_texture_id   = load_texture(WIN_SPRITE_FILEPATH);
    g_lose_texture_id  = load_texture(LOSE_SPRITE_FILEPATH);
    g_sushi_texture_id = load_texture(SUSHI_SPRITE_FILEPATH);
    
    // ————— PLAYER ————— //
        g_george_texture_id = load_texture(GEORGE_SPRITE_FILEPATH);
    int player_walking_animation[4][4] =
        {
            { 1, 5, 9, 13 },  // for George to move to the left,
            { 3, 7, 11, 15 }, // for George to move to the right,
            { 2, 6, 10, 14 }, // for George to move upwards,
            { 0, 4, 8, 12 }   // for George to move downwards
        };
        
        g_game_state.player = new Entity(
            g_george_texture_id,         // texture id
            1.0f,                      // speed
            player_walking_animation,  // animation index sets
            0.0f,                      // animation time
            4,                         // animation frame amount
            0,                         // current animation index
            4,                         // animation column amount
            4                          // animation row amount
        );
    
        g_game_state.player->set_position(glm::vec3(-4.0f, 4.0f, 0.0f));
        g_game_state.player->set_acceleration(glm::vec3(0.0f, ACC_OF_GRAVITY * 0.1f, 0.0f));
        g_game_state.player->face_right();
    
    // ————— PLATFORM ————— //
        g_game_state.platform = new Entity[PLATFORM_COUNT];

        for (int i = 0; i < (PLATFORM_COUNT/2) + 1; i++)
        {
            g_game_state.platform[i].set_texture_id(load_texture(PLATFORM_SPRITE_FILEPATH));
            g_game_state.platform[i].set_position(glm::vec3((4*i) - 4.0f, 2.0f - (i*.5), 0.0f));
            g_game_state.platform[i].set_scale(glm::vec3(1.2f, 1.2f, 1.2f));
            g_game_state.platform[i].update(0.0f, nullptr, 0);
        }
    
        for (int j = (PLATFORM_COUNT/2); j < PLATFORM_COUNT; j++)
        {
            if (j == (PLATFORM_COUNT/2)) {
                g_game_state.platform[j].set_texture_id(load_texture(SUSHI_SPRITE_FILEPATH));
                g_game_state.platform[j].set_position(glm::vec3(3.0f, -1.0f, 0.0f));
                g_game_state.platform[j].set_sushi(true);
            } else {
                g_game_state.platform[j].set_texture_id(load_texture(PLATFORM_SPRITE_FILEPATH));
                g_game_state.platform[j].set_position(glm::vec3(7.0f - (j * 2.15f), 2.0f - j, 0.0f));
            }
            g_game_state.platform[j].set_scale(glm::vec3(1.2f, 1.2f, 1.2f));
            g_game_state.platform[j].update(0.0f, nullptr, 0);
        }
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


void process_input() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
                // ends game
                case SDL_QUIT:
                
                case SDL_WINDOWEVENT_CLOSE:
                     g_app_status = TERMINATED;
                     break;
                    
                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym) {
                            
                        // quits game with escape
                        case SDLK_ESCAPE:
                            g_app_status = TERMINATED;
                            break;
                            
                        case SDLK_UP:
                            if (g_game_state.player->get_collided_bottom())
                                g_game_state.player->jump();
                            break;
                            
                        default:
                            break;
                    }
                                                                                  
                default:
                break;
        }
    }
    const Uint8 *key_state = SDL_GetKeyboardState(NULL);

        if (key_state[SDL_SCANCODE_LEFT])
        {
            g_game_state.player->move_left();
        }
        else if (key_state[SDL_SCANCODE_RIGHT])
        {
            g_game_state.player->move_right();
        }
        
        if (glm::length(g_game_state.player->get_movement()) > 1.0f)
            g_game_state.player->normalise_movement();
}


void update() {
    
    // ————— DELTA TIME ————— //
        float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
        float delta_time = ticks - g_previous_ticks;
        g_previous_ticks = ticks;

        // ————— FIXED TIMESTEP ————— //
        // STEP 1: Keep track of how much time has passed since last step
        delta_time += g_time_accumulator;

        // STEP 2: Accumulate the ammount of time passed while we're under our fixed timestep
        if (delta_time < FIXED_TIMESTEP)
        {
            g_time_accumulator = delta_time;
            return;
        }

        // STEP 3: Once we exceed our fixed timestep, apply that elapsed time into the
        //         objects' update function invocation
        while (delta_time >= FIXED_TIMESTEP)
        {
            // Notice that we're using FIXED_TIMESTEP as our delta time
            g_game_state.player->update(FIXED_TIMESTEP, g_game_state.platform,
                                        PLATFORM_COUNT);
            delta_time -= FIXED_TIMESTEP;
            
        }

        g_time_accumulator = delta_time;
        
        /* Game logic */
            // infinitely bring the character to the top when they fall off
            glm::vec3 adder = g_game_state.player->get_position();
            if (adder[1] < -Y_BOUND) { adder[1] += 2 * Y_BOUND; }
            g_game_state.player->set_position(adder);

    if (!endgame) {
        if (g_game_state.player->get_collided_bottom()){
            if (g_game_state.player->get_position()[0] <= 3.5f && g_game_state.player->get_position()[0] >= 2.5f &&
                g_game_state.player->get_position()[1] <= 1.0f && g_game_state.player->get_position()[1] >= 0.0f){
                endgame = true;
                win     = true;
                g_game_state.player->set_acceleration(glm::vec3 (0.0f));
            }
            
        }
        /* okay i know that there's a way to prevent the infinite fall from 'breaking' the game and removing the way george will infinitely accelerate by setting a check to make sure the acceleration does not exceed a certain point. i however will not be doing that because this is much funnier. */
        
        // if the player goes out of bounds left or right, they fail
        if (adder[0] > X_BOUND || adder [0] < -X_BOUND){
            endgame = true;
            win     = false;
            g_game_state.player->set_acceleration(glm::vec3 (0.0f));
        }
    }
        if (endgame){
            if (win) {
                        g_win_matrix = glm::mat4(1.0f);
                        g_win_matrix = glm::translate(g_win_matrix, glm::vec3(0.0f, 0.0f, 0.0f));
                        g_win_matrix = glm::scale(g_win_matrix, END_SCALE);
                    } else {
                        g_lose_matrix = glm::mat4(1.0f);
                        g_lose_matrix = glm::translate(g_lose_matrix, glm::vec3(0.0f, 0.0f, 0.0f));
                        g_lose_matrix = glm::scale(g_lose_matrix, END_SCALE);
                    }
        }
}


void render() {
    // ————— GENERAL ————— //
       glClear(GL_COLOR_BUFFER_BIT);
        // Vertices
            float vertices[] =
            {
                -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,  // triangle 1
                -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f   // triangle 2
            };

            // Textures
            float texture_coordinates[] =
            {
                0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,     // triangle 1
                0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,     // triangle 2
            };
            
            glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false,
                                  0, vertices);
            glEnableVertexAttribArray(g_shader_program.get_position_attribute());
            
            glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT,
                                  false, 0, texture_coordinates);
            glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
    
    if (!endgame){
        // ————— PLAYER ————— //
        g_game_state.player->render(&g_shader_program);
        
        // ————— PLATFORM ————— //
        for (int i = 0; i < PLATFORM_COUNT; i++) g_game_state.platform[i].render(&g_shader_program);
        
        draw_object(g_sushi_matrix, g_sushi_texture_id);
        
    } else {
        
        if (win) { draw_object(g_win_matrix, g_win_texture_id);   }
        else     { draw_object(g_lose_matrix, g_lose_texture_id); }
        
    }
    // ————— GENERAL ————— //
        // We disable two attribute arrays now
       glDisableVertexAttribArray(g_shader_program.get_position_attribute());
       glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
       SDL_GL_SwapWindow(g_display_window);
}


void shutdown() {
    SDL_Quit();
    delete   g_game_state.player;
    delete[] g_game_state.platform;
}


int main(int argc, char* argv[]) {
    initialise();
    
    while (g_app_status == RUNNING)
    {
        process_input();
        update();
        render();
    }
    
    shutdown();
    return 0;
}

