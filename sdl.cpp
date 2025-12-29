#include "sdl.h"
#include <iostream>

Sdl::Sdl(int width, int height) : width(width), height(height) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "Erreur SDL: " << SDL_GetError() << std::endl;
        exit(EXIT_FAILURE);
    }
    window = SDL_CreateWindow("Simulation de chaleur", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Erreur création fenêtre: " << SDL_GetError() << std::endl;
        exit(EXIT_FAILURE);
    }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
}

Sdl::~Sdl() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
void drawFilledCircle(SDL_Renderer* renderer, int centerX, int centerY, int radius) {
    for (int w = 0; w < radius * 2; w++) {
        for (int h = 0; h < radius * 2; h++) {
            int dx = radius - w; // Distance horizontale par rapport au centre
            int dy = radius - h; // Distance verticale par rapport au centre
            if ((dx * dx + dy * dy) <= (radius * radius)) {
                SDL_RenderDrawPoint(renderer, centerX + dx, centerY + dy);
            }
        }
    }
}

void Sdl::render(const std::vector<double>& temperatures) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Couleur de fond : noir
    SDL_RenderClear(renderer);

    for (size_t i = 0; i < temperatures.size(); ++i) {
        int x = static_cast<int>(i * width / (temperatures.size() - 1));
        int y = height - static_cast<int>(temperatures[i] * height / 100.0);

        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); 
        drawFilledCircle(renderer, x, y, 7); 
    }

    SDL_RenderPresent(renderer);
}

