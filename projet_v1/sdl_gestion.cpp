#include "sdl_gestion.h"
#include <iostream>
#include <algorithm> // pour std::max et std::min

GestionSDL::GestionSDL(std::string titre, int largeur, int hauteur) 
    : largeur_(largeur), hauteur_(hauteur) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Erreur SDL Init: " << SDL_GetError() << std::endl;
        exit(1);
    }
    fenetre_ = SDL_CreateWindow(titre.c_str(), SDL_WINDOWPOS_CENTERED, 
                                SDL_WINDOWPOS_CENTERED, largeur, hauteur, 0);
    rendu_ = SDL_CreateRenderer(fenetre_, -1, SDL_RENDERER_ACCELERATED);
}

GestionSDL::~GestionSDL() {
    SDL_DestroyRenderer(rendu_);
    SDL_DestroyWindow(fenetre_);
    SDL_Quit();
}

void GestionSDL::effacer() {
    SDL_SetRenderDrawColor(rendu_, 0, 0, 0, 255); // Noir
    SDL_RenderClear(rendu_);
}

void GestionSDL::afficher() {
    SDL_RenderPresent(rendu_);
}

bool GestionSDL::verifier_evenement_quitter() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) return true;
    }
    return false;
}

void GestionSDL::attendre(int millisecondes) {
    SDL_Delay(millisecondes);
}

// Fonction pour dessiner la barre.
// Chaque segment de la barre aura une couleur correspondant à sa température.
void GestionSDL::dessiner_barre(const std::vector<double>& temp, double t_min, double t_max) {
    int nb_points = temp.size();
    // Largeur d'un segment à l'écran
    double largeur_segment = (double)largeur_ / nb_points;
    int hauteur_barre = 100; // Hauteur visuelle de la barre
    int y_pos = hauteur_ / 2 - hauteur_barre / 2;

    for (int i = 0; i < nb_points; ++i) {
        // Normalisation de la température entre 0.0 et 1.0
        double ratio = (temp[i] - t_min) / (t_max - t_min);
        ratio = std::max(0.0, std::min(1.0, ratio));

        // Couleur : Bleu (froid) -> Rouge (chaud)
        Uint8 r = (Uint8)(255 * ratio);
        Uint8 b = (Uint8)(255 * (1.0 - ratio));
        Uint8 g = 0; // On garde simple : transition bleu/rouge

        SDL_SetRenderDrawColor(rendu_, r, g, b, 255);
        
        SDL_Rect rect;
        rect.x = (int)(i * largeur_segment);
        rect.y = y_pos;
        rect.w = (int)largeur_segment + 1; // +1 pour éviter les trous
        rect.h = hauteur_barre;

        SDL_RenderFillRect(rendu_, &rect);
    }
}