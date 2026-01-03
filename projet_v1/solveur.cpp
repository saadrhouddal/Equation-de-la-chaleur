#include "solveur.h"
#include <cmath>
#include <iostream>
#include <algorithm>

// =========================================================
// IMPLÉMENTATION 1D (Implicite)
// =========================================================

Solveur1D::Solveur1D(Materiau mat, int nb_points, double L, double t_max)
    : Solveur(mat, L, t_max), nb_points_(nb_points) {
    dx_ = longueur_ / (nb_points_ - 1);
    // Pas de temps pour 1D (1000 pas suffisent pour être fluide et précis)
    dt_ = t_max_ / 1000.0; 
    u_.assign(nb_points_, 13.0 + 273.15);
    u_next_ = u_;
}

double Solveur1D::source_F(double x) {
    double f = 80.0; 
    
    // --- CORRECTION : Utilisation des bornes exactes du sujet (L/6, 2L/6, etc.) ---
    // Au lieu de 0.1, 0.2 qui étaient approximatifs.
    
    double l_6 = longueur_ / 6.0;
    
    // Zone 1 : [L/6, 2L/6]
    if (x >= l_6 && x <= 2.0 * l_6) return t_max_ * f * f;
    
    // Zone 2 : [4L/6, 5L/6] (Symétrique, supposé selon logique du sujet)
    // Le code précédent mettait 0.5-0.6, on corrige pour être cohérent avec la 2D
    if (x >= 4.0 * l_6 && x <= 5.0 * l_6) return 0.75 * t_max_ * f * f;
    
    return 0.0;
}

void Solveur1D::avancer_temps() {
    double alpha = materiau_.get_diffusivite();
    double r = (alpha * dt_) / (dx_ * dx_);
    int n = nb_points_;
    
    // Construction du système tridiagonal
    std::vector<double> a(n, -r), b(n, 1.0 + 2.0*r), c(n, -r), rhs(n);

    // --- CORRECTION : Condition de Neumann à l'ordre 2 (Point fantôme) ---
    // Avant : b[0] = 1.0 + r (Ordre 1)
    // Après : b[0] = 1.0 + 2r (Ordre 2) -> Plus précis
    b[0] = 1.0 + 2.0 * r; 
    c[0] = -2.0 * r; 
    a[0] = 0.0; 
    
    // Droite (Dirichlet) : u = T_init
    b[n-1] = 1.0; a[n-1] = 0.0; c[n-1] = 0.0;

    double const_src = dt_ / (materiau_.get_rho() * materiau_.get_c());

    for(int i=0; i<n; ++i) {
        rhs[i] = u_[i] + source_F(i*dx_) * const_src;
    }
    
    // Condition Dirichlet au bord droit (valeur imposée dans le second membre)
    rhs[n-1] = 13.0 + 273.15; 

    // Algo Thomas simplifié (tridiagonal solver)
    std::vector<double> cp(n), dp(n);
    cp[0] = c[0]/b[0];
    dp[0] = rhs[0]/b[0];
    for(int i=1; i<n; i++) {
        double temp = b[i] - a[i]*cp[i-1];
        cp[i] = c[i] / temp;
        dp[i] = (rhs[i] - a[i]*dp[i-1])/temp;
    }
    u_next_[n-1] = dp[n-1];
    for(int i=n-2; i>=0; i--) {
        u_next_[i] = dp[i] - cp[i]*u_next_[i+1];
    }

    u_ = u_next_;
    temps_actuel_ += dt_;
}

// =========================================================
// IMPLÉMENTATION 2D (Méthode ADI - Implicite)
// =========================================================

Solveur2D::Solveur2D(Materiau mat, int nb_points, double L, double t_max)
    : Solveur(mat, L, t_max), N_(nb_points) {
    
    dx_ = longueur_ / (N_ - 1);
    
    // Avec ADI (Implicite), on est inconditionnellement stable.
    dt_ = t_max_ / 400.0; 

    // Initialisation Uniforme (13°C en Kelvin)
    u_.assign(N_ * N_, 13.0 + 273.15);
    u_next_ = u_;
    u_demi_ = u_; 
}

double Solveur2D::source_F(double x, double y) {
    // Définition des zones de chauffe selon le PDF (Fractions exactes)
    double l_6 = longueur_ / 6.0;

    bool zone_x1 = (x >= l_6 && x <= 2.0 * l_6);
    bool zone_x2 = (x >= 4.0 * l_6 && x <= 5.0 * l_6);
    bool zone_y1 = (y >= l_6 && y <= 2.0 * l_6);
    bool zone_y2 = (y >= 4.0 * l_6 && y <= 5.0 * l_6);
    
    double val = t_max_ * 6400.0; 

    if ((zone_x1 && zone_y1) || (zone_x2 && zone_y1) || 
        (zone_x1 && zone_y2) || (zone_x2 && zone_y2)) {
        return val;
    }
    return 0.0;
}

