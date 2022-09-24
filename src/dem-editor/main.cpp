#include "dem-editor/Application.h"

int main() {

    auto window = gaunlet::Core::CreateRef<gaunlet::Core::Window>("DEM Editor");
    gaunlet::Core::RunLoop runLoop(window);
    DemEditor::Application app;

    runLoop.run(app);

    return 0;

}