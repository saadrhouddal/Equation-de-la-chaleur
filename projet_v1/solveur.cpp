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
    
    // 1000 pas de temps pour 1D.
    
    dt_ = t_max_ / 1000.0; 
    
    u_.assign(nb_points_, 13.0 + 273.15); // [cite: 47] Température initiale 13°C
    u_next_ = u_;
}

double Solveur1D::source_F(double x) {
    double f = 80.0; 
    
    // --- CORRECTION MAJEURE --- 
    // Le sujet  spécifie des zones basées sur L/10, pas L/6.
    double l_10 = longueur_ / 10.0;
    
    // Zone 1 : [L/10, 2L/10] -> F = t_max * f^2
    if (x >= l_10 && x <= 2.0 * l_10) return t_max_ * f * f;
    
    // Zone 2 : [5L/10, 6L/10] -> F = 3/4 * t_max * f^2
    if (x >= 5.0 * l_10 && x <= 6.0 * l_10) return 0.75 * t_max_ * f * f;
    
    return 0.0;
}

void Solveur1D::avancer_temps() {
    double alpha = materiau_.get_diffusivite();
    double r = (alpha * dt_) / (dx_ * dx_);
    int n = nb_points_;
    
    std::vector<double> a(n, -r), b(n, 1.0 + 2.0*r), c(n, -r), rhs(n);

    // Condition Neumann (x=0) : Ordre 2
    b[0] = 1.0 + 2.0 * r; 
    c[0] = -2.0 * r; 
    a[0] = 0.0; 
    
    // Condition Dirichlet (x=L)
    b[n-1] = 1.0; a[n-1] = 0.0; c[n-1] = 0.0;

    double const_src = dt_ / (materiau_.get_rho() * materiau_.get_c());

    for(int i=0; i<n; ++i) {
        rhs[i] = u_[i] + source_F(i*dx_) * const_src;
    }
    
    // Imposition Dirichlet à droite (13°C)
    rhs[n-1] = 13.0 + 273.15; 

    // Algo Thomas
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
    compteur_pas_++;
}

// =========================================================
// IMPLÉMENTATION 2D (Méthode ADI - Implicite)
// =========================================================

Solveur2D::Solveur2D(Materiau mat, int nb_points, double L, double t_max)
    : Solveur(mat, L, t_max), N_(nb_points) {
    
    dx_ = longueur_ / (N_ - 1);
    
    
    dt_ = t_max_ / 1000.0; 
    
    u_.assign(N_ * N_, 13.0 + 273.15);
    u_next_ = u_;
    u_demi_ = u_; 
    
    // --- OPTIMISATION ---
    // Pré-allocation des vecteurs de travail pour éviter la réallocation dans la boucle
    diag_inf_.resize(N_);
    diag_.resize(N_);
    diag_sup_.resize(N_);
    rhs_.resize(N_);
    result_.resize(N_);
}

double Solveur2D::source_F(double x, double y) {
    // Le sujet [cite: 88] utilise des divisions par 6 pour la plaque
    double l_6 = longueur_ / 6.0;
    bool zone_x1 = (x >= l_6 && x <= 2.0 * l_6);
    bool zone_x2 = (x >= 4.0 * l_6 && x <= 5.0 * l_6);
    bool zone_y1 = (y >= l_6 && y <= 2.0 * l_6);
    bool zone_y2 = (y >= 4.0 * l_6 && y <= 5.0 * l_6);
    
    double val = t_max_ * 80.0 * 80.0; // f^2 = 6400
    
    // Zones en "damier"
    if ((zone_x1 && zone_y1) || (zone_x2 && zone_y1) || 
        (zone_x1 && zone_y2) || (zone_x2 && zone_y2)) return val;
        
    return 0.0;
}

void Solveur2D::resoudre_thomas(int n, 
                                const std::vector<double>& a_inf, 
                                const std::vector<double>& b_diag, 
                                const std::vector<double>& c_sup, 
                                const std::vector<double>& d_rhs, 
                                std::vector<double>& x_sol) {
    // Note: Pour une optimisation extrême, ces vecteurs aussi devraient être membres de classe
    // Mais ils sont petits (1D), donc moins critique que les matrices principales.
    static std::vector<double> c_prime, d_prime;
    if ((int)c_prime.size() != n) { c_prime.resize(n); d_prime.resize(n); }

    c_prime[0] = c_sup[0] / b_diag[0];
    d_prime[0] = d_rhs[0] / b_diag[0];

    for (int i = 1; i < n; i++) {
        double temp = b_diag[i] - a_inf[i] * c_prime[i - 1];
        if (i < n - 1) c_prime[i] = c_sup[i] / temp;
        d_prime[i] = (d_rhs[i] - a_inf[i] * d_prime[i - 1]) / temp;
    }

    x_sol[n - 1] = d_prime[n - 1];
    for (int i = n - 2; i >= 0; i--) {
        x_sol[i] = d_prime[i] - c_prime[i] * x_sol[i + 1];
    }
}

