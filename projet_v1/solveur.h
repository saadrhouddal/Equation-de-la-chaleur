#ifndef SOLVEUR_H
#define SOLVEUR_H

#include <vector>
#include "materiau.h"

class Solveur {
protected:
    Materiau materiau_;
    double longueur_;
    double t_max_;
    double dt_;
    double temps_actuel_;

public:
    Solveur(Materiau mat, double L, double t_max) 
        : materiau_(mat), longueur_(L), t_max_(t_max), temps_actuel_(0.0) {}
    
    virtual ~Solveur() {}
    virtual void avancer_temps() = 0;
    virtual const std::vector<double>& get_donnees() const = 0;

    double get_temps_actuel() const { return temps_actuel_; }
    double get_dt() const { return dt_; }
    Materiau get_materiau() const { return materiau_; }
    double get_temps_max() const { return t_max_; }
};

// --- Solveur 1D ---
class Solveur1D : public Solveur {
private:
    int nb_points_;
    double dx_;
    std::vector<double> u_;
    std::vector<double> u_next_; 
    double source_F(double x);

public:
    Solveur1D(Materiau mat, int nb_points, double L, double t_max);
    void avancer_temps() override;
    const std::vector<double>& get_donnees() const override { return u_; }
};

// --- Solveur 2D ---
class Solveur2D : public Solveur {
private:
    int N_;
    double dx_;
    std::vector<double> u_;
    std::vector<double> u_next_;
    
    // Nouveau vecteur pour l'étape intermédiaire (t + dt/2)
    std::vector<double> u_demi_; 

    double source_F(double x, double y);

    // Méthode utilitaire pour résoudre un système tridiagonal (Algo de Thomas)
    void resoudre_thomas(int n, 
                         const std::vector<double>& a_inf, 
                         const std::vector<double>& b_diag, 
                         const std::vector<double>& c_sup, 
                         const std::vector<double>& d_rhs, 
                         std::vector<double>& x_sol);

public:
    Solveur2D(Materiau mat, int nb_points, double L, double t_max);
    void avancer_temps() override;
    const std::vector<double>& get_donnees() const override { return u_; }

    // --- C'est la méthode qu'il manquait pour le main.cpp ---
    int get_N() const { return N_; }
};

#endif