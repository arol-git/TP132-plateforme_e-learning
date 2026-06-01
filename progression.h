#ifndef PROGRESSION_H
#define PROGRESSION_H

#include "syllabus.h"

/* Noeud de la liste chaînée des leçons terminées. */
typedef struct ActiviteTerminee {
    char *titre_lecon;
    struct ActiviteTerminee *suivant;
} ActiviteTerminee;

/* Données simples de suivi pour un étudiant. */
typedef struct Progression {
    ActiviteTerminee *liste_terminees;
    int score_total;
    int qcm_passes;
} Progression;

/* Création et libération de la progression. */
Progression* creer_progression(void);
void liberer_progression(Progression *p);

/* Gestion des leçons terminées. */
void marquer_terminee(Progression *p, const char *titre);
void demarquer_terminee(Progression *p, const char *titre);
int  est_terminee(Progression *p, const char *titre);

/* Gestion des scores de QCM et affichage du bilan. */
void ajouter_score(Progression *p, int score, int total);
void afficher_progression(Progression *p, Cours *cours);

#endif
