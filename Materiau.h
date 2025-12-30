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
     * @brief Énumération des types de matériaux.
     */
    enum class TypeMateriau {
        Cuivre,
        Fer,
        Verre,
        Polystyrene
    };

    /**
     * @brief Constructeur basé sur le type de matériau.
     * @param type Type du matériau.
     */
    Materiau(TypeMateriau type);

    /**
     * @brief Obtient la conductivité thermique du matériau.
     * @return Conductivité thermique en W.m-1·K-1.
     */
    double obtenirLambda() const;

    /**
     * @brief Obtient la masse volumique du matériau.
     * @return Masse volumique en kg.m-3.
     */
    double obtenirMasseVolumique() const;

    /**
     * @brief Obtient la capacité thermique spécifique du matériau.
     * @return Capacité thermique en J.kg-1·K-1.
     */
    double obtenirCapaciteThermique() const;

private:
    double lambda_;                 // Conductivité thermique.
    double masse_volumique_;        // Densité du matériau.
    double capacite_thermique_;     // Capacité thermique spécifique.
};

#endif