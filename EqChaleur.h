#ifndef EQCHALEUR_H
#define EQCHALEUR_H

#include "Materiau.h"

/**
 * @class EquationDeLaChaleur
 * @brief Classe abstraite pour modéliser l'équation de la chaleur.
 *
 * Cette classe sert de base pour implémenter des modèles 1D et 2D
 * pour résoudre numériquement l'équation de la chaleur avec des sources de chaleur.
 */
class EquationDeLaChaleur {
public:
    /**
     * @brief Constructeur de la classe EquationDeLaChaleur.
     * @param mat Matériau utilisé pour la simulation.
     */
    EquationDeLaChaleur(const Materiau& mat);

    /**
     * @brief Destructeur virtuel par défaut.
     */
    virtual ~EquationDeLaChaleur() = default;

    /**
     * @brief Calcule la source de chaleur à un point donné.
     * @param x Position sur l'axe x.
     * @param y Position sur l'axe y (par défaut à 0.0 pour le 1D).
     * @return Valeur de la source de chaleur.
     */
    virtual double Source(double x, double y = 0.0) const = 0;

    /**
     * @brief Résout l'équation de la chaleur pour un instant donné.
     * @param t Pas de temps pour la résolution.
     */
    virtual void resoudre(double t) = 0;

    /**
     * @brief Obtient la température à un point donné.
     * @param x Position sur l'axe x.
     * @param y Position sur l'axe y (par défaut à 0.0 pour le 1D).
     * @return Température à la position spécifiée.
     */
    virtual double getTemperatureAtPoint(double x, double y = 0.0) const = 0;

protected:
    Materiau mat_; ///< Matériau utilisé pour la simulation.
};

#endif // EQCHALEUR_H
