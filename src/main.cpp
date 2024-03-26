#include "app.hpp"

// enforce optimus usage on hybrid GPU systems (integrated + nvidia)
#ifdef _WIN32
#include <windows.h>
extern "C" {
    _declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}
#endif

int main() {
    std::cout << "Start" << std::endl;
    App app;
    std::cout << "App built" << std::endl;
    return app.run();
}