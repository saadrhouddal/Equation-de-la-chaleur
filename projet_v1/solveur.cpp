#include "solveur.h"
#include <cmath>
#include <iostream>
#include <algorithm>

// =========================================================
// IMPLÉMENTATION 1D (Reste inchangée - Implicite classique)
// =========================================================

Solveur1D::Solveur1D(Materiau mat, int nb_points, double L, double t_max)
    : Solveur(mat, L, t_max), nb_points_(nb_points) {
    dx_ = longueur_ / (nb_points_ - 1);
    dt_ = t_max_ / 1000.0; 
    u_.assign(nb_points_, 13.0 + 273.15);
    u_next_ = u_;
}

double Solveur1D::source_F(double x) {
    double f = 80.0; 
    if (x >= 0.1 * longueur_ && x <= 0.2 * longueur_) return t_max_ * f * f;
    if (x >= 0.5 * longueur_ && x <= 0.6 * longueur_) return 0.75 * t_max_ * f * f;
    return 0.0;
}

void Solveur1D::avancer_temps() {
    double alpha = materiau_.get_diffusivite();
    double r = (alpha * dt_) / (dx_ * dx_);
    int n = nb_points_;
    
    // Construction du système tridiagonal
    std::vector<double> a(n, -r), b(n, 1.0 + 2.0*r), c(n, -r), rhs(n);

    // Conditions aux limites intégrées
    // Gauche (Neumann) : u0 - u1 = 0 => (1+r)u0 - r u1 = ...
    b[0] = 1.0 + r; c[0] = -r; a[0] = 0.0; 
    
    // Droite (Dirichlet) : u = T_init
    b[n-1] = 1.0; a[n-1] = 0.0; c[n-1] = 0.0;

    double const_src = dt_ / (materiau_.get_rho() * materiau_.get_c());

    for(int i=0; i<n; ++i) {
        rhs[i] = u_[i] + source_F(i*dx_) * const_src;
    }
    rhs[n-1] = 13.0 + 273.15; // Force Dirichlet value RHS

    // Algo Thomas simplifié intégré ici pour la 1D
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
    // On prend un pas de temps plus grand pour que l'animation soit fluide et visible.
    dt_ = t_max_ / 400.0; 

    // Initialisation Uniforme (13°C en Kelvin)
    u_.assign(N_ * N_, 13.0 + 273.15);
    u_next_ = u_;
    u_demi_ = u_; // Initialisation du buffer intermédiaire
}

double Solveur2D::source_F(double x, double y) {
    // Définition des zones de chauffe selon le PDF (simplifié)
    // L/6 approx 0.16, 2L/6 approx 0.33, etc.
    bool zone_x1 = (x >= 1.0/6.0 && x <= 2.0/6.0);
    bool zone_x2 = (x >= 4.0/6.0 && x <= 5.0/6.0);
    bool zone_y1 = (y >= 1.0/6.0 && y <= 2.0/6.0);
    bool zone_y2 = (y >= 4.0/6.0 && y <= 5.0/6.0);
    
    // Valeur source très chaude
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

    // Descente (Forward elimination)
    c_prime[0] = c_sup[0] / b_diag[0];
    d_prime[0] = d_rhs[0] / b_diag[0];

    for (int i = 1; i < n; i++) {
        double temp = b_diag[i] - a_inf[i] * c_prime[i - 1];
        if (i < n - 1) {
            c_prime[i] = c_sup[i] / temp;
        }
        d_prime[i] = (d_rhs[i] - a_inf[i] * d_prime[i - 1]) / temp;
    }

    // Remontée (Back substitution)
    x_sol[n - 1] = d_prime[n - 1];
    for (int i = n - 2; i >= 0; i--) {
        x_sol[i] = d_prime[i] - c_prime[i] * x_sol[i + 1];
    }
}

