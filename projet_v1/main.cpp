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
    double t_max = 16.0; 
    double L = 1.0;      
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
    return { solveur.get_temps_actuel(), temp_max_k - 273.15, temp_moy_k - 273.15 };
}

int main(int argc, char* argv[]) {
    (void)argc; (void)argv; 

    // Configuration initiale
    GestionSDL graphisme("Projet Chaleur", 800, 800); 
    
    // Définition des matériaux
    Materiau cuivre("Cuivre", 389.0, 8940.0, 380.0);
    Materiau fer("Fer", 80.2, 7874.0, 440.0);
    Materiau verre("Verre", 1.2, 2530.0, 840.0);
    Materiau polystyrene("Polystyrene", 0.1, 1040.0, 1200.0);
    
    Materiau mat_courant = cuivre; 
    
    int choix_simu = 0;
    std::cout << "Choisir simulation : [1] 1D (Barre) ou [2] 2D (Plaque) : ";
    std::cin >> choix_simu;
    if (choix_simu != 1 && choix_simu != 2) choix_simu = 1;

    // Création initiale du solveur
    auto solveur = creer_solveur(choix_simu, mat_courant);
    
    // --- CALCUL DE LA FREQUENCE D'AFFICHAGE ---
    // On veut 100 valeurs réparties uniformément.
    double dt = solveur->get_dt();
    double total_temps = solveur->get_temps_max();
    long long total_pas = (long long)(total_temps / dt);
    
    long long frequence_affichage = total_pas / 100; 
    if (frequence_affichage < 1) frequence_affichage = 1;

    bool quitter = false;
    bool simulation_terminee = false;
    bool rapport_affiche = false;
    
    std::vector<EtapeLog> historique;
    
    // Température max globale pour adapter l'échelle de couleur
    double max_temp_globale = 15.0; // Valeur min pour commencer

    // --- Boucle Principale ---
    while (!quitter) {
        // 1. Gestion des Entrées
        int action = graphisme.verifier_entree();
        if (action == 1) {
            quitter = true;
        } else if (action == 2) { 
            // Touche 'M' : Changement de matériau
            if (mat_courant.get_nom() == "Cuivre") mat_courant = fer;
            else if (mat_courant.get_nom() == "Fer") mat_courant = verre;
            else if (mat_courant.get_nom() == "Verre") mat_courant = polystyrene;
            else mat_courant = cuivre;
            
            solveur = creer_solveur(choix_simu, mat_courant);
            
            // Recalcul fréquence car dt change selon le solveur (1D vs 2D)
            dt = solveur->get_dt();
            total_pas = (long long)(solveur->get_temps_max() / dt);
            frequence_affichage = total_pas / 100;
            if (frequence_affichage < 1) frequence_affichage = 1;

            historique.clear();
            simulation_terminee = false;
            rapport_affiche = false;
            max_temp_globale = 15.0;
            std::cout << "\n>>> Changement materiau : " << mat_courant.get_nom() << " <<<\n";
        }

        // 2. Physique
        if (!simulation_terminee) {
            // On avance tant qu'on n'a pas dépassé t_max
            if (solveur->get_temps_actuel() < solveur->get_temps_max() - (dt/2.0)) { 
                solveur->avancer_temps();
                
                // Enregistrement stats au moment de l'affichage
                if (solveur->get_compteur_pas() % frequence_affichage == 0) { 
                     EtapeLog log = calculer_stats(*solveur);
                     historique.push_back(log);
                     if (log.t_max > max_temp_globale) max_temp_globale = log.t_max;
                }
            } else {
                simulation_terminee = true;
                // Forcer une dernière stat à la fin pour être sûr d'avoir t=16s
                historique.push_back(calculer_stats(*solveur));
            }
        }

        // 3. Affichage
        // On affiche uniquement si c'est le moment (pour avoir 100 images) ou si fini
        if ((solveur->get_compteur_pas() % frequence_affichage == 0) || simulation_terminee) {
            
            // --- RESTAURATION DU RAPPORT FINAL ---
            if (simulation_terminee && !rapport_affiche) {
                std::cout << "\n=== RAPPORT FINAL (" << mat_courant.get_nom() << ") ===\n";
                std::cout << "|  Temps (s)  |  T_Max (C)  |  T_Moy (C)  |\n";
                std::cout << "|-------------|-------------|-------------|\n";
                
                // On affiche environ 20 lignes réparties sur l'historique pour ne pas saturer le terminal
                size_t pas_log = std::max((size_t)1, historique.size() / 20);
                
                for (size_t i = 0; i < historique.size(); i += pas_log) {
                    const auto& log = historique[i];
                    printf("|  %5.1f s    |  %8.2f   |  %8.2f   |\n", 
                           log.temps, log.t_max, log.t_moy);
                }
                
                // Afficher explicitement la toute dernière étape (t=16s)
                if (!historique.empty()) {
                     const auto& log = historique.back();
                     // Si la dernière n'a pas déjà été affichée par la boucle
                     if (historique.size() % pas_log != 1) { 
                        printf("|  %5.1f s    |  %8.2f   |  %8.2f   |\n", 
                               log.temps, log.t_max, log.t_moy);
                     }
                }

                std::cout << "======================================================\n";
                std::cout << ">>> APPUYEZ SUR 'M' POUR CHANGER DE MATERIAU <<<\n";
                rapport_affiche = true;
            }

            // Mise à jour Titre Fenêtre
            std::string titre;
            if (simulation_terminee) titre = "FINI - " + solveur->get_materiau().get_nom() + " - PRESS M";
            else titre = "T=" + std::to_string((int)solveur->get_temps_actuel()) + "s - " + solveur->get_materiau().get_nom();
            
            graphisme.changer_titre(titre);
            graphisme.effacer();
            
            const auto& donnees = solveur->get_donnees();
            
            // --- ECHELLE DE COULEUR DYNAMIQUE ---
            double t_visu_min = 286.15; // 13°C
            double t_visu_max = (max_temp_globale + 273.15);
            // Contraste minimum de 2 degrés
            if (t_visu_max < t_visu_min + 2.0) t_visu_max = t_visu_min + 2.0;

            if (choix_simu == 1) {
                graphisme.dessiner_barre(donnees, t_visu_min, t_visu_max);
            } else {
                Solveur2D* s2d = dynamic_cast<Solveur2D*>(solveur.get());
                if (s2d) {
                    graphisme.dessiner_surface_2d(donnees, s2d->get_N(), t_visu_min, t_visu_max);
                }
            }
            
            graphisme.afficher();
        }
        
        // Temporisation
        if (simulation_terminee) graphisme.attendre(100);
        else graphisme.attendre(1); 
    }

    return 0;
}