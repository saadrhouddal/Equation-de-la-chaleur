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
    long long compteur_pas_; // Ajout pour suivre le nombre exact de pas

public:
    Solveur(Materiau mat, double L, double t_max) 
        : materiau_(mat), longueur_(L), t_max_(t_max), temps_actuel_(0.0), compteur_pas_(0) {}
    
    virtual ~Solveur() {}
    virtual void avancer_temps() = 0;
    virtual const std::vector<double>& get_donnees() const = 0;

    double get_temps_actuel() const { return temps_actuel_; }
    double get_dt() const { return dt_; }
    Materiau get_materiau() const { return materiau_; }
    double get_temps_max() const { return t_max_; }
    long long get_compteur_pas() const { return compteur_pas_; }
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
    
    // Vecteur pour l'étape intermédiaire (ADI)
    std::vector<double> u_demi_; 

    // Vecteurs pré-alloués pour éviter l'allocation dans la boucle (Optimisation)
    std::vector<double> diag_inf_, diag_, diag_sup_, rhs_, result_;

    double source_F(double x, double y);

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

    int get_N() const { return N_; }
};

#endif