void Solveur2D::avancer_temps() {
    double alpha = materiau_.get_diffusivite();
    
    // Facteur r pour un demi-pas de temps (dt/2)
    double r = (alpha * dt_ / 2.0) / (dx_ * dx_);
    
    // Constante source thermique (pour un demi-pas)
    double source_coeff = (dt_ / 2.0) / (materiau_.get_rho() * materiau_.get_c());
    
    double T_bord = 13.0 + 273.15; // Température imposée (Dirichlet)

    // Vecteurs pour stocker les coefficients de la matrice tridiagonale
    std::vector<double> diag_inf(N_);
    std::vector<double> diag(N_);
    std::vector<double> diag_sup(N_);
    std::vector<double> second_membre(N_);
    std::vector<double> resultat_ligne(N_);

    // ---------------------------------------------------------
    // ÉTAPE 1 : Implicite en X (Lignes), Explicite en Y
    // ---------------------------------------------------------
    
    // On traite chaque ligne j (sauf les bords Y fixes)
    for (int j = 1; j < N_ - 1; ++j) {
        
        for (int i = 0; i < N_; ++i) {
            double src = source_F(i * dx_, j * dx_) * source_coeff;
            
            // Diffusion explicite verticale (axe Y) à partir de u_ actuel
            double diff_y = r * (u_[i * N_ + (j - 1)] - 2.0 * u_[i * N_ + j] + u_[i * N_ + (j + 1)]);
            
            // Second membre
            second_membre[i] = u_[i * N_ + j] + diff_y + src;

            // Coefficients matrice implicite (axe X)
            diag_inf[i] = -r;
            diag[i]     = 1.0 + 2.0 * r;
            diag_sup[i] = -r;
        }

        // --- Conditions aux limites pour la ligne j ---
        
        // Bord Gauche (i=0) : Neumann (Flux nul)
        // (1 + 2r)u0 - 2r u1 = RHS
        diag[0] = 1.0 + 2.0 * r;
        diag_sup[0] = -2.0 * r;
        diag_inf[0] = 0.0; 

        // Bord Droit (i=N-1) : Dirichlet (Température fixe)
        diag[N_ - 1] = 1.0;
        diag_inf[N_ - 1] = 0.0;
        diag_sup[N_ - 1] = 0.0;
        second_membre[N_ - 1] = T_bord;

        // Résolution du système tridiagonal
        resoudre_thomas(N_, diag_inf, diag, diag_sup, second_membre, resultat_ligne);

        // Stockage dans le buffer intermédiaire u_demi_
        for (int i = 0; i < N_; ++i) {
            u_demi_[i * N_ + j] = resultat_ligne[i];
        }
    }
    
    // Appliquer Dirichlet sur les bords Y (Haut et Bas) pour l'étape intermédiaire
    for (int i = 0; i < N_; ++i) {
        u_demi_[i * N_ + 0]      = T_bord;
        u_demi_[i * N_ + (N_-1)] = T_bord;
    }

    // ---------------------------------------------------------
    // ÉTAPE 2 : Explicite en X, Implicite en Y (Colonnes)
    // ---------------------------------------------------------

    std::vector<double> resultat_colonne(N_);

    // On traite chaque colonne i (y compris les bords X car ils évoluent selon Y)
    for (int i = 0; i < N_; ++i) {
        
        for (int j = 0; j < N_; ++j) {
            double src = source_F(i * dx_, j * dx_) * source_coeff;
            
            // Diffusion explicite horizontale (axe X) à partir de u_demi_
            // Gestion des voisins i-1 et i+1
            double val_gauche = (i > 0)      ? u_demi_[(i - 1) * N_ + j] : u_demi_[1 * N_ + j]; // Miroir pour Neumann
            double val_droite = (i < N_ - 1) ? u_demi_[(i + 1) * N_ + j] : T_bord;              // Fixe pour Dirichlet
            double val_centre = u_demi_[i * N_ + j];

            double diff_x = r * (val_gauche - 2.0 * val_centre + val_droite);

            // Second membre
            second_membre[j] = val_centre + diff_x + src;

            // Coefficients matrice implicite (axe Y)
            diag_inf[j] = -r;
            diag[j]     = 1.0 + 2.0 * r;
            diag_sup[j] = -r;
        }

        // --- Conditions aux limites pour la colonne i ---
        
        // Bord Haut (j=0) : Dirichlet
        diag[0] = 1.0; diag_sup[0] = 0.0; diag_inf[0] = 0.0; second_membre[0] = T_bord;

        // Bord Bas (j=N-1) : Dirichlet
        diag[N_-1] = 1.0; diag_inf[N_-1] = 0.0; diag_sup[N_-1] = 0.0; second_membre[N_-1] = T_bord;

        // Résolution
        resoudre_thomas(N_, diag_inf, diag, diag_sup, second_membre, resultat_colonne);

        // Stockage final dans u_next_
        for (int j = 0; j < N_; ++j) {
            u_next_[i * N_ + j] = resultat_colonne[j];
        }
    }

    // Mise à jour finale
    u_ = u_next_;
    temps_actuel_ += dt_;
}