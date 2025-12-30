#ifndef SOLVEUR_HPP
#define SOLVEUR_HPP

#include <vector>
#include "materiau.h"

class SolveurChaleur {
private:
    Materiau materiau_;
    int nb_points_;
    double longueur_;
    double t_max_;
    double dt_; // Pas de temps
    double dx_; // Pas d'espace
    
    // Température actuelle (u^n) et future (u^{n+1})
    std::vector<double> u_actuel_;
    std::vector<double> u_suivant_;

    // Coefficients pour la matrice tridiagonale (Méthode implicite)
    std::vector<double> diag_a_; // Diagonale inférieure
    std::vector<double> diag_b_; // Diagonale principale
    std::vector<double> diag_c_; // Diagonale supérieure

    // Initialise les vecteurs et les conditions initiales
    void initialiser_conditions_initiales();

    // Calcule la source de chaleur F(x)
    double calculer_source_F(double x);

    // Résolution du système tridiagonal (Algorithme de Thomas)
    void resoudre_tridiagonal(const std::vector<double>& source_term);

public:
    // Constructeur
    SolveurChaleur(Materiau mat, int nb_points, double L, double t_max);

    // Effectue un pas de temps de simulation
    void avancer_temps();

    // Récupère les températures pour l'affichage
    const std::vector<double>& get_temperatures() const;

    // Retourne le temps max total
    double get_temps_total() const { return t_max_; }
    double get_dt() const { return dt_; }
};

#endif