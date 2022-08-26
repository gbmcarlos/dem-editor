#include "gaunlet/Core.h"

int main() {

    engine::Core::Application app("Empty Screen");
    engine::Core::RunLoop runLoop(app);

    runLoop.run();

    return 0;

}