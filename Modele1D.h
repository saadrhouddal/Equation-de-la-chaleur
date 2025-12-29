#ifndef MODELE1D_H
#define MODELE1D_H

#include "EqChaleur.h"
#include <vector>

/**
 * @class Modele1D
 * @brief Classe pour simuler la diffusion de la chaleur dans un système 1D.
 */
class Modele1D : public EquationDeLaChaleur {
public:
    /**
     * @brief Constructeur de la classe Modele1D.
     * @param mat Matériau utilisé.
     * @param f Intensité de la source de chaleur.
     * @param L Longueur de la barre.
     * @param tmax Temps maximum de simulation.
     * @param n_points Nombre de points de discrétisation.
     * @param u0 Température initiale.
     */
    Modele1D(const Materiau& mat, double f, double L, double tmax, int n_points, double u0);

    /**
     * @brief Fonction pour définir la source de chaleur.
     * @param x Position sur la barre.
     * @param y Non utilisé pour le modèle 1D.
     * @return Intensité de la source à la position donnée.
     */
    double Source(double x, double y = 0) const override;

    /**
     * @brief Résolution du système pour un pas de temps donné.
     * @param dt Pas de temps.
     */
    void resoudre(double dt) override;

    /**
     * @brief Récupère la température à un point donné.
     * @param x Position sur la barre.
     * @param y Non utilisé pour le modèle 1D.
     * @return Température au point donné.
     */
    double getTemperatureAtPoint(double x, double y = 0) const override;

private:
    double f_;         ///< Intensité de la source de chaleur.
    double L_;         ///< Longueur de la barre.
    double tmax_;      ///< Temps maximum de simulation.
    int n_points_;     ///< Nombre de points de discrétisation.
    std::vector<double> u_; ///< Vecteur contenant les températures.
    /**
     * @brief Implémente l'algorithme de Thomas pour résoudre un système tridiagonal.
     * @param a Vecteur des coefficients sous-diagonaux.
     * @param b Vecteur des coefficients diagonaux.
     * @param c Vecteur des coefficients sur-diagonaux.
     * @param d Vecteur des termes constants.
     * @param x Vecteur solution.
     * @param n Taille du système.
     */
    void thomasAlgorithm(const std::vector<double>& a, const std::vector<double>& b, 
                         const std::vector<double>& c, std::vector<double>& d, 
                         std::vector<double>& x, int n);

};

#endif // MODELE1D_H
