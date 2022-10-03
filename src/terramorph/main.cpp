#include "terramorph/Application.h"

int main() {

    auto window = gaunlet::Core::CreateRef<gaunlet::Core::Window>("Terramorph");
    gaunlet::Core::RunLoop runLoop(window);
    terramorph::Application app;

    runLoop.run(app);

    return 0;

}