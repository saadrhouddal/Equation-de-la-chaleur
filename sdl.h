#ifndef SDL_H
#define SDL_H

#include <SDL2/SDL.h>
#include <vector>

/**
 * @class Sdl
 * @brief Classe pour gérer l'affichage des températures avec SDL.
 */
class Sdl {
public:
    /**
     * @brief Constructeur de la classe Sdl.
     * @param width Largeur de la fenêtre.
     * @param height Hauteur de la fenêtre.
     */
    Sdl(int width, int height);

    /**
     * @brief Destructeur de la classe Sdl.
     */
    ~Sdl();

    /**
     * @brief Affiche les températures sur la fenêtre SDL.
     * @param temperatures Vecteur des températures.
     */
    void render(const std::vector<double>& temperatures);

private:
    int width; ///< Largeur de la fenêtre.
    int height; ///< Hauteur de la fenêtre.
    SDL_Window* window; ///< Pointeur vers la fenêtre SDL.
    SDL_Renderer* renderer; ///< Pointeur vers le renderer SDL.
};

#endif // SDL_H