// ... (Haut du fichier inchangé jusqu'à resoudre_thomas) ...

// DECOUPAGE DE LA FONCTION POUR RESPECTER LA LIMITE DE 50 LIGNES 
void Solveur2D::etape_1_x_implicite(double r, double source_coeff) {
    double T_bord = 13.0 + 273.15;

    // Balayage des lignes j. j=0 (Neumann) à j=N-2. j=N-1 est Dirichlet (fixe).
    for (int j = 0; j < N_ - 1; ++j) {
        for (int i = 0; i < N_; ++i) {
            double src = source_F(i * dx_, j * dx_) * source_coeff;
            
            // Diffusion Explicite Y (gestion Neumann j=0 via ghost point simulé)
            double u_haut = (j > 0) ? u_[i * N_ + (j - 1)] : u_[i * N_ + 1]; 
            double u_bas  = (j < N_ - 1) ? u_[i * N_ + (j + 1)] : T_bord;

            double diff_y = r * (u_haut - 2.0 * u_[i * N_ + j] + u_bas);
            rhs_[i] = u_[i * N_ + j] + diff_y + src;

            // Construction Matrice X (Implicite)
            diag_inf_[i] = -r; diag_[i] = 1.0 + 2.0 * r; diag_sup_[i] = -r;
        }

        // CL X (Ligne j) : Gauche Neumann (i=0), Droite Dirichlet (i=N-1)
        diag_[0] = 1.0 + 2.0 * r; diag_sup_[0] = -2.0 * r; diag_inf_[0] = 0.0;
        
        diag_[N_ - 1] = 1.0; diag_inf_[N_ - 1] = 0.0; diag_sup_[N_ - 1] = 0.0; 
        rhs_[N_ - 1] = T_bord;

        resoudre_thomas(N_, diag_inf_, diag_, diag_sup_, rhs_, result_);
        for (int i = 0; i < N_; ++i) u_demi_[i * N_ + j] = result_[i];
    }
    // Dirichlet Bas (y=L)
    for (int i = 0; i < N_; ++i) u_demi_[i * N_ + (N_-1)] = T_bord;
}

void Solveur2D::etape_2_y_implicite(double r, double source_coeff) {
    double T_bord = 13.0 + 273.15;

    // Balayage des colonnes i
    for (int i = 0; i < N_; ++i) {
        for (int j = 0; j < N_; ++j) {
            double src = source_F(i * dx_, j * dx_) * source_coeff;
            
            // Diffusion Explicite X (gestion Neumann i=0 via ghost point)
            double u_gauche = (i > 0) ? u_demi_[(i - 1) * N_ + j] : u_demi_[1 * N_ + j]; 
            double u_droite = (i < N_ - 1) ? u_demi_[(i + 1) * N_ + j] : T_bord;
            
            double diff_x = r * (u_gauche - 2.0 * u_demi_[i * N_ + j] + u_droite);
            rhs_[j] = u_demi_[i * N_ + j] + diff_x + src;

            // Construction Matrice Y (Implicite)
            diag_inf_[j] = -r; diag_[j] = 1.0 + 2.0 * r; diag_sup_[j] = -r;
        }

        // CL Y (Colonne i) : Haut Neumann (j=0), Bas Dirichlet (j=N-1)
        diag_[0] = 1.0 + 2.0 * r; diag_sup_[0] = -2.0 * r; diag_inf_[0] = 0.0;

        diag_[N_-1] = 1.0; diag_inf_[N_-1] = 0.0; diag_sup_[N_-1] = 0.0; 
        rhs_[N_-1] = T_bord;

        resoudre_thomas(N_, diag_inf_, diag_, diag_sup_, rhs_, result_);
        for (int j = 0; j < N_; ++j) u_next_[i * N_ + j] = result_[j];
    }
}

void Solveur2D::avancer_temps() {
    double alpha = materiau_.get_diffusivite();
    double r = (alpha * dt_ / 2.0) / (dx_ * dx_);
    double source_coeff = (dt_ / 2.0) / (materiau_.get_rho() * materiau_.get_c());

    etape_1_x_implicite(r, source_coeff);
    etape_2_y_implicite(r, source_coeff);

    u_ = u_next_;
    temps_actuel_ += dt_;
    compteur_pas_++;
}