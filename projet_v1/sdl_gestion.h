#ifndef SDL_GESTION_H
#define SDL_GESTION_H

#include <SDL2/SDL.h>
#include <vector>
#include <string>

/**
 * @brief Classe gérant l'affichage graphique via la bibliothèque SDL2.
 * * Cette classe encapsule la création de la fenêtre, du renderer, et 
 * fournit des primitives de dessin pour visualiser la température en 1D et 2D.
 */
class GestionSDL {
private:
    SDL_Window* fenetre_;
    SDL_Renderer* rendu_;
    int largeur_;
    int hauteur_;

    /**
     * @brief Définit la couleur de dessin en fonction d'une température.
     * * Utilise un dégradé (Bleu -> Rouge) pour représenter la chaleur.
     * * @param temp Température actuelle de la cellule.
     * @param t_min Température minimale de l'échelle (bleu).
     * @param t_max Température maximale de l'échelle (rouge).
     */
    void definir_couleur_temp(double temp, double t_min, double t_max);

public:
    /**
     * @brief Constructeur : Initialise la SDL et crée la fenêtre.
     * @param titre Titre initial de la fenêtre.
     * @param largeur Largeur de la fenêtre en pixels.
     * @param hauteur Hauteur de la fenêtre en pixels.
     */
    GestionSDL(std::string titre, int largeur, int hauteur);

    /**
     * @brief Destructeur : Libère la mémoire et quitte la SDL.
     */
    ~GestionSDL();

    /**
     * @brief Efface le contenu de la fenêtre (remplit en noir).
     */
    void effacer();

    /**
     * @brief Met à jour l'écran avec le rendu actuel (double buffering).
     */
    void afficher();
    
    /**
     * @brief Vérifie les événements utilisateur (clavier, fermeture).
     * @return int Code d'action :
     * 0 : Aucun événement spécial.
     * 1 : Demande de fermeture (croix ou Alt+F4).
     * 2 : Touche 'M' pressée (demande de changement de matériau).
     */
    int verifier_entree();
    
    /**
     * @brief Met le programme en pause.
     * @param millisecondes Durée d'attente en millisecondes.
     */
    void attendre(int millisecondes);

    /**
     * @brief Modifie le titre de la fenêtre (utilisé pour afficher le temps et le matériau).
     * @param nouveau_titre Le nouveau titre à appliquer.
     */
    void changer_titre(const std::string& nouveau_titre);

    /**
     * @brief Dessine la visualisation 1D (Barre).
     * @param temperatures Vecteur contenant les températures le long de la barre.
     * @param t_min Température min pour l'échelle de couleur.
     * @param t_max Température max pour l'échelle de couleur.
     */
    void dessiner_barre(const std::vector<double>& temperatures, double t_min, double t_max);

    /**
     * @brief Dessine la visualisation 2D (Plaque).
     * @param grille Vecteur linéarisé contenant les températures de la grille (taille N*N).
     * @param nb_points_cote Nombre de points N sur un côté de la grille carrée.
     * @param t_min Température min pour l'échelle de couleur.
     * @param t_max Température max pour l'échelle de couleur.
     */
    void dessiner_surface_2d(const std::vector<double>& grille, int nb_points_cote, double t_min, double t_max);
};

#endif