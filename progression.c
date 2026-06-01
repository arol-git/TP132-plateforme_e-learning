/*
 * progression.c - Suivi de la progression de l'étudiant.
 */
#include "progression.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Initialise une progression vide. */
Progression* creer_progression(void) {
    Progression *p     = malloc(sizeof(Progression));
    p->liste_terminees = NULL;
    p->score_total     = 0;
    p->qcm_passes      = 0;
    return p;
}

/* Ajoute une leçon à la liste des leçons terminées si elle n'y est pas déjà. */
void marquer_terminee(Progression *p, const char *titre) {
    if (est_terminee(p, titre)) {
        ui_info("Cette leçon est déjà marquée comme terminée.");
        return;
    }
    ActiviteTerminee *a = malloc(sizeof(ActiviteTerminee));
    a->titre_lecon = strdup_securise(titre);
    a->suivant     = p->liste_terminees;
    p->liste_terminees = a;
}

/* Retire une leçon de la liste des leçons terminées. */
void demarquer_terminee(Progression *p, const char *titre) {
    ActiviteTerminee *prev = NULL, *a = p->liste_terminees;
    while (a) {
        if (strcmp(a->titre_lecon, titre) == 0) {
            if (prev) prev->suivant      = a->suivant;
            else      p->liste_terminees = a->suivant;
            free(a->titre_lecon); free(a);
            ui_succes("Leçon démarquée.");
            return;
        }
        prev = a; a = a->suivant;
    }
}

/* Vérifie si une leçon est déjà marquée comme terminée. */
int est_terminee(Progression *p, const char *titre) {
    for (ActiviteTerminee *a = p->liste_terminees; a; a = a->suivant)
        if (strcmp(a->titre_lecon, titre) == 0) return 1;
    return 0;
}

/* Ajoute le score d'un QCM au total de l'étudiant. */
void ajouter_score(Progression *p, int score, int total) {
    if (total > 0) { p->score_total += score; p->qcm_passes++; }
}

/* Affiche la progression chapitre par chapitre puis le total global. */
void afficher_progression(Progression *p, Cours *cours) {
    if (!cours) { ui_erreur("Aucun cours sélectionné."); return; }
    ui_sous_titre("Ma Progression");

    int grand_total = 0, grand_term = 0;

    for (Chapitre *ch = cours->premier_chapitre; ch; ch = ch->suivant) {
        int total = 0, term = 0;
        for (Lecon *l = ch->lecons; l; l = l->suivant) {
            total++; grand_total++;
            if (est_terminee(p, l->titre)) { term++; grand_term++; }
        }
        printf("\n  - %s  %d/%d\n", ch->titre, term, total);
        for (Lecon *l = ch->lecons; l; l = l->suivant) {
            const char *sym = est_terminee(p, l->titre)
                ? "[x]"
                : "[ ]";
            printf("  %s %s\n", sym, l->titre);
        }
        if (!ch->lecons)
            printf("    (aucune leçon)\n");
    }

    if (grand_total == 0) { ui_info("Le cours ne contient aucune leçon."); return; }

    printf("\n  Progression globale :\n");
    ui_barre_progression(grand_term, grand_total, 30);

    if (p->qcm_passes > 0)
        printf("  QCM : %d passé(s), score total %d pts\n",
               p->qcm_passes, p->score_total);
}

/* Libère toute la liste chaînée des activités terminées. */
void liberer_progression(Progression *p) {
    ActiviteTerminee *a = p->liste_terminees;
    while (a) { ActiviteTerminee *t=a; a=a->suivant; free(t->titre_lecon); free(t); }
    free(p);
}
