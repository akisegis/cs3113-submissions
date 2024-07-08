/**
* Author: Aki Segismundo
* Assignment: Pong Clone
* Date due: 2024-06-29, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/


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

enum AppStatus { RUNNING, TERMINATED };

bool IsCpu = false,
     CpuUp = true,
     Win   = false,
     P1    = true;

constexpr int WINDOW_WIDTH  = 640,
              WINDOW_HEIGHT = 480;

constexpr float BG_RED     = 0.9765625f,
                BG_GREEN   = 0.97265625f,
                BG_BLUE    = 0.9609375f,
                BG_OPACITY = 1.0f,

                Y_BOUND    = 3.3f, // floor/ceiling
                X_BOUND    = 4.3f; // left/right walls

constexpr int VIEWPORT_X      = 0,
              VIEWPORT_Y      = 0,
              VIEWPORT_WIDTH  = WINDOW_WIDTH,
              VIEWPORT_HEIGHT = WINDOW_HEIGHT;

constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
               F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

constexpr float MILLISECONDS_IN_SECOND      = 1000.0,
                MINIMUM_COLLISION_DISTANCE  = 0.05f,
                PLAYER_WIDTH                = 0.4f,
                PLAYER_HEIGHT               = 0.8f,
                BALL_WIDTH                  = 0.9f,
                BALL_HEIGHT                 = 0.4f,
                PLAYER_SPEED                = 2.0f;

constexpr GLint NUMBER_OF_TEXTURES = 1, // to be generated, that is
                LEVEL_OF_DETAIL    = 0, // mipmap reduction image level
                TEXTURE_BORDER     = 0; // this value MUST be zero

/* 
   player source: https://pluspng.com/img-png/ping-pong-paddle-pin-png-1436.png
   ball   source: https://www.stickpng.com/img/food/pizza/pizza-slice
*/

constexpr char P1_SPRITE_FILEPATH[]   = "p1.png",
               P2_SPRITE_FILEPATH[]   = "p2.png",
               BALL_SPRITE_FILEPATH[] = "ball.png",
               WIN1_SPRITE_FILEPATH[] = "WP1.png",
               WIN2_SPRITE_FILEPATH[] = "WP2.png";

constexpr glm::vec3 INIT_SCALE     = glm::vec3(0.5f, 0.8f, 0.0f),
                    BALL_SCALE     = glm::vec3(0.8f, 0.8f, 0.0f),
                    WIN_SCALE      = glm::vec3(3.8f, 3.0f, 0.0f),
                    WIN_POS        = glm::vec3(0.0f, 0.0f, 0.0f);

SDL_Window*   g_display_window;
AppStatus     g_app_status = RUNNING;
ShaderProgram g_shader_program = ShaderProgram();

glm::mat4 g_view_matrix,
          g_p1_matrix,
          g_p2_matrix,
          g_ball_matrix,
          g_win1_matrix,
          g_win2_matrix,
          g_projection_matrix;

float     g_previous_ticks = 0.0f,
          movement         = 1.0f;


glm::vec3 movement_p1    = glm::vec3(0.0f, 0.0f, 0.0f),
          position_p1    = glm::vec3(-4.5f, 0.0f, 0.0f),

          movement_p2    = glm::vec3(0.0f, 0.0f, 0.0f),
          position_p2    = glm::vec3(4.5f, 0.0f, 0.0f),

          movement_ball  = glm::vec3(0.5f, 0.125f, 0.0f),
          position_ball  = glm::vec3(0.0f, 0.0f, 0.0f);

GLuint    g_p1_texture_id,
          g_p2_texture_id,
          g_ball_texture_id,
          g_win1_texture_id,
          g_win2_texture_id;

// all functions

          void initialise();
          void process_input();
          void update();
          void render();
          void shutdown();

          GLuint load_texture(const char* filepath);
          void draw_object(glm::mat4 &object_model_matrix, GLuint &object_texture_id);
          bool ball_collision(glm::vec3& paddle_position);

