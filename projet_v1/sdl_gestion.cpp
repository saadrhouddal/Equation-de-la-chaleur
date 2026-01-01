#include "sdl_gestion.h"
#include <iostream>
#include <algorithm>
#include <cmath>

GestionSDL::GestionSDL(std::string titre, int largeur, int hauteur) 
    : largeur_(largeur), hauteur_(hauteur) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Erreur SDL: " << SDL_GetError() << std::endl;
        exit(1);
    }
    fenetre_ = SDL_CreateWindow(titre.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, largeur, hauteur, 0);
    rendu_ = SDL_CreateRenderer(fenetre_, -1, SDL_RENDERER_ACCELERATED);
}

GestionSDL::~GestionSDL() {
    SDL_DestroyRenderer(rendu_);
    SDL_DestroyWindow(fenetre_);
    SDL_Quit();
}

void GestionSDL::effacer() {
    SDL_SetRenderDrawColor(rendu_, 0, 0, 0, 255); // Fond noir
    SDL_RenderClear(rendu_);
}

void GestionSDL::afficher() {
    SDL_RenderPresent(rendu_);
}

void GestionSDL::changer_titre(const std::string& titre) {
    SDL_SetWindowTitle(fenetre_, titre.c_str());
}

int GestionSDL::verifier_entree() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) return 1;
        if (e.type == SDL_KEYDOWN) {
            if (e.key.keysym.sym == SDLK_m) return 2;
        }
    }
    return 0;
}

void GestionSDL::attendre(int ms) {
    SDL_Delay(ms);
}

// --- NOUVELLE PALETTE : MONOCHROME ROUGE ---
void GestionSDL::definir_couleur_temp(double temp, double t_min, double t_max) {
    double ratio = (temp - t_min) / (t_max - t_min);
    if (ratio < 0.0) ratio = 0.0;
    if (ratio > 1.0) ratio = 1.0;

    // Logique :
    // 0% (Froid) -> Gris sombre (50, 50, 50) pour voir le matériau
    // 100% (Chaud) -> Rouge vif (255, 0, 0)
    
    // Interpolation linéaire
    // R passe de 50 à 255
    // G et B passent de 50 à 0 (le gris disparait pour laisser le rouge pur)
    
    Uint8 r = (Uint8)(50 + (205 * ratio)); 
    Uint8 g = (Uint8)(50 - (50 * ratio));
    Uint8 b = (Uint8)(50 - (50 * ratio));

    SDL_SetRenderDrawColor(rendu_, r, g, b, 255);
}

void GestionSDL::dessiner_barre(const std::vector<double>& temp, double t_min, double t_max) {
    int n = temp.size();
    double w_seg = (double)largeur_ / n;
    int h_barre = 100;
    int y = hauteur_ / 2 - h_barre / 2;

    for (int i = 0; i < n; ++i) {
        definir_couleur_temp(temp[i], t_min, t_max);
        SDL_Rect r = {(int)(i * w_seg), y, (int)w_seg + 1, h_barre};
        SDL_RenderFillRect(rendu_, &r);
    }
}

void GestionSDL::dessiner_surface_2d(const std::vector<double>& grille, int N, double t_min, double t_max) {
    double cell_w = (double)largeur_ / N;
    double cell_h = (double)hauteur_ / N;

    for (int i = 0; i < N; ++i) { 
        for (int j = 0; j < N; ++j) { 
            
            definir_couleur_temp(grille[i * N + j], t_min, t_max);
            
            SDL_Rect r;
            r.x = (int)(j * cell_w);
            // Inversion Y pour avoir le bas en bas
            r.y = (int)((N - 1 - i) * cell_h);
            r.w = (int)cell_w + 1;
            r.h = (int)cell_h + 1;
            SDL_RenderFillRect(rendu_, &r);
        }
    }
}