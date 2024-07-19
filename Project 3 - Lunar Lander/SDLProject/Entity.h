enum AnimationDirection { LEFT, RIGHT, UP, DOWN };

class Entity
{
private:
    bool active    = true,
         sushi     = false;
    
    int m_walking[4][4]; // 4x4 array for walking animations
    // ––––– ANIMATION ––––– //
    int*  m_animation_right = NULL, // move to the right
        * m_animation_left  = NULL, // move to the left
        * m_animation_up    = NULL, // move upwards
        * m_animation_down  = NULL; // move downwards

    // ––––– PHYSICS (GRAVITY) ––––– //
    glm::vec3 m_position;
    glm::vec3 m_velocity;
    glm::vec3 m_acceleration;

    // ————— TRANSFORMATIONS ————— //
    float     m_speed;
    glm::vec3 m_movement;
    glm::mat4 m_model_matrix;
    glm::vec3 m_scale;

    float m_width = 1;
    float m_height = 1;


public:
    // ————— STATIC VARIABLES ————— //
    static const int GRAVITY = -1;
    static const int SECONDS_PER_FRAME = 4;
    static const int LEFT   = 0,
                     RIGHT  = 1,
                     UP     = 2,
                     DOWN   = 3;
    
    // ––––– JUMPING ––––– //
    bool  is_jump        = false;
    float jump_power     = 1.25f;

    // ––––– COLLISIONS ––––– //
    bool top_collision     = false;
    bool bottom_collision  = false;
    bool left_collision    = false;
    bool right_collision   = false;

    // ————— ANIMATION ————— //

    int m_animation_frames = 0,
        m_animation_index  = 0,
        m_animation_cols   = 0,
        m_animation_rows   = 0;

    int* m_animation_indices = NULL;
    float m_animation_time   = 0.0f;

    GLuint    m_texture_id;

    // ————— METHODS ————— //
    
    Entity();
    Entity(GLuint texture_id, float speed, int walking[4][4], float animation_time,
           int animation_frames, int animation_index, int animation_cols,
           int animation_rows);
    Entity(GLuint texture_id, float speed); // Simpler constructor
    ~Entity();

    void draw_sprite_from_texture_atlas(ShaderProgram* program, GLuint texture_id, int index);
    bool const check_collision(Entity* other) const;
    void const check_collision_y(Entity* collidable_entities, int collidable_entity_count);
    void const check_collision_x(Entity* collidable_entities, int collidable_entity_count);

    void update(float delta_time, Entity* collidable_entities, int collidable_entity_count);
    void render(ShaderProgram* program);
    
    void face_left()  { m_animation_indices = m_walking[LEFT];  };
    void face_right() { m_animation_indices = m_walking[RIGHT]; };
    void face_up()    { m_animation_indices = m_walking[UP];    };
    void face_down()  { m_animation_indices = m_walking[DOWN];  };

    void move_left()  { m_movement.x = -1.0f; face_left();  };
    void move_right() { m_movement.x =  1.0f; face_right(); };
    void move_up()    { m_movement.y =  1.0f; face_up();    };
    void move_down()  { m_movement.y = -1.0f; face_down();  };
    
    void const jump() { is_jump = true; };
    
    void normalise_movement() { m_movement = glm::normalize(m_movement); };

    void activate()   { active = true; };
    void deactivate() { active = false; };

    // ————— GETTERS ————— //
    
    glm::vec3 const get_scale()        const { return m_scale;        };
    glm::vec3 const get_position()     const { return m_position;     };
    glm::vec3 const get_velocity()     const { return m_velocity;     };
    glm::vec3 const get_acceleration() const { return m_acceleration; };
    glm::vec3 const get_movement()     const { return m_movement;     };
    float     const get_speed()        const { return m_speed;        };
    int       const get_width()        const { return m_width;        };
    int       const get_height()       const { return m_height;       };
    
    bool      const get_active() const    { return active;    };
    bool      const get_sushi() const    { return sushi;    };
    bool      const get_collided_top() const    { return top_collision;    };
    bool      const get_collided_bottom() const { return bottom_collision; };
    bool      const get_collided_right() const  { return right_collision;  };
    bool      const get_collided_left() const   { return left_collision;   };

    // ————— SETTERS ————— //
    
    void const set_texture_id(GLuint new_texture_id)        { m_texture_id     = new_texture_id;           };
    void const set_scale(glm::vec3 new_scale)               { m_scale          = new_scale;                };
    void const set_position(glm::vec3 new_position)         { m_position       = new_position;             };
    void const set_velocity(glm::vec3 new_velocity)         { m_velocity       = new_velocity;             };
    void const set_acceleration(glm::vec3 new_position)     { m_acceleration   = new_position;             };
    void const sub_acceleration_x(float sub_x)              { m_acceleration.x = m_acceleration.x - sub_x; };
    void const set_movement(glm::vec3 new_movement)         { m_movement       = new_movement;             };
    void const set_speed(float new_speed)                   { m_speed          = new_speed;                };
    void const set_width(float new_width)                   { m_width          = new_width;                };
    void const set_height(float new_height)                 { m_height         = new_height;               };
    void const set_sushi(float new_sushi)                   { sushi            = new_sushi;                };
    
    // Setter for m_walking
        void set_walking(int walking[4][4])
        {
            for (int i = 0; i < 4; ++i)
            {
                for (int j = 0; j < 4; ++j)
                {
                    m_walking[i][j] = walking[i][j];
                }
            }
        }
};
