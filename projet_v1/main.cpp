#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <iomanip>
#include <cmath>
#include <cstdio> // Pour printf
#include "materiau.h"
#include "solveur.h"
#include "sdl_gestion.h"

// Structure pour stocker l'historique pour le tableau de fin
struct EtapeLog {
    double temps;
    double t_max;
    double t_moy;
};

// Création du solveur selon le choix
std::unique_ptr<Solveur> creer_solveur(int type_simu, const Materiau& mat) {
    double t_max = 16.0; // Durée fixe de 16 secondes
    double L = 1.0;
    
    if (type_simu == 1) {
        return std::unique_ptr<Solveur>(new Solveur1D(mat, 100, L, t_max));
    } else {
        return std::unique_ptr<Solveur>(new Solveur2D(mat, 60, L, t_max));
    }
}

// Calcul des statistiques à l'instant T
EtapeLog calculer_stats(const Solveur& solveur) {
    const auto& donnees = solveur.get_donnees();
    double temp_max_k = -1.0;
    double somme_temp = 0.0;
    
    for(double t : donnees) {
        if (t > temp_max_k) temp_max_k = t;
        somme_temp += t;
    }
    double temp_moy_k = somme_temp / donnees.size();

    // Retourne en Celsius
    return { solveur.get_temps_actuel(), temp_max_k - 273.15, temp_moy_k - 273.15 };
}

int main(int argc, char* argv[]) {
    (void)argc; (void)argv;

    // --- CONFIGURATION ---
    std::vector<Materiau> biblio;
    biblio.push_back(Materiau("Cuivre", 389.0, 8940.0, 380.0));
    biblio.push_back(Materiau("Aluminium", 237.0, 2700.0, 900.0));
    biblio.push_back(Materiau("Fer", 80.2, 7874.0, 440.0));
    biblio.push_back(Materiau("Verre", 1.2, 2530.0, 840.0));
    biblio.push_back(Materiau("Polystyrene", 0.1, 1040.0, 1200.0));

    std::cout << "=== PROJET CHALEUR (Live Stats + Rapport) ===" << std::endl;
    std::cout << "1. Barre 1D" << std::endl;
    std::cout << "2. Plaque 2D" << std::endl;
    std::cout << "Choix : ";
    int choix_simu;
    std::cin >> choix_simu;

    int index_mat = 0;
    auto solveur = creer_solveur(choix_simu, biblio[index_mat]);
    GestionSDL graphisme("Simulation Thermique", 600, 600);

    // --- VARIABLES DE GESTION ---
    std::vector<EtapeLog> historique; 
    double prochain_log = 0.0;        
    bool simulation_terminee = false;
    bool rapport_affiche = false;

    // État initial
    historique.push_back(calculer_stats(*solveur));
    prochain_log = 1.0;

    std::cout << "\n>>> Materiau : " << biblio[index_mat].get_nom() << " <<<\n";
    std::cout << "----------------------------------------------------------\n";
    std::cout << " MONITORING LIVE \n";
    std::cout << "----------------------------------------------------------\n";

    bool continuer = true;

    // --- BOUCLE PRINCIPALE ---
    while (continuer) {
        // 1. GESTION ENTREES
        int action = graphisme.verifier_entree();
        if (action == 1) continuer = false; 
        else if (action == 2) {
            // REDEMARRAGE (Touche M)
            index_mat = (index_mat + 1) % biblio.size();
            solveur = creer_solveur(choix_simu, biblio[index_mat]);
            
            // Reset complet
            historique.clear();
            historique.push_back(calculer_stats(*solveur));
            prochain_log = 1.0;
            simulation_terminee = false;
            rapport_affiche = false;

            std::cout << "\n\n>>> Changement : " << biblio[index_mat].get_nom() << " <<<\n";
            std::cout << "----------------------------------------------------------\n";
            std::cout << " MONITORING LIVE \n";
            std::cout << "----------------------------------------------------------\n";
        }

        // 2. CALCULS PHYSIQUES
        if (!simulation_terminee) {
            double dt = solveur->get_dt();
            // On calcule assez d'étapes pour que l'animation soit fluide (~0.05s simu / frame)
            int etapes = (int)(0.05 / dt); 
            if (etapes < 1) etapes = 1;

            for(int k=0; k < etapes; ++k) {
                if (solveur->get_temps_actuel() < solveur->get_temps_max()) {
                    solveur->avancer_temps();

                    // Enregistrement historique chaque seconde ronde
                    if (solveur->get_temps_actuel() >= prochain_log) {
                        historique.push_back(calculer_stats(*solveur));
                        prochain_log += 1.0;
                    }
                } else {
                    simulation_terminee = true;
                    // On force l'enregistrement du point final (16s)
                    historique.push_back(calculer_stats(*solveur));
                    break;
                }
            }
        }

        // 3. AFFICHAGE DANS LE TERMINAL
        if (!simulation_terminee) {
            // MODE LIVE : On affiche tout sur la même ligne qui s'écrase (\r)
            auto stats = calculer_stats(*solveur);
            
            printf("\r [LIVE] Temps: %5.2f s | Max: %6.2f C | Moy: %6.2f C      ", 
                   stats.temps, stats.t_max, stats.t_moy);
            
            fflush(stdout); // Important pour forcer l'affichage immédiat
        }
        else if (!rapport_affiche) {
            // MODE RAPPORT (Une fois à la fin)
            
            // On affiche une dernière fois la ligne propre à 16.00s
            auto fin = calculer_stats(*solveur);
            printf("\r [FINI] Temps: 16.00 s | Max: %6.2f C | Moy: %6.2f C      ", 
                   fin.t_max, fin.t_moy);

            std::cout << "\n\n"; 
            std::cout << "======================================================\n";
            std::cout << " RAPPORT FINAL : " << solveur->get_materiau().get_nom() << "\n";
            std::cout << "======================================================\n";
            std::cout << "|  Temps (s)  |  T_Max (C)  |  T_Moy (C)  |\n";
            std::cout << "|-------------|-------------|-------------|\n";
            
            for (const auto& log : historique) {
                printf("|  %5.1f s    |  %8.2f   |  %8.2f   |\n", 
                       log.temps, log.t_max, log.t_moy);
            }
            std::cout << "======================================================\n";
            std::cout << ">>> PRESS 'M' TO CHANGE MATERIAL <<<\n";
            
            rapport_affiche = true;
        }

        // 4. AFFICHAGE GRAPHIQUE (SDL)
        std::string titre;
        if (simulation_terminee) {
            titre = "FINI (16s) - PRESS M - " + solveur->get_materiau().get_nom();
        } else {
            titre = "T=" + std::to_string((int)solveur->get_temps_actuel()) + "s - " + solveur->get_materiau().get_nom();
        }
        
        graphisme.changer_titre(titre);
        graphisme.effacer();
        const auto& donnees = solveur->get_donnees();
        
        // Echelle de couleurs adaptée (13°C à 45°C)
        double t_min = 286.15;
        double t_max = 318.15;

        if (choix_simu == 1) {
            graphisme.dessiner_barre(donnees, t_min, t_max);
        } else {
             auto* s2d = dynamic_cast<Solveur2D*>(solveur.get());
             if (s2d) graphisme.dessiner_surface_2d(donnees, s2d->get_N(), t_min, t_max);
        }

        graphisme.afficher();
        graphisme.attendre(20); 
    }

    return 0;
}