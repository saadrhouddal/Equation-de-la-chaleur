#include "Modele2D.h"
#include <iostream>
#include <iomanip>
#include <vector>

Modele2D::Modele2D(const Materiau& mat, double f, double L, double tmax, int n_points_x, int n_points_y, double u0)
    : EquationDeLaChaleur(mat), f_(f), L_(L), tmax_(tmax), n_points_x_(n_points_x), n_points_y_(n_points_y) {
    u_.resize(n_points_x_, std::vector<double>(n_points_y_, u0 + 273.15));
}

double Modele2D::Source(double x, double y) const {
    double tmax_f2 = tmax_ * f_ * f_;
    if (x >= L_ / 10.0 && x <= 2 * L_ / 10.0 && y >= L_ / 10.0 && y <= 2 * L_ / 10.0) {
        return tmax_f2;
    } else {
        return 0.0;
    }
}

void Modele2D::decomposeLU(std::vector<std::vector<double>>& A, std::vector<std::vector<double>>& L, std::vector<std::vector<double>>& U) {
    int n = A.size();
    L.resize(n, std::vector<double>(n, 0.0));
    U.resize(n, std::vector<double>(n, 0.0));

    for (int i = 0; i < n; ++i) {
        for (int j = i; j < n; ++j) {
            U[i][j] = A[i][j];
            for (int k = 0; k < i; ++k) {
                U[i][j] -= L[i][k] * U[k][j];
            }
        }
        for (int j = i; j < n; ++j) {
            if (i == j) {
                L[i][i] = 1.0;
            } else {
                L[j][i] = A[j][i];
                for (int k = 0; k < i; ++k) {
                    L[j][i] -= L[j][k] * U[k][i];
                }
                L[j][i] /= U[i][i];
            }
        }
    }
}

void Modele2D::resoudreLU(const std::vector<std::vector<double>>& L, const std::vector<std::vector<double>>& U, const std::vector<double>& b, std::vector<double>& x) {
    int n = L.size();
    std::vector<double> y(n, 0.0);

    for (int i = 0; i < n; ++i) {
        y[i] = b[i];
        for (int j = 0; j < i; ++j) {
            y[i] -= L[i][j] * y[j];
        }
    }

    for (int i = n - 1; i >= 0; --i) {
        x[i] = y[i];
        for (int j = i + 1; j < n; ++j) {
            x[i] -= U[i][j] * x[j];
        }
        x[i] /= U[i][i];
    }
}

void Modele2D::resoudre(double dt) {
    double dx = L_ / (n_points_x_ - 1);
    double dy = L_ / (n_points_y_ - 1);
    double alpha = mat_.getLambda() / (mat_.getRho() * mat_.getC());

    double r_x = alpha * dt / (dx * dx);
    double r_y = alpha * dt / (dy * dy);

    std::vector<std::vector<double>> L, U;
    std::vector<std::vector<double>> A(n_points_x_, std::vector<double>(n_points_x_, 0.0));

    for (int i = 0; i < n_points_x_; ++i) {
        A[i][i] = 1 + 2 * r_x + 2 * r_y;
        if (i > 0) A[i][i - 1] = -r_x;
        if (i < n_points_x_ - 1) A[i][i + 1] = -r_x;
    }

    decomposeLU(A, L, U);

    for (double t = 0; t <= tmax_; t += dt) {
        std::vector<double> b(n_points_x_, 0.0);
        std::vector<double> x(n_points_x_, 0.0);

        for (int i = 1; i < n_points_y_ - 1; ++i) {
            for (int j = 1; j < n_points_x_ - 1; ++j) {
                b[j] = u_[j][i] + r_y * (u_[j][i - 1] + u_[j][i + 1]);
            }
            resoudreLU(L, U, b, x);

            for (int j = 0; j < n_points_x_; ++j) {
                u_[j][i] = x[j];
            }
        }
    }
}

double Modele2D::getTemperatureAtPoint(double x, double y) const {
    int idx_x = static_cast<int>(x / L_ * (n_points_x_ - 1));
    int idx_y = static_cast<int>(y / L_ * (n_points_y_ - 1));
    return u_[idx_x][idx_y];
}
