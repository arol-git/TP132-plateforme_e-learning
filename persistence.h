#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include "syllabus.h"
#include "progression.h"

/* Sauvegarde uniquement les cours. Gardé pour compatibilité avec le module. */
int    sauvegarder_tous_les_cours(Cours *liste, const char *fichier);

/* Sauvegarde les cours et la progression dans le même fichier JSON. */
int    sauvegarder_base(Cours *liste, Progression *prog, const char *fichier);

/* Charge tous les cours depuis le fichier JSON. */
Cours* charger_tous_les_cours(const char *fichier);

/* Charge la progression de l'étudiant depuis le fichier JSON. */
Progression* charger_progression(const char *fichier);

#endif
