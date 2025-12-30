#ifndef EQUATION_CHALEUR_ABSTRAITE_H
#define EQUATION_CHALEUR_ABSTRAITE_H

#include "materiau.h"

/**
 * @class SolveurChaleurAbstrait
 * @brief Classe de base (abstraite) pour les modèles de simulation thermique (1D/2D).
 * @details Cette classe définit l'interface obligatoire que tous les solveurs doivent implémenter.
 */
class SolveurChaleurAbstrait {
public:
    /**
     * @brief Constructeur de la classe SolveurChaleurAbstrait.
     * @param donnees_mat Les propriétés physiques du matériau utilisé.
     */
    SolveurChaleurAbstrait(const Materiau& donnees_mat);

    /**
     * @brief Destructeur virtuel par défaut.
     */
    virtual ~SolveurChaleurAbstrait() = default;

    /**
     * @brief Calcule la valeur de la source de chaleur à une position donnée.
     * @param x Coordonnée spatiale sur l'axe x.
     * @param y Coordonnée spatiale sur l'axe y (par défaut à 0.0 pour le 1D).
     * @return Intensité de la source de chaleur.
     */
    virtual double calculerSource(double x, double y = 0.0) const = 0;

    /**
     * @brief Avance la simulation d'un pas de temps.
     * @param dt Le pas de temps pour la résolution.
     * @throw ErreurNumerique En cas de problème de stabilité ou de singularité.
     */
    virtual void resoudrePas(double dt) = 0;

    /**
     * @brief Obtient la température calculée à un point du domaine.
     * @param x Coordonnée spatiale sur l'axe x.
     * @param y Coordonnée spatiale sur l'axe y (par défaut à 0.0 pour le 1D).
     * @return Température au point spécifié (en Kelvin).
     */
    virtual double obtenirTemperature(double x, double y = 0.0) const = 0;

protected:
    Materiau donnees_materiau_; ///< Données physiques du matériau.
};



#endif