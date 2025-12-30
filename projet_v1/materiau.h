#ifndef MATERIAU_H
#define MATERIAU_H

#include <string>

class Materiau {
private:
    std::string nom_;
    double lambda_; // Conductivité (W/m.K)
    double rho_;    // Masse volumique (kg/m3)
    double c_;      // Chaleur massique (J/kg.K)

public:
    Materiau(std::string nom, double lambda, double rho, double c)
        : nom_(nom), lambda_(lambda), rho_(rho), c_(c) {}

    double get_lambda() const { return lambda_; }
    double get_rho() const { return rho_; }
    double get_c() const { return c_; }
    std::string get_nom() const { return nom_; }
    
    // La propriété physique clé : Diffusivité thermique = lambda / (rho * c)
    double get_diffusivite() const { return lambda_ / (rho_ * c_); }
};

#endif