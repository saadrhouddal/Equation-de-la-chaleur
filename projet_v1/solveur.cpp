#include "solveur.h"
#include <cmath>
#include <iostream>

// Constructeur : initialise la géométrie et le maillage
SolveurChaleur::SolveurChaleur(Materiau mat, int nb_points, double L, double t_max)
    : materiau_(mat), nb_points_(nb_points), longueur_(L), t_max_(t_max) {
    
    dx_ = longueur_ / (nb_points_ - 1);
    
    // On veut 100 itérations pour l'animation (consigne)
    // Mais pour la stabilité/précision, on peut en faire plus.
    // Ici on fixe dt pour avoir environ 100 frames sur t_max.
    dt_ = t_max_ / 1000.0; // Plus de pas de calcul que d'affichage pour la précision

    initialiser_conditions_initiales();
}

void SolveurChaleur::initialiser_conditions_initiales() {
    u_actuel_.resize(nb_points_);
    u_suivant_.resize(nb_points_);

    // Condition initiale : Température uniforme de 13°C (286.15 K)
    // Consigne [cite: 4174, 4202]
    double temp_initiale = 13.0 + 273.15; 
    for (int i = 0; i < nb_points_; ++i) {
        u_actuel_[i] = temp_initiale;
    }
}

// Terme source F(x) défini par morceaux [cite: 4198]
double SolveurChaleur::calculer_source_F(double x) {
    double f = 80.0; // Valeur f donnée en Celsius, on l'utilise comme delta
    double val_F = 0.0;

    // Attention aux intervalles donnés [L/10, 2L/10] etc.
    if (x >= longueur_ / 10.0 && x <= 2.0 * longueur_ / 10.0) {
        val_F = t_max_ * f * f;
    } else if (x >= 5.0 * longueur_ / 10.0 && x <= 6.0 * longueur_ / 10.0) {
        val_F = 0.75 * t_max_ * f * f;
    }
    return val_F;
}

const std::vector<double>& SolveurChaleur::get_temperatures() const {
    return u_actuel_;
}

// Algorithme de Thomas pour inverser la matrice tridiagonale
void SolveurChaleur::resoudre_tridiagonal(const std::vector<double>& d) {
    int n = nb_points_;
    std::vector<double> c_prime(n);
    std::vector<double> d_prime(n);

    // Coefficients matriciels
    double lambda = materiau_.get_lambda();
    double rho = materiau_.get_rho();
    double c_p = materiau_.get_c();
    double r = (lambda * dt_) / (rho * c_p * dx_ * dx_);

    // Construction des diagonales A, B, C pour la méthode implicite
    // Équation : -r*u_{i-1} + (1+2r)*u_{i} -r*u_{i+1} = RHS
    
    // Forward sweep
    // Condition Neumann en x=0 (i=0) : u_{-1} = u_{1} => modifie l'équation 0
    // Condition Dirichlet en x=L (i=n-1) : u fixé
    
    // Pour simplifier ici (code simple), on applique Dirichlet aux deux bouts pour l'algo Thomas standard, 
    // puis on corrigera pour Neumann ou on adapte les coeffs.
    // Adaptation Neumann i=0: (1+2r)u_0 - 2r u_1 = RHS_0 (car u_-1 = u_1)
    
    std::vector<double> a(n, -r);       // Diag inférieure
    std::vector<double> b_mat(n, 1.0 + 2.0 * r); // Diag principale
    std::vector<double> c(n, -r);       // Diag supérieure

    // Correction Neumann à gauche (x=0)
    c[0] = -2.0 * r; 
    
    // Correction Dirichlet à droite (x=L): T est fixe à T_init
    // On force l'équation u_{N-1} = T_init -> b=1, a=0, c=0, d=T_init
    a[n-1] = 0.0;
    b_mat[n-1] = 1.0;
    c[n-1] = 0.0; // Hors bornes mais pour la forme

    // Forward elimination
    c_prime[0] = c[0] / b_mat[0];
    d_prime[0] = d[0] / b_mat[0];

    for (int i = 1; i < n; i++) {
        double temp = b_mat[i] - a[i] * c_prime[i - 1];
        if (i < n - 1) {
            c_prime[i] = c[i] / temp;
        }
        d_prime[i] = (d[i] - a[i] * d_prime[i - 1]) / temp;
    }

    // Back substitution
    u_suivant_[n - 1] = d_prime[n - 1];
    for (int i = n - 2; i >= 0; i--) {
        u_suivant_[i] = d_prime[i] - c_prime[i] * u_suivant_[i + 1];
    }
}

void SolveurChaleur::avancer_temps() {
    std::vector<double> rhs(nb_points_);
    double rho = materiau_.get_rho();
    double c_p = materiau_.get_c();

    // Construction du membre de droite (RHS) : u_n + source
    for (int i = 0; i < nb_points_; ++i) {
        double x = i * dx_;
        double terme_source = (calculer_source_F(x) / (rho * c_p)) * dt_;
        
        rhs[i] = u_actuel_[i] + terme_source;
    }

    // Force la condition de Dirichlet sur le RHS pour le dernier point
    // u(t, L) = u0 = 286.15 K
    rhs[nb_points_ - 1] = 13.0 + 273.15;

    resoudre_tridiagonal(rhs);

    // Mise à jour
    u_actuel_ = u_suivant_;
}