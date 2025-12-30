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

// --- PALETTE FEU : BLEU -> JAUNE -> ORANGE -> ROUGE ---
void GestionSDL::definir_couleur_temp(double temp, double t_min, double t_max) {
    double ratio = (temp - t_min) / (t_max - t_min);
    if (ratio < 0.0) ratio = 0.0;
    if (ratio > 1.0) ratio = 1.0;

    Uint8 r = 0, g = 0, b = 0;

    if (ratio <= 0.33) {
        // Tiers 1 : De Bleu (0,0,255) à Jaune (255,255,0)
        double local = ratio / 0.33;
        r = (Uint8)(255 * local);
        g = (Uint8)(255 * local);
        b = (Uint8)(255 * (1.0 - local));
    } 
    else if (ratio <= 0.66) {
        // Tiers 2 : De Jaune (255,255,0) à Orange (255,128,0)
        double local = (ratio - 0.33) / 0.33;
        r = 255;
        g = (Uint8)(255 - (127 * local)); 
        b = 0;
    } 
    else {
        // Tiers 3 : De Orange (255,128,0) à Rouge (255,0,0)
        double local = (ratio - 0.66) / 0.34;
        r = 255;
        g = (Uint8)(128 * (1.0 - local));
        b = 0;
    }

    // Affichage gris foncé pour la température ambiante exacte (pour voir les murs froids)
    if (std::abs(temp - 286.15) < 0.1) {
       r = 40; g = 40; b = 40;
    }

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

// --- CORRECTION AXE Y ---
void GestionSDL::dessiner_surface_2d(const std::vector<double>& grille, int N, double t_min, double t_max) {
    double cell_w = (double)largeur_ / N;
    double cell_h = (double)hauteur_ / N;

    for (int i = 0; i < N; ++i) { // i est l'indice de ligne (Y mathématique)
        for (int j = 0; j < N; ++j) { // j est l'indice de colonne (X mathématique)
            
            definir_couleur_temp(grille[i * N + j], t_min, t_max);
            
            SDL_Rect r;
            r.x = (int)(j * cell_w);
            
            // INVERSION ICI : 
            // En SDL, Y=0 est en haut. En physique, Y=0 est en bas.
            // On dessine la ligne i=0 tout en bas de la fenêtre.
            r.y = (int)((N - 1 - i) * cell_h);
            
            r.w = (int)cell_w + 1;
            r.h = (int)cell_h + 1;
            SDL_RenderFillRect(rendu_, &r);
        }
    }
}