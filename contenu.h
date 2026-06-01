#ifndef CONTENU_H
#define CONTENU_H

#include "syllabus.h"

/* Modifie le texte associé à une leçon. */
void modifier_contenu_lecon(Lecon *l, const char *nv);

/* Affiche une leçon avec son contenu, ses mots-clés et son QCM. */
void afficher_contenu_lecon(Lecon *l);

#endif
