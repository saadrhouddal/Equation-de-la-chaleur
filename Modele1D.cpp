#include "Modele1D.h"
#include <vector>
#include <cmath>
#include <iostream>

Modele1D::Modele1D(const Materiau& mat, double f, double L, double tmax, int n_points, double u0)
    : EquationDeLaChaleur(mat), f_(f), L_(L), tmax_(tmax), n_points_(n_points) {
    u_.resize(n_points_, u0 + 273.15); // Convertir u0 en Kelvin et initialiser
}

double Modele1D::Source(double x,double /*y*/) const {
    double tmax_f2 = tmax_ * f_ * f_;
    if (x >= L_ / 10.0 && x <= 2 * L_ / 10.0) {
        return tmax_f2;
    } else if (x >= 5 * L_ / 10.0 && x <= 6 * L_ / 10.0) {
        return (3.0 / 4.0) * tmax_f2;
    } else {
        return 0.0;
    }
}



void Modele1D::thomasAlgorithm(const std::vector<double>& a, const std::vector<double>& b, 
                     const std::vector<double>& c, std::vector<double>& d, 
                     std::vector<double>& x, int n) {
    std::vector<double> c_star(n, 0.0);
    std::vector<double> d_star(n, 0.0);

    // Modification des coefficients de la première ligne
    c_star[0] = c[0] / b[0];
    d_star[0] = d[0] / b[0];

    // Élimination vers l'avant
    for (int i = 1; i < n; i++) {
        double m = 1.0 / (b[i] - a[i] * c_star[i - 1]);
        c_star[i] = c[i] * m;
        d_star[i] = (d[i] - a[i] * d_star[i - 1]) * m;
    }

    // Substitution arrière
    x[n - 1] = d_star[n - 1];
    for (int i = n - 2; i >= 0; i--) {
        x[i] = d_star[i] - c_star[i] * x[i + 1];
    }
}



void Modele1D::resoudre(double dt) {
    double dx = L_ / (n_points_ - 1);
    double alpha = mat_.getLambda() / (mat_.getRho() * mat_.getC());

    std::vector<double> a(n_points_, -alpha * dt / (dx * dx));
    std::vector<double> b(n_points_, 1 + 2 * alpha * dt / (dx * dx));
    std::vector<double> c(n_points_, -alpha * dt / (dx * dx));
    std::vector<double> d(n_points_);

    // Condition de Neumann en x = 0
    a[0] = 0.; // Non utilisé
    b[0] = 1.; // Assure que u_0 = u_1
    c[0] = -1.; // Assure que u_0 = u_1
    d[0] = 0.; // Assure que u_0 = u_1

    // Condition de Dirichlet en x = 1m
    c[n_points_ - 1] = 0.; // Non utilisé
    a[n_points_ - 1] = 0.; //Assure que u_n=13°C
    b[n_points_ - 1] = 1.;//Assure que u_n=13°C
    d[n_points_ - 1] = 13.+273.15; // Valeur fixée pour la condition de Dirichlet

    // Remplissage de d pour les autres points
    for (int i = 1; i < n_points_ - 1; ++i) {
        d[i] = u_[i] + (dt * Source(dx * i)) / (mat_.getRho() * mat_.getC());
    }
    
    std::cout << std::endl;
    // Résolution du système tridiagonal
    thomasAlgorithm(a, b, c, d, u_, n_points_);
}
    

double Modele1D::getTemperatureAtPoint(double x, double /*y*/) const {
    int idx = static_cast<int>(x / L_ * (n_points_ - 1));
    return u_[idx];
}
