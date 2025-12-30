#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include "materiau.h"
#include "solveur.h"
#include "sdl_gestion.h"

// Fonction pour créer un solveur (1D ou 2D) avec un matériau donné
std::unique_ptr<Solveur> creer_solveur(int type_simu, const Materiau& mat) {
    double t_max = 16.0;
    double L = 1.0;
    
    if (type_simu == 1) {
        return std::unique_ptr<Solveur>(new Solveur1D(mat, 100, L, t_max));
    } else {
        // En 2D explicite, attention aux matériaux très conducteurs (ex: Or)
        // Le calcul sera plus lent car le pas de temps sera minuscule.
        return std::unique_ptr<Solveur>(new Solveur2D(mat, 50, L, t_max));
    }
}

int main(int argc, char* argv[]) {
    (void)argc; (void)argv;

    // --- 1. Définition de la bibliothèque de matériaux ---
    std::vector<Materiau> biblio;
    biblio.push_back(Materiau("Cuivre (Conducteur)", 389.0, 8940.0, 380.0));
    biblio.push_back(Materiau("Fer (Moyen)", 80.2, 7874.0, 440.0));
    biblio.push_back(Materiau("Verre (Isolant)", 1.2, 2530.0, 840.0));
    biblio.push_back(Materiau("Polystyrene (Tres Isolant)", 0.1, 1040.0, 1200.0));
    biblio.push_back(Materiau("Or (Tres Conducteur)", 317.0, 19300.0, 129.0));
    biblio.push_back(Materiau("Beton", 1.5, 2300.0, 880.0));

    // --- 2. Menu de démarrage ---
    std::cout << "=== PROJET CHALEUR MULTI-MATERIAUX ===" << std::endl;
    std::cout << "1. Mode 1D (Barre)" << std::endl;
    std::cout << "2. Mode 2D (Plaque)" << std::endl;
    std::cout << "Votre choix : ";
    int choix_simu;
    std::cin >> choix_simu;

    int index_mat = 0; // On commence par le premier (Cuivre)
    
    // Création initiale
    auto solveur = creer_solveur(choix_simu, biblio[index_mat]);
    
    GestionSDL graphisme("Simulation", 600, 600);
    
    std::cout << "\nCommandes :" << std::endl;
    std::cout << " - Appuyez sur 'M' pour changer de materiau." << std::endl;
    std::cout << " - Appuyez sur la croix pour quitter." << std::endl;

    bool continuer = true;

    while (continuer) {
        // Gestion des événements
        int action = graphisme.verifier_entree();
        
        if (action == 1) { 
            continuer = false; 
        }
        else if (action == 2) {
            // Touche 'M' pressée : on passe au matériau suivant
            index_mat = (index_mat + 1) % biblio.size();
            Materiau mat_suivant = biblio[index_mat];
            
            std::cout << ">>> Changement : " << mat_suivant.get_nom() << std::endl;
            
            // On remplace l'objet solveur par un tout neuf (reset temps et temp)
            solveur = creer_solveur(choix_simu, mat_suivant);
        }

        // Mise à jour titre fenêtre avec info temps et matériau
        std::string info = "Mat: " + solveur->get_materiau().get_nom() + 
                           " | Temps: " + std::to_string((int)solveur->get_temps_actuel()) + "s";
        graphisme.changer_titre(info);

        // Calculs physiques (plusieurs pas par frame)
        // On adapte le nombre de pas de calcul par affichage selon le dt
        // pour garder une vitesse d'animation visible
        double dt = solveur->get_dt();
        int pas_par_frame = (int)(0.05 / dt); // Vise 0.05s simulée par frame
        if (pas_par_frame < 1) pas_par_frame = 1;

        for(int k=0; k<pas_par_frame; ++k) {
             solveur->avancer_temps();
        }

        // Affichage
        graphisme.effacer();
        const auto& donnees = solveur->get_donnees();
        
        double t_min = 286.15; // 13°C
        double t_max = 373.15; // 100°C

        if (choix_simu == 1) {
            graphisme.dessiner_barre(donnees, t_min, t_max);
        } else {
            auto* s2d = dynamic_cast<Solveur2D*>(solveur.get());
            if (s2d) {
                graphisme.dessiner_surface_2d(donnees, s2d->get_N(), t_min, t_max);
            }
        }

        graphisme.afficher();
        graphisme.attendre(20); 
    }

    return 0;
}