GLuint load_texture(const char* filepath) {
    // STEP 1: Loading the image file
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);
    
    if (image == NULL)
    {
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

bool ball_collision(glm::vec3& player_position) {
    float x_distance = fabs(player_position[0] - position_ball[0]) - ((PLAYER_WIDTH + BALL_WIDTH) / 2.0f);
    float y_distance = fabs(player_position[1] - position_ball[1]) - ((PLAYER_HEIGHT + BALL_HEIGHT) / 2.0f);

    if (x_distance < MINIMUM_COLLISION_DISTANCE && y_distance < MINIMUM_COLLISION_DISTANCE) { return true; }
    else { return false; }
}


void initialise() {
    // Initialise video and joystick subsystems
    SDL_Init(SDL_INIT_VIDEO);
    
    g_display_window = SDL_CreateWindow("Segismundo Pong Clone",
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
    
    g_p1_matrix         = glm::mat4(1.0f);
    g_p2_matrix         = glm::mat4(1.0f);
    g_ball_matrix       = glm::mat4(1.0f);
    g_win1_matrix       = glm::mat4(1.0f);
    g_win2_matrix       = glm::mat4(1.0f);
    g_view_matrix       = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    
    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);
    
    glUseProgram(g_shader_program.get_program_id());
    
    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
    
    g_p1_texture_id   = load_texture(P1_SPRITE_FILEPATH);
    g_p2_texture_id   = load_texture(P2_SPRITE_FILEPATH);
    g_ball_texture_id = load_texture(BALL_SPRITE_FILEPATH);
    g_win1_texture_id = load_texture(WIN1_SPRITE_FILEPATH);
    g_win2_texture_id = load_texture(WIN2_SPRITE_FILEPATH);
    
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
                    
                        // switches p2 into a computer player with t
                        case SDLK_t:
                            IsCpu = !IsCpu;
                            break;

                        default:
                            break;
                    }
                                                                                  
                default:
                break;
        }
    }
    const Uint8* key_state = SDL_GetKeyboardState(NULL);
    
    //player one movement
        if (key_state[SDL_SCANCODE_Q]) { movement_p1.y = 1.0f; }
        // if Q is pressed, move p1 up
        else if (key_state[SDL_SCANCODE_S]) { movement_p1.y = -1.0f; }
        // if S is pressed, move p1 down
        else if (!key_state[SDL_SCANCODE_Q] && !key_state[SDL_SCANCODE_S]){ movement_p1.y = 0.0f; }
        // if both Q and S are not pressed, do not move p1


    // player 2 movement given player 2 is controllable
        if (IsCpu == false) {
                 if (key_state[SDL_SCANCODE_P]) { movement_p2.y = 1.0f; }
            // if P is pressed move p2 up
            else if (key_state[SDL_SCANCODE_L]) { movement_p2.y = -1.0f; }
            // if L is pressed move p2 down
            else if (!key_state[SDL_SCANCODE_P] && !key_state[SDL_SCANCODE_L]){ movement_p2.y = 0.0f; }
            // if both P and L are not pressed, do not move p2
    }
    
    // normalize player movement
    if (glm::length(movement_p1) > 1.0f) { movement_p1 = glm::normalize(movement_p1); }
    if (glm::length(movement_p2) > 1.0f) { movement_p2 = glm::normalize(movement_p2); }
}


