#ifndef MODELE2D_H
#define MODELE2D_H

#include <vector>
#include "EqChaleur.h"

/**
 * @class Modele2D
 * @brief Classe pour simuler la diffusion de la chaleur dans un système bidimensionnel.
 */
class Modele2D : public EquationDeLaChaleur {
public:
    /**
     * @brief Constructeur de la classe Modele2D.
     * @param mat Matériau utilisé pour la simulation.
     * @param f Intensité de la source de chaleur.
     * @param L Longueur du domaine en mètres (assumé carré).
     * @param tmax Temps maximum de simulation en secondes.
     * @param n_points_x Nombre de points en direction x.
     * @param n_points_y Nombre de points en direction y.
     * @param u0 Température initiale en degrés Celsius.
     */
    Modele2D(const Materiau& mat, double f, double L, double tmax, int n_points_x, int n_points_y, double u0);

    /**
     * @brief Définit l'intensité de la source de chaleur en fonction des coordonnées.
     * @param x Position en x dans le domaine.
     * @param y Position en y dans le domaine.
     * @return Intensité de la source de chaleur à la position donnée.
     */
    double Source(double x, double y) const override;

    /**
     * @brief Résout l'équation de la chaleur pour un pas de temps donné.
     * @param dt Pas de temps utilisé dans la résolution numérique.
     */
    void resoudre(double dt) override;

    /**
     * @brief Récupère la température à un point donné dans le domaine.
     * @param x Position en x dans le domaine.
     * @param y Position en y dans le domaine.
     * @return Température au point (x, y) en degrés Celsius.
     */
    double getTemperatureAtPoint(double x, double y) const override;

private:
    /**
     * @brief Décompose une matrice en facteurs L et U (décomposition LU).
     * @param A Matrice à décomposer.
     * @param L Matrice triangulaire inférieure résultante.
     * @param U Matrice triangulaire supérieure résultante.
     */
    void decomposeLU(std::vector<std::vector<double>>& A, std::vector<std::vector<double>>& L, std::vector<std::vector<double>>& U);

    /**
     * @brief Résout un système linéaire en utilisant les matrices L et U obtenues par décomposition LU.
     * @param L Matrice triangulaire inférieure.
     * @param U Matrice triangulaire supérieure.
     * @param b Vecteur des termes constants.
     * @param x Vecteur solution.
     */
    void resoudreLU(const std::vector<std::vector<double>>& L, const std::vector<std::vector<double>>& U, const std::vector<double>& b, std::vector<double>& x);

    double f_; ///< Intensité de la source de chaleur.
    double L_; ///< Longueur du domaine en mètres.
    double tmax_; ///< Temps maximum de simulation en secondes.
    int n_points_x_; ///< Nombre de points de discrétisation en direction x.
    int n_points_y_; ///< Nombre de points de discrétisation en direction y.
    std::vector<std::vector<double>> u_; ///< Matrice contenant les températures du domaine.
};

#endif // MODELE2D_H
