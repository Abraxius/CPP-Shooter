#pragma once

struct Window {
    Window(int window_width, int window_height, int nSamples);
    ~Window();
    void swap();
    void handle_event(SDL_Event& event) {
        // TODO
    }
    
    SDL_Window* pWindow;
    SDL_GLContext context;
    int width;
    int height;
};