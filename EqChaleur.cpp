#include "EqChaleur.h"

EquationDeLaChaleur::EquationDeLaChaleur(const Materiau& m) : mat_(m) {
    
}

// Comme la classe est abstraite, nous n'avons pas besoin d'implémenter les méthodes virtuelles pures ici.
// Elles seront implémentées dans les classes dérivées.
