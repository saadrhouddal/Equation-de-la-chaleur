#ifndef SOLVEUR_H
#define SOLVEUR_H

#include <vector>
#include "materiau.h"

/**
 * @brief Classe abstraite gérant la résolution de l'équation de la chaleur.
 */
class Solveur {
protected:
    Materiau materiau_;
    double longueur_;
    double t_max_;
    double dt_;
    double temps_actuel_;
    long long compteur_pas_; 

public:
    /**
     * @brief Constructeur du solveur générique.
     * @param mat Matériau étudié.
     * @param L Longueur du domaine (m).
     * @param t_max Temps maximum de simulation (s).
     */
    Solveur(Materiau mat, double L, double t_max) 
        : materiau_(mat), longueur_(L), t_max_(t_max), temps_actuel_(0.0), compteur_pas_(0) {}
    
    virtual ~Solveur() {}

    /**
     * @brief Avance la simulation d'un pas de temps dt.
     */
    virtual void avancer_temps() = 0;

    /**
     * @brief Récupère les données actuelles de température.
     * @return Référence constante vers le vecteur de températures.
     */
    virtual const std::vector<double>& get_donnees() const = 0;

    double get_temps_actuel() const { return temps_actuel_; }
    double get_dt() const { return dt_; }
    Materiau get_materiau() const { return materiau_; }
    double get_temps_max() const { return t_max_; }
    long long get_compteur_pas() const { return compteur_pas_; }
};

//Solveur 1D
/**
 * @brief Implémentation 1D du solveur
 */
class Solveur1D : public Solveur {
private:
    int nb_points_;
    double dx_;
    std::vector<double> u_;
    std::vector<double> u_next_;
    std::vector<double> cp_, dp_;
    
    /**
     * @brief Calcule F(x).
     */
    double source_F(double x);

public:
    Solveur1D(Materiau mat, int nb_points, double L, double t_max);
    void avancer_temps() override;
    const std::vector<double>& get_donnees() const override { return u_; }
};

//Solveur 2D
/**
 * @brief Implémentation 2D du solveur
 */
class Solveur2D : public Solveur {
private:
    int N_;
    double dx_;
    std::vector<double> u_;
    std::vector<double> u_next_;
    std::vector<double> u_demi_;
    std::vector<double> c_prime_, d_prime_;

    // Vecteurs pré-alloués
    std::vector<double> diag_inf_, diag_, diag_sup_, rhs_, result_;

    double source_F(double x, double y);

    /**
     * @brief Résolution d'un système tridiagonal (Algo de Thomas)
     */
    void resoudre_thomas(int n, 
                         const std::vector<double>& a_inf, 
                         const std::vector<double>& b_diag, 
                         const std::vector<double>& c_sup, 
                         const std::vector<double>& d_rhs, 
                         std::vector<double>& x_sol);
    
    /**
     * @brief Étape 1 ADI : Implicite en X, Explicite en Y.
     * @param r Coefficient de diffusion discretisé.
     * @param source_coeff Coefficient multiplicateur de la source.
     */
    void etape_1_x_implicite(double r, double source_coeff);

    /**
     * @brief Étape 2 ADI : Explicite en X, Implicite en Y.
     * @param r Coefficient de diffusion discretisé.
     * @param source_coeff Coefficient multiplicateur de la source.
     */
    void etape_2_y_implicite(double r, double source_coeff);

public:
    Solveur2D(Materiau mat, int nb_points, double L, double t_max);
    void avancer_temps() override;
    const std::vector<double>& get_donnees() const override { return u_; }
    int get_N() const { return N_; }
};

#endif