void update() {
    if (!Win) { // as long as there is no winner
        /* Delta time calculations */
        float ticks      = (float) SDL_GetTicks() / MILLISECONDS_IN_SECOND;
        float delta_time = ticks - g_previous_ticks;
        g_previous_ticks = ticks;
        
        /* Game logic */
        // Player 1 Floor and Ceiling Logic
        if ( position_p1[1] < -Y_BOUND){
            // if p1 hits the floor, set movement to 0 and 'bounce' the player upwards
            movement_p1.y = 0.0f;
            position_p1[1] = -Y_BOUND + 0.1f;
        }
        else if (position_p1[1] >  Y_BOUND) {
            // if p1 hits the floor, set movement to 0 and 'bounce' the player downwards
            movement_p1.y = 0.0f;
            position_p1[1] = Y_BOUND - 0.1f;
        }
        else { position_p1 += movement_p1 * PLAYER_SPEED * delta_time; }
        // otherwise, move normally
        
        // Player 2 Floor and Ceiling Logic
        if ( position_p2[1] < -Y_BOUND){
            // if p2 hits the floor, set movement to 0 and 'bounce' the player upwards
            movement_p2.y = 0.0f;
            position_p2[1] = -Y_BOUND + 0.1f;
        }
        else if (position_p2[1] >  Y_BOUND) {
            // if p2 hits the floor, set movement to 0 and 'bounce' the player downwards
            movement_p2.y = 0.0f;
            position_p2[1] = Y_BOUND - 0.1f;
        }
        else { position_p2 += movement_p2 * PLAYER_SPEED * delta_time; }
        // otherwise, move normally
        
        // CPU Movement Logic
        if (IsCpu == true){
            // if CPU mode is active, throw everything above away and do this
            if (position_p2[1] < -Y_BOUND) { CpuUp = true; } // flip bool
            if (position_p2[1] >  Y_BOUND) { CpuUp = false; }
            if (CpuUp) { movement_p2.y =  movement;} // if below the ceiling, move upward
            else       { movement_p2.y = -movement;} // if above the floor  , move downward
        }
        
        // Ball Logic
        
        if ( ball_collision(position_p1) || ball_collision(position_p2) )
        { movement_ball.x *= -1.0;}
        
        if ( position_ball[1] > (Y_BOUND + 0.5f) || position_ball[1] < -Y_BOUND)
        { movement_ball.y *= -1.0; }
        
        if ( position_ball[0] > (X_BOUND - MINIMUM_COLLISION_DISTANCE)){
            Win = true;
            P1  = true;
        }
        if ( position_ball[0] < (-X_BOUND + MINIMUM_COLLISION_DISTANCE)){
            Win = true;
            P1  = false;
        }
        
        position_ball += movement_ball * PLAYER_SPEED * delta_time;
        
        
        /* Model matrix resets */
        g_p1_matrix   = glm::mat4(1.0f);
        g_p2_matrix   = glm::mat4(1.0f);
        g_ball_matrix = glm::mat4(1.0f);
        
        /* Transformations */
        // player 1
        g_p1_matrix = glm::translate(g_p1_matrix, position_p1);
        g_p1_matrix = glm::scale(g_p1_matrix, INIT_SCALE);
        
        // player 2
        g_p2_matrix = glm::translate(g_p2_matrix, position_p2);
        g_p2_matrix = glm::scale(g_p2_matrix, INIT_SCALE);
        
        // ball
        g_ball_matrix = glm::translate(g_ball_matrix, position_ball);
        g_ball_matrix = glm::scale(g_ball_matrix, BALL_SCALE);
    } else {
        if (P1) {
            g_win1_matrix = glm::mat4(1.0f);
            g_win1_matrix = glm::translate(g_win1_matrix, WIN_POS);
            g_win1_matrix = glm::scale(g_win1_matrix, WIN_SCALE);
        } else {
            g_win2_matrix = glm::mat4(1.0f);
            g_win2_matrix = glm::translate(g_win2_matrix, WIN_POS);
            g_win2_matrix = glm::scale(g_win2_matrix, WIN_SCALE);
        }
    }
}


void render() {
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
    
    // Bind texture
    if (!Win) {
        draw_object(g_p1_matrix, g_p1_texture_id);
        draw_object(g_p2_matrix, g_p2_texture_id);
        draw_object(g_ball_matrix, g_ball_texture_id);
    } else {
        if (P1) { draw_object(g_win1_matrix, g_win1_texture_id); }
        else    { draw_object(g_win2_matrix, g_win2_texture_id); }
    }
    // We disable two attribute arrays now
    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
    
    SDL_GL_SwapWindow(g_display_window);
}


void shutdown() { SDL_Quit(); }


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

