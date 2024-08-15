#include "Scene.h"

class Lose : public Scene {
public:
    int NPC_COUNT = 1;

    ~Lose();

    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram* program) override;
};
