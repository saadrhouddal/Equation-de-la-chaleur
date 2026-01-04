// On a mis ici le commentaire de la page principale Doxygen :

/**
 * \mainpage Documentation du Projet Simulation Chaleur
 *
 * \section intro_sec Introduction
 * Bienvenue dans la documentation technique du solveur thermique.
 *
 * \section arch_sec Architecture
 * Voici l'architecture globale du projet :
 * \image html uml.png "Diagramme UML des classes" width=600px
 */


#ifndef MATERIAU_H
#define MATERIAU_H

#include <string>

/**
 * @brief Représente les propriétés thermiques d'un matériau.
 * * Stocke les constantes physiques nécessaires à l'équation de la chaleur.
 */
class Materiau {
private:
    std::string nom_;
    double lambda_; // Conductivité thermique
    double rho_;    // Masse volumique
    double c_;      // Chaleur massique

public:
    /**
     * @brief Constructeur d'un matériau.
     * @param nom Nom du matériau .
     * @param lambda Conductivité thermique
     * @param rho Masse volumique
     * @param c Chaleur massique
     */
    Materiau(std::string nom, double lambda, double rho, double c)
        : nom_(nom), lambda_(lambda), rho_(rho), c_(c) {}

    double get_lambda() const { return lambda_; }
    double get_rho() const { return rho_; }
    double get_c() const { return c_; }
    std::string get_nom() const { return nom_; }
    
    /**
     * @brief Calcule la diffusivité thermique.
     * @return Diffusivité alpha = lambda / (rho * c).
     */
    double get_diffusivite() const { return lambda_ / (rho_ * c_); }
};

#endif