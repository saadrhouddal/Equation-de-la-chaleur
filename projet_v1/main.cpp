#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <iomanip>
#include <cmath>
#include <cstdio>
#include "materiau.h"
#include "solveur.h"
#include "sdl_gestion.h"

// Structure pour stocker l'historique des stats
struct EtapeLog {
    double temps;
    double t_max;
    double t_moy;
};

// Fonction factory pour créer le solveur avec les bons paramètres
std::unique_ptr<Solveur> creer_solveur(int type_simu, const Materiau& mat) {
    double t_max = 16.0; // Durée totale simulée (selon sujet)
    double L = 1.0;      // Longueur 1m (selon sujet)
    
    // --- CORRECTION DU SUJET : 1001 POINTS ---
    // Le sujet exige 1001 points de maillage.
    // Grâce à la méthode ADI (Implicite), c'est calculable en temps raisonnable.
    int nb_points = 1001; 

    if (type_simu == 1) {
        return std::unique_ptr<Solveur>(new Solveur1D(mat, nb_points, L, t_max));
    } else {
        return std::unique_ptr<Solveur>(new Solveur2D(mat, nb_points, L, t_max));
    }
}

EtapeLog calculer_stats(const Solveur& solveur) {
    const auto& donnees = solveur.get_donnees();
    double temp_max_k = -1.0;
    double somme_temp = 0.0;
    
    // Calcul Min/Max/Moyenne
    for(double t : donnees) {
        if (t > temp_max_k) temp_max_k = t;
        somme_temp += t;
    }
    
    double temp_moy_k = somme_temp / donnees.size();
    
    // Conversion Kelvin -> Celsius pour l'affichage
    return { solveur.get_temps_actuel(), temp_max_k - 273.15, temp_moy_k - 273.15 };
}

int main(int argc, char* argv[]) {
    (void)argc; (void)argv; // Évite les warnings variables inutilisées

    // --- Configuration initiale ---
    // Largeur fenêtre augmentée pour mieux voir les détails de la grille fine
    GestionSDL graphisme("Projet Chaleur", 800, 800); 
    
    // Définition des matériaux (Sujet page 3)
    Materiau cuivre("Cuivre", 389.0, 8940.0, 380.0);
    Materiau fer("Fer", 80.2, 7874.0, 440.0);
    Materiau verre("Verre", 1.2, 2530.0, 840.0);
    Materiau polystyrene("Polystyrene", 0.1, 1040.0, 1200.0);
    
    Materiau mat_courant = cuivre; // Par défaut
    
    int choix_simu = 0;
    std::cout << "Choisir simulation : [1] 1D (Barre) ou [2] 2D (Plaque) : ";
    std::cin >> choix_simu;
    if (choix_simu != 1 && choix_simu != 2) choix_simu = 1;

    // Création initiale du solveur
    auto solveur = creer_solveur(choix_simu, mat_courant);
    
    bool quitter = false;
    bool simulation_terminee = false;
    bool rapport_affiche = false;
    
    std::vector<EtapeLog> historique;
    int compteur_affichage = 0;

    // --- Boucle Principale ---
    while (!quitter) {
        // 1. Gestion des Entrées (Clavier)
        int action = graphisme.verifier_entree();
        if (action == 1) {
            quitter = true;
        } else if (action == 2) { 
            // Touche 'M' pressée -> Changement de matériau
            if (mat_courant.get_nom() == "Cuivre") mat_courant = fer;
            else if (mat_courant.get_nom() == "Fer") mat_courant = verre;
            else if (mat_courant.get_nom() == "Verre") mat_courant = polystyrene;
            else mat_courant = cuivre;
            
            // On redémarre la simulation avec le nouveau matériau
            solveur = creer_solveur(choix_simu, mat_courant);
            historique.clear();
            simulation_terminee = false;
            rapport_affiche = false;
            std::cout << "\n>>> Changement materiau : " << mat_courant.get_nom() << " <<<\n";
        }

        // 2. Physique (Avancer le temps)
        if (!simulation_terminee) {
            if (solveur->get_temps_actuel() < solveur->get_temps_max()) {
                solveur->avancer_temps();
                
                // Enregistrement stats (optionnel : ne pas le faire à chaque pas pour perf)
                if (compteur_affichage % 10 == 0) { 
                     historique.push_back(calculer_stats(*solveur));
                }
            } else {
                simulation_terminee = true;
            }
        }

        // 3. Affichage (Rendu)
        // OPTIMISATION : On ne redessine que 1 fois toutes les 5 itérations de calcul
        // Sinon avec 1 million de points (1001x1001), l'affichage ralentit trop le calcul.
        compteur_affichage++;
        if (compteur_affichage % 5 == 0 || simulation_terminee) {
            
            // Sortie console des résultats finaux une seule fois
            if (simulation_terminee && !rapport_affiche) {
                std::cout << "\n=== RAPPORT FINAL (" << mat_courant.get_nom() << ") ===\n";
                std::cout << "|  Temps (s)  |  T_Max (C)  |  T_Moy (C)  |\n";
                std::cout << "|-------------|-------------|-------------|\n";
                
                // On affiche quelques points de l'historique pour ne pas saturer la console
                size_t pas_log = std::max((size_t)1, historique.size() / 20);
                for (size_t i = 0; i < historique.size(); i += pas_log) {
                    const auto& log = historique[i];
                    printf("|  %5.1f s    |  %8.2f   |  %8.2f   |\n", 
                           log.temps, log.t_max, log.t_moy);
                }
                // Afficher la toute dernière étape
                if (!historique.empty()) {
                     const auto& log = historique.back();
                     printf("|  %5.1f s    |  %8.2f   |  %8.2f   |\n", 
                           log.temps, log.t_max, log.t_moy);
                }

                std::cout << "======================================================\n";
                std::cout << ">>> APPUYEZ SUR 'M' POUR CHANGER DE MATERIAU <<<\n";
                rapport_affiche = true;
            }

            // Mise à jour Titre Fenêtre
            std::string titre;
            if (simulation_terminee) titre = "FINI (16s) - PRESS M";
            else titre = "T=" + std::to_string((int)solveur->get_temps_actuel()) + "s - " + solveur->get_materiau().get_nom();
            
            graphisme.changer_titre(titre);
            graphisme.effacer();
            
            const auto& donnees = solveur->get_donnees();
            
            // Échelle de couleurs pour la visualisation
            // Ajustez ces valeurs si vous voulez voir des contrastes différents
            // Le sujet impose T_init = 13°C (286.15 K)
            double t_visu_min = 286.15;
            // On met un max à 15°C (288.15 K) pour bien voir la diffusion initiale
            // Car la chaleur diffuse lentement dans le verre/polystyrène
            double t_visu_max = 288.15; 

            if (choix_simu == 1) {
                graphisme.dessiner_barre(donnees, t_visu_min, t_visu_max);
            } else {
                // Utilisation du cast dynamique pour récupérer N si on est en 2D
                // Note: nécessite que get_N() soit ajouté dans solveur.h comme vu précédemment
                Solveur2D* s2d = dynamic_cast<Solveur2D*>(solveur.get());
                if (s2d) {
                    graphisme.dessiner_surface_2d(donnees, s2d->get_N(), t_visu_min, t_visu_max);
                }
            }
            
            graphisme.afficher();
        }
        
        // Petite pause pour ne pas surcharger le CPU si la simulation est finie
        if (simulation_terminee) graphisme.attendre(100);
        else graphisme.attendre(1); // Très court délai pour laisser SDL respirer
    }

    return 0;
}