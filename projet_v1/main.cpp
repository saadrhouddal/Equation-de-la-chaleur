#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <iomanip> // Pour l'affichage propre
#include "materiau.h"
#include "solveur.h"
#include "sdl_gestion.h"

// Fonction pour créer un solveur
std::unique_ptr<Solveur> creer_solveur(int type_simu, const Materiau& mat) {
    double t_max = 300.0; // Durée longue
    double L = 1.0;
    
    if (type_simu == 1) {
        return std::unique_ptr<Solveur>(new Solveur1D(mat, 100, L, t_max));
    } else {
        return std::unique_ptr<Solveur>(new Solveur2D(mat, 60, L, t_max));
    }
}

int main(int argc, char* argv[]) {
    (void)argc; (void)argv;

    // Bibliothèque de matériaux
    std::vector<Materiau> biblio;
    biblio.push_back(Materiau("Cuivre (Tres Conducteur)", 389.0, 8940.0, 380.0));
    biblio.push_back(Materiau("Aluminium (Conducteur)", 237.0, 2700.0, 900.0));
    biblio.push_back(Materiau("Fer (Moyen)", 80.2, 7874.0, 440.0));
    biblio.push_back(Materiau("Verre (Isolant)", 1.2, 2530.0, 840.0));
    biblio.push_back(Materiau("Polystyrene (Tres Isolant)", 0.1, 1040.0, 1200.0));

    // Menu
    std::cout << "=== PROJET CHALEUR AVANCÉ ===" << std::endl;
    std::cout << "1. Barre 1D" << std::endl;
    std::cout << "2. Plaque 2D" << std::endl;
    std::cout << "Choix : ";
    int choix_simu;
    std::cin >> choix_simu;

    int index_mat = 0;
    auto solveur = creer_solveur(choix_simu, biblio[index_mat]);
    
    GestionSDL graphisme("Simulation Thermique", 600, 600);

    // Entête du tableau dans le terminal
    std::cout << "\n-------------------------------------------------------------\n";
    std::cout << "|   Temps (s)   |   T_max (C)   |   T_moy (C)   |  Materiau  |\n";
    std::cout << "-------------------------------------------------------------\n";

    bool continuer = true;

    while (continuer) {
        int action = graphisme.verifier_entree();
        if (action == 1) continuer = false; 
        else if (action == 2) {
            index_mat = (index_mat + 1) % biblio.size();
            solveur = creer_solveur(choix_simu, biblio[index_mat]);
            std::cout << "\n>>> Changement Matériau : " << biblio[index_mat].get_nom() << " <<<\n\n";
        }

        // Titre fenêtre
        graphisme.changer_titre("Mat: " + solveur->get_materiau().get_nom() + 
                                " | 'M' pour changer");

        // --- CALCULS PHYSIQUES ACCÉLÉRÉS ---
        // On calcule beaucoup de pas de temps pour que l'affichage soit fluide
        double dt = solveur->get_dt();
        int nombre_etapes = (int)(0.1 / dt); 
        if (nombre_etapes < 10) nombre_etapes = 10;
        if (nombre_etapes > 5000) nombre_etapes = 5000;

        for(int k=0; k < nombre_etapes; ++k) {
             solveur->avancer_temps();
        }

        // --- ANALYSE DES DONNEES POUR LE TERMINAL ---
        const auto& donnees = solveur->get_donnees();
        
        // Recherche Max et Moyenne
        double temp_max_k = -1.0;
        double somme_temp = 0.0;
        
        for(double t : donnees) {
            if (t > temp_max_k) temp_max_k = t;
            somme_temp += t;
        }
        double temp_moy_k = somme_temp / donnees.size();

        // Conversion Kelvin -> Celsius
        double max_c = temp_max_k - 273.15;
        double moy_c = temp_moy_k - 273.15;

        // Affichage console formaté
        // \r permet d'écraser la ligne précédente pour faire un effet "compteur"
        // std::flush force l'affichage immédiat
        printf("\r|  %9.2f s  |  %9.2f C  |  %9.2f C  |  %-10s", 
               solveur->get_temps_actuel(), 
               max_c, 
               moy_c,
               solveur->get_materiau().get_nom().substr(0, 10).c_str());
        std::cout << std::flush;


        // --- AFFICHAGE GRAPHIQUE ---
        graphisme.effacer();
        
        // Echelle de couleurs : 13°C (Bleu/Gris) à 40°C (Rouge vif)
        // On garde une échelle serrée pour bien voir la diffusion
        graphisme.dessiner_barre(donnees, 286.15, 313.15); // En 1D ou 2D la fonction gère
        if (choix_simu == 2) {
             auto* s2d = dynamic_cast<Solveur2D*>(solveur.get());
             if (s2d) graphisme.dessiner_surface_2d(donnees, s2d->get_N(), 286.15, 313.15);
        }

        graphisme.afficher();
        graphisme.attendre(20); 
    }
    
    std::cout << "\n\nSimulation terminee.\n";
    return 0;
}