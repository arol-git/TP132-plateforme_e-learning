/*
 * contenu.c - Affichage et modification du contenu des leçons.
 */
#include "contenu.h"
#include "qcm.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Remplace le contenu d'une leçon par un nouveau texte. */
void modifier_contenu_lecon(Lecon *l, const char *nv) {
    free(l->contenu);
    l->contenu = strdup_securise(nv);
    ui_succes("Contenu mis à jour.");
}

/* Affiche le détail d'une leçon pour l'espace étudiant. */
void afficher_contenu_lecon(Lecon *l) {
    /* En-tête */
    printf("\n");
    ui_ligne(54);
    printf("  Leçon #%d - %s\n", l->id, l->titre);
    ui_ligne(54);

    /* Contenu ligne par ligne */
    const char *c = l->contenu ? l->contenu : "(aucun contenu)";
    printf("\n");
    while (*c) {
        const char *nl = strchr(c, '\n');
        int len = nl ? (int)(nl - c) : (int)strlen(c);
        printf("  %.*s\n", len, c);
        c += len + (nl ? 1 : 0);
        if (!nl) break;
    }
    printf("\n");

    /* Mots-clés */
    if (l->mots_cles) {
        printf("  Mots-clés :");
        for (MotCle *m = l->mots_cles; m; m = m->suivant)
            printf("  [%s]", m->mot);
        printf("\n");
    }

    /* Indicateur QCM */
    if (l->questions) {
        int nq = 0;
        for (Question *q = l->questions; q; q = q->suivant) nq++;
        printf("  %d question(s) QCM disponible(s)\n", nq);
    }
    ui_ligne(54);
}
