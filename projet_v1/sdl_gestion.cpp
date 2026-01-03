#include "sdl_gestion.h"
#include <iostream>
#include <algorithm>
#include <cmath>

Sdl::Sdl(std::string titre, int largeur, int hauteur) 
    : largeur_(largeur), hauteur_(hauteur) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Erreur SDL: " << SDL_GetError() << std::endl;
        exit(1);
    }
    fenetre_ = SDL_CreateWindow(titre.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, largeur, hauteur, 0);
    rendu_ = SDL_CreateRenderer(fenetre_, -1, SDL_RENDERER_ACCELERATED);
}

Sdl::~Sdl() {
    SDL_DestroyRenderer(rendu_);
    SDL_DestroyWindow(fenetre_);
    SDL_Quit();
}

void Sdl::effacer() {
    SDL_SetRenderDrawColor(rendu_, 0, 0, 0, 255); 
    SDL_RenderClear(rendu_);
}

void Sdl::afficher() {
    SDL_RenderPresent(rendu_);
}

void Sdl::changer_titre(const std::string& titre) {
    SDL_SetWindowTitle(fenetre_, titre.c_str());
}

int Sdl::verifier_entree() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) return 1;
        if (e.type == SDL_KEYDOWN) {
            if (e.key.keysym.sym == SDLK_m) return 2;
        }
    }
    return 0;
}

void Sdl::attendre(int millisecondes) {
    SDL_Delay(millisecondes);
}

void Sdl::definir_couleur_temp(double temp, double t_min, double t_max) {
    double ratio = (temp - t_min) / (t_max - t_min);
    if (ratio < 0.0) ratio = 0.0;
    if (ratio > 1.0) ratio = 1.0;
    
    Uint8 r = (Uint8)(50 + (205 * ratio)); 
    Uint8 g = (Uint8)(50 - (50 * ratio));
    Uint8 b = (Uint8)(50 - (50 * ratio));
    SDL_SetRenderDrawColor(rendu_, r, g, b, 255);
}

void Sdl::dessiner_barre(const std::vector<double>& temp, double t_min, double t_max) {
    int n = temp.size();
    // Utilisation de double pour w_seg pour éviter la division entière donnant 0
    double w_seg = (double)largeur_ / n;
    int h_barre = 100;
    int y_start = hauteur_ / 2 - h_barre / 2;

    for (int i = 0; i < n; ++i) {
        definir_couleur_temp(temp[i], t_min, t_max);
        
        // Calcul précis des coordonnées
        int x = (int)(i * w_seg);
        int w = (int)((i + 1) * w_seg) - x; // Astuce pour éviter les trous
        if (w < 1) w = 1; // Au moins 1 pixel de large

        SDL_Rect r = {x, y_start, w, h_barre};
        SDL_RenderFillRect(rendu_, &r);
    }
}

void Sdl::dessiner_surface_2d(const std::vector<double>& grille, int N, double t_min, double t_max) {
    // Optimisation : On ne dessine pas des rectangles, mais des points (pixels).
    // Si la grille est plus grande que la fenêtre, on saute des points (downsampling).
    
    double step_x = (double)largeur_ / N;
    double step_y = (double)hauteur_ / N;
    
    // Si la grille est très fine (1001 pts) pour une fenêtre de 800px, 
    // step est < 1. On risque de dessiner plusieurs points au même pixel (lent).
    
    // Stratégie optimisée : parcourir les PIXELS de l'écran et chercher la valeur correspondante
    // C'est beaucoup plus rapide (800x800 iterations) que parcourir la grille (1000x1000).
    
    for (int y = 0; y < hauteur_; ++y) {
        for (int x = 0; x < largeur_; ++x) {
            // Retrouver l'indice i,j dans la grille correspondant au pixel x,y
            int i = (int)(x / step_x); 
            int j = (int)(y / step_y);
            
            // Sécurité bornes
            if (i >= N) i = N - 1;
            if (j >= N) j = N - 1;

            double val = grille[i * N + j];
            definir_couleur_temp(val, t_min, t_max);
            SDL_RenderDrawPoint(rendu_, x, y);
        }
    }
}