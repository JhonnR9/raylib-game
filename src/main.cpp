
#include "app.h"

int main() {
    const auto* app = new rpg::APP();

    app->run();
    delete app;

    return 0;
}
