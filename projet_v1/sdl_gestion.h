#ifndef SDL_GESTION_HPP
#define SDL_GESTION_HPP

#include <SDL2/SDL.h>
#include <vector>
#include <string>

class GestionSDL {
private:
    SDL_Window* fenetre_;
    SDL_Renderer* rendu_;
    int largeur_;
    int hauteur_;

public:
    // Constructeur : initialise la SDL et crée la fenêtre
    GestionSDL(std::string titre, int largeur, int hauteur);
    
    // Destructeur : nettoie la mémoire SDL
    ~GestionSDL();

    // Efface l'écran en noir
    void effacer();

    // Met à jour l'écran (présente le rendu)
    void afficher();

    // Dessine la barre 1D avec des couleurs selon la température
    void dessiner_barre(const std::vector<double>& temperatures, double t_min, double t_max);

    // Gère les événements (pour fermer la fenêtre)
    bool verifier_evenement_quitter();
    
    // Pause simple
    void attendre(int millisecondes);
};

#endif