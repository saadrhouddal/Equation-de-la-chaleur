#include "Materiau.h"

// Constructeur basé sur le type de matériau
Materiau::Materiau(TypeMateriau type) {
    switch(type) {
        case TypeMateriau::Cuivre:
            lambda = 389.0;
            rho = 8940.0;
            c = 380.0;
            break;
        case TypeMateriau::Fer:
            lambda = 80.2;
            rho = 7874.0;
            c = 440.0;
            break;
        case TypeMateriau::Verre:
            lambda = 1.2;
            rho = 2530.0;
            c = 840.0;
            break;
        case TypeMateriau::Polystyrene:
            lambda = 0.1;
            rho = 1040.0;
            c = 1200.0;
            break;
    }
}

double Materiau::getLambda() const {
    return lambda;
}

double Materiau::getRho() const {
    return rho;
}

double Materiau::getC() const {
    return c;
}


