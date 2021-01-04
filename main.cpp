#include "controller.h"
#include <string>

using namespace sb;

int main(int argc, char* argv[])
{
    ControllerState initState = MENU;
    if(argc > 1 && argv[1][1] == 'd')
        initState = DEBUG;
        
    sb::Controller controller(initState);

    controller.initApp();
    controller.getRoot()->startRendering();
    controller.cleanup(true);
    controller.closeApp();

    return 0;
}
