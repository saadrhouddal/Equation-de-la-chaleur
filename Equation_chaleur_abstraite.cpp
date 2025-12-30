#include "Equation_chaleur_abstraite.h"
/**
 * @brief Définition du constructeur.
 * * Initialise le membre protégé 'donnees_materiau_' avec les données du matériau passé en argument.
 */
SolveurChaleurAbstrait::SolveurChaleurAbstrait(const Materiau& donnees_mat) 
    : donnees_materiau_(donnees_mat) // Mise à jour du membre et du paramètre
{
}

// Les autres méthodes virtuelles pures (calculerSource, resoudrePas, obtenirTemperature)
// n'ont pas de définition ici car elles sont implémentées dans les classes dérivées (Modele1D, Modele2D).