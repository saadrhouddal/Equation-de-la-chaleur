#ifndef MATERIAU_H
#define MATERIAU_H

/**
 * @class Materiau
 * @brief Classe représentant les propriétés physiques d'un matériau.
 *
 * Cette classe contient les propriétés nécessaires pour résoudre l'équation
 * de la chaleur, telles que la conductivité thermique, la densité et la capacité thermique.
 */
class Materiau {
public:
    /**
     * @enum TypeMateriau
     * @brief Énumération des types de matériaux disponibles.
     */
    enum class TypeMateriau {
        Cuivre,      ///< Matériau : Cuivre.
        Fer,         ///< Matériau : Fer.
        Verre,       ///< Matériau : Verre.
        Polystyrene  ///< Matériau : Polystyrène.
    };

    /**
     * @brief Constructeur basé sur le type de matériau.
     * @param type Type du matériau (par exemple : Cuivre, Fer).
     */
    Materiau(TypeMateriau type);

    /**
     * @brief Obtient la conductivité thermique du matériau.
     * @return Conductivité thermique en W/(m·K).
     */
    double getLambda() const;

    /**
     * @brief Obtient la densité du matériau.
     * @return Densité en kg/m³.
     */
    double getRho() const;

    /**
     * @brief Obtient la capacité thermique du matériau.
     * @return Capacité thermique en J/(kg·K).
     */
    double getC() const;

private:
    double lambda; ///< Conductivité thermique.
    double rho;    ///< Densité du matériau.
    double c;      ///< Capacité thermique spécifique.
};

#endif // MATERIAU_H
