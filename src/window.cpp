#include <glbinding/gl46core/gl.h>
#include <glbinding/glbinding.h>
#include <glbinding/AbstractFunction.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>
//
#include <iostream>
#include <cassert>
//
#include "window.hpp"

Window::Window(int window_width, int window_height, int nSamples = 1) : width(window_width), height(window_height) {
    if (SDL_InitSubSystem(SDL_InitFlags::SDL_INIT_VIDEO | SDL_InitFlags::SDL_INIT_AUDIO)) std::cout << SDL_GetError();

    // set opengl version
    SDL_GL_SetAttribute(SDL_GLattr::SDL_GL_CONTEXT_PROFILE_MASK, SDL_GLprofile::SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GLattr::SDL_GL_CONTEXT_MAJOR_VERSION, 4); // adjust
    SDL_GL_SetAttribute(SDL_GLattr::SDL_GL_CONTEXT_MINOR_VERSION, 6); // adjust
    SDL_GL_SetAttribute(SDL_GLattr::SDL_GL_DOUBLEBUFFER, 1);
    if (nSamples > 1) { // set up multisampling capabilities
        if ((nSamples & (nSamples - 1))) std::cerr << "Multisample samples need to be a power of two" << std::endl;
        SDL_GL_SetAttribute(SDL_GLattr::SDL_GL_MULTISAMPLEBUFFERS, 1); // enable multisampling
        SDL_GL_SetAttribute(SDL_GLattr::SDL_GL_MULTISAMPLESAMPLES, nSamples); // set number of samples per pixel
    }

    #ifndef NDEBUG
    // only enable explicit logging in debug mode
    SDL_GL_SetAttribute(SDL_GLattr::SDL_GL_CONTEXT_FLAGS, SDL_GLcontextFlag::SDL_GL_CONTEXT_DEBUG_FLAG);
    #endif

    // create OpenGL window
    // SDL_GL_MULTISAMPLEBUFFERS
    pWindow = SDL_CreateWindow("OpenGL Renderer", width, height, SDL_WINDOW_OPENGL);
    if (pWindow == nullptr) std::cout << SDL_GetError();

    // create opengl context
    context = SDL_GL_CreateContext(pWindow);
    if (context == nullptr) std::cout << SDL_GetError();

    // set up glbinding loader (lazy loading)
    glbinding::initialize(SDL_GL_GetProcAddress, false);
    // error logging (currently enabled in release mode as well)
    glbinding::setCallbackMaskExcept(glbinding::CallbackMask::After | glbinding::CallbackMask::ParametersAndReturnValue, { "glGetError" });
    glbinding::setAfterCallback([](const glbinding::FunctionCall& call) {
        static unsigned int callCount = 0;
        static constexpr unsigned int maxCalls = 3;

        const auto errorCode = glGetError();
        if (errorCode != GL_NO_ERROR)
        {
            // only allow a certain number of errors to be printed
            if (++callCount >= maxCalls) {
                if (callCount == maxCalls) std::cerr << "Too many OpenGL errors" << std::endl;
                return;
            }

            // print out the function name, parameters and return value
            std::cout << call.function->name() << '(';
            for (unsigned i = 0; i < call.parameters.size(); ++i) {
                std::cout << call.parameters[i].get();
                if (i < call.parameters.size() - 1) std::cout << ", ";
            }
            std::cout << ')';
            if (call.returnValue) std::cout << " -> " << call.returnValue.get();
            std::cout << '\n';

            // print out error code
            std::string error;
            switch (errorCode) {
                case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
                case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
                case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
                case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
                case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
                case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
                case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
            }
            std::cout << "\tError: " << error << std::endl;
        }
    });

    glClearColor(.5f, .5f, .5f, 1.0f); // default screen color
    glEnable(GL_CULL_FACE); // cull backfaces
    glEnable(GL_DEPTH_TEST); // enable depth buffer and depth testing
    // glEnable(GL_FRAMEBUFFER_SRGB); // gamma corrected framebuffer
    SDL_SetRelativeMouseMode(SDL_TRUE); // capture mouse for better camera controls
    SDL_GL_SetSwapInterval(1); // vsync
    if (nSamples > 1) {
        glEnable(GL_MULTISAMPLE);  // enable multisampling application-wide
        glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE); // enable color blending via multisampling
    }

    // color blending via fragment shader's alpha value
    // glEnable(GL_BLEND); // color blending for proper transparency
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // define function for blending colors

    // initialize ImGUI
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui_ImplSDL3_InitForOpenGL(pWindow, context);
    ImGui_ImplOpenGL3_Init();
}
Window::~Window() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    SDL_Quit();
}

void Window::swap() {
    SDL_GL_SwapWindow(pWindow);
}