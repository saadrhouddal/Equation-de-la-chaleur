#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <algorithm>
#include <iomanip> 
#include "materiau.h"
#include "solveur.h"
#include "sdl_gestion.h"


struct EtapeLog {
    double temps;
    double t_max;
    double t_moy;
};


std::unique_ptr<Solveur> fabriquer_solveur(int choix, const Materiau& mat) {
    double t_max = 16.0;
    double L = 1.0;
    int nb_points = 1001;
    if (choix == 2) return std::unique_ptr<Solveur>(new Solveur2D(mat, nb_points, L, t_max));
    return std::unique_ptr<Solveur>(new Solveur1D(mat, nb_points, L, t_max));
}

Materiau cycle_materiau(const Materiau& m) {
    if (m.get_nom() == "Cuivre") return Materiau("Fer", 80.2, 7874.0, 440.0);
    if (m.get_nom() == "Fer") return Materiau("Verre", 1.2, 2530.0, 840.0);
    if (m.get_nom() == "Verre") return Materiau("Polystyrene", 0.1, 1040.0, 1200.0);
    return Materiau("Cuivre", 389.0, 8940.0, 380.0);
}

EtapeLog enregistrer_stats(const Solveur& solveur, double& max_temp_globale) {
    const auto& donnees = solveur.get_donnees();
    double t_max = -1.0;
    double somme = 0.0;
    for(double t : donnees) {
        if (t > t_max) t_max = t;
        somme += t;
    }
    double t_max_c = t_max - 273.15;
    if (t_max_c > max_temp_globale) max_temp_globale = t_max_c;
    return { solveur.get_temps_actuel(), t_max_c, (somme / donnees.size()) - 273.15 };
}

void afficher_rapport(const std::vector<EtapeLog>& historique, const std::string& nom_mat) {
    std::cout << "\n=== RAPPORT FINAL (" << nom_mat << ") ===\n";
    std::cout << "|  Temps (s)  |  T_Max (C)  |  T_Moy (C)  |\n";
    std::cout << "|-------------|-------------|-------------|\n";
    
    size_t pas = std::max((size_t)1, historique.size() / 20);
    
    // Configuration du formatage pour les nombres flottants
    std::cout << std::fixed; 

    for (size_t i = 0; i < historique.size(); i += pas) {
        std::cout << "|  " 
                  << std::setw(5) << std::setprecision(1) << historique[i].temps << " s    |  "
                  << std::setw(8) << std::setprecision(2) << historique[i].t_max << "   |  "
                  << std::setw(8) << std::setprecision(2) << historique[i].t_moy << "   |\n";
    }
    
    if (!historique.empty()) {
        const auto& last = historique.back();
        std::cout << "|  " 
                  << std::setw(5) << std::setprecision(1) << last.temps << " s    |  "
                  << std::setw(8) << std::setprecision(2) << last.t_max << "   |  "
                  << std::setw(8) << std::setprecision(2) << last.t_moy << "   |\n";
    }
    
    std::cout.unsetf(std::ios_base::floatfield); 
    
    std::cout << "======================================================\n";
    std::cout << ">>> APPUYEZ SUR 'M' POUR CHANGER DE MATERIAU <<<\n";
}

void mettre_a_jour_affichage(Sdl& sdl, Solveur* solv, int type_simu, double max_temp) {
    (void)type_simu; // Pour éviter le warning "unused parameter" si on n'utilise pas type_simu ici
    
    std::string titre = "T=" + std::to_string((int)solv->get_temps_actuel()) + "s - " + solv->get_materiau().get_nom();
    if (solv->get_temps_actuel() >= solv->get_temps_max()) titre = "FINI - " + solv->get_materiau().get_nom() + " - PRESS M";
    
    sdl.changer_titre(titre);
    sdl.effacer();
    
    double t_min = 286.15; // 13°C
    double t_max = max_temp + 273.15;
    if (t_max < t_min + 2.0) t_max = t_min + 2.0;

    // Détection automatique du type via dynamic_cast
    auto* s2d = dynamic_cast<Solveur2D*>(solv);
    if (s2d) {
        sdl.dessiner_surface_2d(solv->get_donnees(), s2d->get_N(), t_min, t_max);
    } else {
        sdl.dessiner_barre(solv->get_donnees(), t_min, t_max);
    }
    sdl.afficher();
}

void gerer_rapport_final(const Solveur& solv, std::vector<EtapeLog>& hist, 
                         double max_temp, bool& rapport_fait) {
    hist.push_back(enregistrer_stats(solv, max_temp));
    afficher_rapport(hist, solv.get_materiau().get_nom());
    rapport_fait = true;
}

// --- MAIN ---
int main(int argc, char* argv[]) {
    (void)argc; (void)argv;
    Sdl graphisme("Projet Chaleur", 800, 800);
    
    Materiau mat_courant("Cuivre", 389.0, 8940.0, 380.0);
    int choix = 0;
    
    std::cout << "Simulation : [1] 1D (Barre) ou [2] 2D (Plaque) : ";
    std::cin >> choix;
    if (choix != 2) choix = 1;
    
    auto solveur = fabriquer_solveur(choix, mat_courant);
    std::vector<EtapeLog> historique;
    
    double max_temp_globale = 15.0;
    bool fini = false, rapport_fait = false, quitter = false;
    
    while (!quitter) {
        int action = graphisme.verifier_entree();
        if (action == 1) quitter = true;
        if (action == 2) {
            mat_courant = cycle_materiau(mat_courant);
            solveur = fabriquer_solveur(choix, mat_courant);
            historique.clear(); 
            max_temp_globale = 15.0; 
            fini = false; 
            rapport_fait = false;
        }
        
        long long freq = (long long)((solveur->get_temps_max()/solveur->get_dt()) / 100);
        if (freq < 1) freq = 1;
        
        if (!fini && solveur->get_temps_actuel() < solveur->get_temps_max() - (solveur->get_dt()/2)) {
            solveur->avancer_temps();
            if (solveur->get_compteur_pas() % freq == 0) 
                historique.push_back(enregistrer_stats(*solveur, max_temp_globale));
        } else {
            fini = true;
            if (!rapport_fait) gerer_rapport_final(*solveur, historique, max_temp_globale, rapport_fait);
        }
        
        if ((solveur->get_compteur_pas() % freq == 0) || fini) 
            mettre_a_jour_affichage(graphisme, solveur.get(), choix, max_temp_globale);
        
        graphisme.attendre(fini ? 100 : 1);
    }
    return 0;
}