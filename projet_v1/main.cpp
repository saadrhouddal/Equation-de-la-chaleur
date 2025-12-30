#include <iostream>
#include <vector>
#include "materiau.h"
#include "solveur.h"
#include "sdl_gestion.h"

// Tableau 2 : Propriétés des matériaux 
Materiau creer_cuivre() { return Materiau("Cuivre", 389.0, 8940.0, 380.0); }
Materiau creer_fer() { return Materiau("Fer", 80.2, 7874.0, 440.0); }
Materiau creer_verre() { return Materiau("Verre", 1.2, 2530.0, 840.0); }
Materiau creer_polystyrene() { return Materiau("Polystyrene", 0.1, 1040.0, 1200.0); }

int main(int argc, char* argv[]) {
    (void)argc; (void)argv; // Évite les warnings variables inutilisées

    // Paramètres numériques [cite: 4199, 4202]
    int nb_points = 1001;
    double longueur = 1.0; // mètres
    double temps_max = 16.0; // secondes

    // Choix du matériau (on peut changer ici pour tester les autres)
    Materiau mat = creer_cuivre();
    std::cout << "Simulation pour : " << mat.get_nom() << std::endl;

    // Création du solveur et de l'interface graphique
    SolveurChaleur solveur(mat, nb_points, longueur, temps_max);
    GestionSDL graphisme("Projet Chaleur 1D", 800, 600);

    // Calcul du nombre total d'itérations
    // Le solveur a un dt interne. On veut afficher 100 frames.
    double dt_solveur = solveur.get_dt();
    int total_iterations = (int)(temps_max / dt_solveur);
    int iterations_par_frame = total_iterations / 100; // Pour avoir 100 images 

    bool continuer = true;
    int iter = 0;

    // Boucle principale
    while (continuer && iter < total_iterations) {
        // Gestion sortie
        if (graphisme.verifier_evenement_quitter()) {
            continuer = false;
        }

        // Calculs physiques (plusieurs petits pas pour une frame affichée)
        for (int k = 0; k < iterations_par_frame; ++k) {
            solveur.avancer_temps();
            iter++;
        }

        // Affichage
        graphisme.effacer();
        
        // On définit une échelle de couleur fixe pour voir l'évolution
        // Min: 286.15K (13°C), Max approx: 353K (80°C + 13°C) pour visualiser l'apport
        graphisme.dessiner_barre(solveur.get_temperatures(), 286.15, 360.0);
        
        graphisme.afficher();

        // Petit délai pour que l'animation soit visible à l'oeil humain
        graphisme.attendre(20); 
    }

    // Attendre avant de fermer à la fin
    if (continuer) {
        std::cout << "Simulation terminée. Appuyez sur la croix pour quitter." << std::endl;
        while (!graphisme.verifier_evenement_quitter()) {
            graphisme.attendre(100);
        }
    }

    return 0;
}