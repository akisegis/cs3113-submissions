#include "Scene.h"

class Win : public Scene {
public:
    int NPC_COUNT = 1;

    ~Win();

    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram* program) override;
};
