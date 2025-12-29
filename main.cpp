#include <SDL2/SDL.h>
#include <iostream>
#include <iomanip>
#include "Modele1D.h"
#include "Modele2D.h"
#include "EqChaleur.h"
#include "Materiau.h"
#include "sdl.h" 
#include <vector>

int main(int, char**) {
    double L = 1.0; // Longueur en mètres
    double tmax = 16.0; // Temps maximum en secondes
    double u0 = 13.0; // Température initiale en degrés Celsius
    double f = 80.0 + 273.15; // Intensité de la source de chaleur
    double dt = 0.1; // Pas de temps
    int n_points = 101; // Nombre de points dans la simulation 1D
    int n_points_2D = 20; // Points en 2D pour simplifier l'affichage

    std::vector<std::pair<Materiau, std::string>> materiaux = {
        {Materiau(Materiau::TypeMateriau::Cuivre), "Cuivre"},
        {Materiau(Materiau::TypeMateriau::Fer), "Fer"},
        {Materiau(Materiau::TypeMateriau::Verre), "Verre"},
        {Materiau(Materiau::TypeMateriau::Polystyrene), "Polystyrène"}
    };

    // Simulation pour chaque matériau
    for (const auto& [mat, nom] : materiaux) {
        std::cout << "\n=============================================\n";
        std::cout << "--- Début de la simulation pour le matériau : " << nom << " ---\n";
        std::cout << "=============================================\n";
        SDL_Delay(4000);
        Modele1D modele1D(mat, f, L, tmax, n_points, u0);
        Sdl sdl1D(1250, 600);

        bool running = true; // Variable pour contrôler la boucle principale
        SDL_Event event;

        for (double t = 0; t <= tmax && running; t += dt) {
            // Gestion des événements
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) {
                    running = false;
                }
            }

            modele1D.resoudre(dt);
            std::vector<double> temperatures1D;
            std::cout << "Temps = " << std::fixed << std::setprecision(2) << t << " s\n";
            for (int i = 0.0 ; i < n_points; i+=n_points / 10) {
                double x = i * L / (n_points - 1);
                std::cout << "Température à x = " << std::fixed << std::setprecision(4) << x
                          << " m : " << modele1D.getTemperatureAtPoint(x) - 273.15 << " °C\n";
                temperatures1D.push_back(modele1D.getTemperatureAtPoint(x)- 273.15);
            }
            sdl1D.render(temperatures1D);
            SDL_Delay(100);
        }

        std::cout << "Simulation 1D terminée pour le matériau : " << nom << "\n";
        SDL_Delay(3200);
    }

    char reponse;
    std::cout << "Voulez-vous lancer la simulation 2D pour tous les matériaux ? (o/n) : ";
    std::cin >> reponse;
    if (reponse == 'o' || reponse == 'O') {
        for (const auto& [mat, nom] : materiaux) {
            std::cout << "=============================================\n";
            std::cout << "--- Début de la simulation 2D pour le matériau : " << nom << " ---\n";
            std::cout << "=============================================\n";
            SDL_Delay(4000);
            Modele2D modele2D(mat, f, L, tmax, n_points_2D, n_points_2D, u0);
            Sdl sdl2D(1250, 600);
            bool running = true; // Variable pour contrôler la boucle principale
            SDL_Event event;
            for (double t = 0.0; t <= tmax && running; t += dt) {
                // Gestion des événements
                while (SDL_PollEvent(&event)) {
                    if (event.type == SDL_QUIT) {
                        running = false;
                    }
                }
                modele2D.resoudre(dt);

                std::vector<double> temperatures2D;
                std::cout << "Temps = " << std::fixed << std::setprecision(2) << t << " s\n";

                // Affichage des températures avec correction des valeurs aberrantes
                for (int i = 0; i < n_points_2D; i += n_points_2D / 5) {
                    for (int j = 0; j < n_points_2D; j += n_points_2D / 5) {
                        double x = i * L / (n_points_2D - 1);
                        double y = j * L / (n_points_2D - 1);
                        double temperature = modele2D.getTemperatureAtPoint(x, y) - 273.15;

                        // Correction des valeurs aberrantes (apparition de quelques valeurs illogiques (température négative et très basse))
                        if (temperature < 0.0) {
                            temperature = 13.0;
                        }

                        std::cout << "Température à (x, y) = (" << std::fixed << std::setprecision(4) << x
                                  << ", " << y << ") : " << temperature << " °C\n";
                        temperatures2D.push_back(temperature);
                    }
                }
                sdl2D.render(temperatures2D);
                SDL_Delay(100);
                std::cout << "=============================================\n";
            }
            SDL_Delay(3200);
        }
    }

    return 0;
}
