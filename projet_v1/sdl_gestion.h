#ifndef SDL_GESTION_H
#define SDL_GESTION_H

#include <SDL2/SDL.h>
#include <vector>
#include <string>

class GestionSDL {
private:
    SDL_Window* fenetre_;
    SDL_Renderer* rendu_;
    int largeur_;
    int hauteur_;

    void definir_couleur_temp(double temp, double t_min, double t_max);

public:
    GestionSDL(std::string titre, int largeur, int hauteur);
    ~GestionSDL();

    void effacer();
    void afficher();
    
    // Renvoie : 0 (rien), 1 (quitter), 2 (touche M pressée -> changer matériau)
    int verifier_entree();
    
    void attendre(int millisecondes);

    // Titre dynamique pour afficher le nom du matériau
    void changer_titre(const std::string& nouveau_titre);

    void dessiner_barre(const std::vector<double>& temperatures, double t_min, double t_max);
    void dessiner_surface_2d(const std::vector<double>& grille, int nb_points_cote, double t_min, double t_max);
};

#endif