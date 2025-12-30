#include "solveur.h"
#include <cmath>
#include <iostream>
#include <algorithm>

// --- IMPLÉMENTATION 1D (Implicite) ---
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
    std::vector<double> a(n, -r), b(n, 1.0 + 2.0*r), c(n, -r), rhs(n);

    for(int i=0; i<n; ++i) {
        double src = (source_F(i*dx_) / (materiau_.get_rho() * materiau_.get_c())) * dt_;
        rhs[i] = u_[i] + src;
    }
    // CL Neumann (gauche) et Dirichlet (droite)
    c[0] = -2.0 * r; 
    a[n-1] = 0.0; b[n-1] = 1.0; c[n-1] = 0.0; rhs[n-1] = 13.0 + 273.15;

    // Thomas Algorithm
    std::vector<double> cp(n), dp(n);
    cp[0] = c[0] / b[0]; dp[0] = rhs[0] / b[0];
    for(int i=1; i<n; ++i) {
        double temp = b[i] - a[i] * cp[i-1];
        if (i < n-1) cp[i] = c[i] / temp;
        dp[i] = (rhs[i] - a[i] * dp[i-1]) / temp;
    }
    u_next_[n-1] = dp[n-1];
    for(int i=n-2; i>=0; --i) u_next_[i] = dp[i] - cp[i] * u_next_[i+1];
    
    u_ = u_next_;
    temps_actuel_ += dt_;
}

// --- IMPLÉMENTATION 2D (Explicite) ---
Solveur2D::Solveur2D(Materiau mat, int points_par_cote, double L, double t_max)
    : Solveur(mat, L, t_max), N_(points_par_cote) {
    dx_ = longueur_ / (N_ - 1);
    
    // Calcul automatique de la stabilité CFL selon le matériau
    double alpha = materiau_.get_diffusivite();
    // Plus alpha est grand (conducteur), plus dt doit être petit
    double dt_stable = (dx_ * dx_) / (4.2 * alpha); 
    dt_ = std::min(t_max_ / 2000.0, dt_stable);

    u_.assign(N_ * N_, 13.0 + 273.15);
    u_next_ = u_;
}

double Solveur2D::source_F(double x, double y) {
    // Zones de chauffe
    bool x_z1 = (x >= 1.0/6.0 && x <= 2.0/6.0);
    bool x_z2 = (x >= 4.0/6.0 && x <= 5.0/6.0);
    bool y_z1 = (y >= 1.0/6.0 && y <= 2.0/6.0);
    bool y_z2 = (y >= 4.0/6.0 && y <= 5.0/6.0);
    double val = t_max_ * 6400.0; 

    if ((x_z1 && y_z1) || (x_z2 && y_z1) || (x_z1 && y_z2) || (x_z2 && y_z2)) return val;
    return 0.0;
}

void Solveur2D::avancer_temps() {
    double alpha = materiau_.get_diffusivite();
    double coeff = (alpha * dt_) / (dx_ * dx_);
    double const_src = dt_ / (materiau_.get_rho() * materiau_.get_c());

    for (int i = 1; i < N_ - 1; ++i) {
        for (int j = 1; j < N_ - 1; ++j) {
            int k = idx(i, j);
            double laplacien = u_[idx(i+1,j)] + u_[idx(i-1,j)] + u_[idx(i,j+1)] + u_[idx(i,j-1)] - 4.0*u_[k];
            u_next_[k] = u_[k] + coeff * laplacien + source_F(j*dx_, i*dx_) * const_src;
        }
    }
    // CL Dirichlet (Haut/Droite)
    double T_b = 13.0 + 273.15;
    for (int j = 0; j < N_; ++j) u_next_[idx(N_-1, j)] = T_b;
    for (int i = 0; i < N_; ++i) u_next_[idx(i, N_-1)] = T_b;
    // CL Neumann (Bas/Gauche)
    for (int j = 0; j < N_; ++j) u_next_[idx(0, j)] = u_next_[idx(1, j)];
    for (int i = 0; i < N_; ++i) u_next_[idx(i, 0)] = u_next_[idx(i, 1)];

    u_ = u_next_;
    temps_actuel_ += dt_;
}