// Algorithme de Thomas (Générique)
void Solveur2D::resoudre_thomas(int n, 
                                const std::vector<double>& a_inf, 
                                const std::vector<double>& b_diag, 
                                const std::vector<double>& c_sup, 
                                const std::vector<double>& d_rhs, 
                                std::vector<double>& x_sol) {
    std::vector<double> c_prime(n);
    std::vector<double> d_prime(n);

    // Descente
    c_prime[0] = c_sup[0] / b_diag[0];
    d_prime[0] = d_rhs[0] / b_diag[0];

    for (int i = 1; i < n; i++) {
        double temp = b_diag[i] - a_inf[i] * c_prime[i - 1];
        if (i < n - 1) {
            c_prime[i] = c_sup[i] / temp;
        }
        d_prime[i] = (d_rhs[i] - a_inf[i] * d_prime[i - 1]) / temp;
    }

    // Remontée
    x_sol[n - 1] = d_prime[n - 1];
    for (int i = n - 2; i >= 0; i--) {
        x_sol[i] = d_prime[i] - c_prime[i] * x_sol[i + 1];
    }
}

void Solveur2D::avancer_temps() {
    double alpha = materiau_.get_diffusivite();
    
    // Facteur r pour un demi-pas de temps (dt/2)
    double r = (alpha * dt_ / 2.0) / (dx_ * dx_);
    
    double source_coeff = (dt_ / 2.0) / (materiau_.get_rho() * materiau_.get_c());
    
    double T_bord = 13.0 + 273.15; 

    std::vector<double> diag_inf(N_);
    std::vector<double> diag(N_);
    std::vector<double> diag_sup(N_);
    std::vector<double> second_membre(N_);
    std::vector<double> resultat_ligne(N_);

    // ---------------------------------------------------------
    // ÉTAPE 1 : Implicite en X (Lignes), Explicite en Y
    // ---------------------------------------------------------
    
    for (int j = 1; j < N_ - 1; ++j) {
        
        for (int i = 0; i < N_; ++i) {
            double src = source_F(i * dx_, j * dx_) * source_coeff;
            
            // Diffusion explicite Y
            double diff_y = r * (u_[i * N_ + (j - 1)] - 2.0 * u_[i * N_ + j] + u_[i * N_ + (j + 1)]);
            
            second_membre[i] = u_[i * N_ + j] + diff_y + src;

            diag_inf[i] = -r;
            diag[i]     = 1.0 + 2.0 * r;
            diag_sup[i] = -r;
        }

        // CL Ligne j
        // Bord Gauche (Neumann ordre 2)
        diag[0] = 1.0 + 2.0 * r;
        diag_sup[0] = -2.0 * r;
        diag_inf[0] = 0.0; 

        // Bord Droit (Dirichlet)
        diag[N_ - 1] = 1.0;
        diag_inf[N_ - 1] = 0.0;
        diag_sup[N_ - 1] = 0.0;
        second_membre[N_ - 1] = T_bord;

        resoudre_thomas(N_, diag_inf, diag, diag_sup, second_membre, resultat_ligne);

        for (int i = 0; i < N_; ++i) {
            u_demi_[i * N_ + j] = resultat_ligne[i];
        }
    }
    
    // Dirichlet Haut/Bas étape intermédiaire
    for (int i = 0; i < N_; ++i) {
        u_demi_[i * N_ + 0]      = T_bord;
        u_demi_[i * N_ + (N_-1)] = T_bord;
    }

    // ---------------------------------------------------------
    // ÉTAPE 2 : Explicite en X, Implicite en Y (Colonnes)
    // ---------------------------------------------------------

    std::vector<double> resultat_colonne(N_);

    for (int i = 0; i < N_; ++i) {
        
        for (int j = 0; j < N_; ++j) {
            double src = source_F(i * dx_, j * dx_) * source_coeff;
            
            // Diffusion explicite X
            double val_gauche = (i > 0)      ? u_demi_[(i - 1) * N_ + j] : u_demi_[1 * N_ + j]; // Miroir pour Neumann
            double val_droite = (i < N_ - 1) ? u_demi_[(i + 1) * N_ + j] : T_bord;              
            double val_centre = u_demi_[i * N_ + j];

            double diff_x = r * (val_gauche - 2.0 * val_centre + val_droite);

            second_membre[j] = val_centre + diff_x + src;

            diag_inf[j] = -r;
            diag[j]     = 1.0 + 2.0 * r;
            diag_sup[j] = -r;
        }

        // CL Colonne i
        diag[0] = 1.0; diag_sup[0] = 0.0; diag_inf[0] = 0.0; second_membre[0] = T_bord;
        diag[N_-1] = 1.0; diag_inf[N_-1] = 0.0; diag_sup[N_-1] = 0.0; second_membre[N_-1] = T_bord;

        resoudre_thomas(N_, diag_inf, diag, diag_sup, second_membre, resultat_colonne);

        for (int j = 0; j < N_; ++j) {
            u_next_[i * N_ + j] = resultat_colonne[j];
        }
    }

    u_ = u_next_;
    temps_actuel_ += dt_;
}