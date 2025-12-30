#include "materiau.h"

Materiau::Materiau(TypeMateriau type) {
    // Initialisation des propriétés selon le type de matériau
    switch(type) {
        case TypeMateriau::Cuivre:
            lambda_ = 389.0;
            masse_volumique_ = 8940.0;
            capacite_thermique_ = 380.0;
            break;
        case TypeMateriau::Fer:
            lambda_ = 80.2;
            masse_volumique_ = 7874.0;
            capacite_thermique_ = 440.0;
            break;
        case TypeMateriau::Verre:
            lambda_ = 1.2;
            masse_volumique_ = 2530.0;
            capacite_thermique_ = 840.0;
            break;
        case TypeMateriau::Polystyrene:
            lambda_ = 0.1;
            masse_volumique_ = 1040.0;
            capacite_thermique_ = 1200.0;
            break;
    }
}

double Materiau::obtenirLambda() const {
    return lambda_;
}

double Materiau::obtenirMasseVolumique() const {
    return masse_volumique_;
}

double Materiau::obtenirCapaciteThermique() const {
    return capacite_thermique_;
}
