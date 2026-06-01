/*
 * qcm.c - Gestion des mots-clés et des QCM (Questions à Choix Multiples).
 */
#include "qcm.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ════════════════════════════════════════════════════════
  MOTS-CLÉS
   ════════════════════════════════════════════════════════ */

/* Ajoute un mot-clé en tête de liste pour une leçon. */
void ajouter_mot_cle(Lecon *l, const char *mot) {
    if (rechercher_mot_cle_lecon(l, mot)) {
        ui_info("Ce mot-clé existe déjà.");
        return;
    }
    MotCle *nv   = malloc(sizeof(MotCle));
    nv->mot      = strdup_securise(mot);
    nv->suivant  = l->mots_cles;
    l->mots_cles = nv;
}

/* Supprime un mot-clé précis dans la liste chaînée. */
void supprimer_mot_cle(Lecon *l, const char *mot) {
    MotCle *prev = NULL, *curr = l->mots_cles;
    while (curr) {
        if (strcmp(curr->mot, mot) == 0) {
            if (prev) prev->suivant = curr->suivant;
            else      l->mots_cles  = curr->suivant;
            free(curr->mot); free(curr);
            ui_succes("Mot-clé supprimé.");
            return;
        }
        prev = curr; curr = curr->suivant;
    }
    ui_erreur("Mot-clé introuvable.");
}

/* Recherche un mot-clé exact dans une leçon. */
int rechercher_mot_cle_lecon(Lecon *l, const char *mot) {
    for (MotCle *m = l->mots_cles; m; m = m->suivant)
        if (strcmp(m->mot, mot) == 0) return 1;
    return 0;
}

/*
 * Recherche par mot-clé : cherche dans les titres de leçons ET dans les mots-clés.
 * (Algorithme de recherche sur chaînes de caractères - cahier des charges §2.2)
 */
void rechercher_mot_cle_cours(Cours *c, const char *mot) {
    int trouve = 0;
    Chapitre *ch = c->premier_chapitre;
    while (ch) {
        Lecon *l = ch->lecons;
        while (l) {
            /* Recherche dans le titre de la leçon */
            int dans_titre = (strstr(l->titre, mot) != NULL);
            /* Recherche dans les mots-clés */
            int dans_mc    = rechercher_mot_cle_lecon(l, mot);
            if (dans_titre || dans_mc) {
                printf("  OK [%s] > %s", ch->titre, l->titre);
                if (dans_titre && !dans_mc)
                    printf(" (titre)\n");
                else if (dans_mc && !dans_titre)
                    printf(" (mot-clé)\n");
                else
                    printf(" (titre + mot-clé)\n");
                trouve = 1;
            }
            l = l->suivant;
        }
        ch = ch->suivant;
    }
    if (!trouve) ui_info("Aucun résultat trouvé.");
}

/* Affiche tous les mots-clés d'une leçon. */
void afficher_mots_cles(Lecon *l) {
    if (!l->mots_cles) { ui_info("Aucun mot-clé."); return; }
    printf("  Mots-clés :");
    for (MotCle *m = l->mots_cles; m; m = m->suivant)
        printf("  [%s]", m->mot);
    printf("\n");
}

/* Libère la liste chaînée des mots-clés. */
void liberer_mots_cles(MotCle *m) {
    while (m) { MotCle *t = m; m = m->suivant; free(t->mot); free(t); }
}

/* ════════════════════════════════════════════════════════
  QUESTIONS / QCM
   ════════════════════════════════════════════════════════ */

/* Crée une question sans proposition au départ. */
Question* creer_question(const char *texte) {
    Question *q    = malloc(sizeof(Question));
    q->texte       = strdup_securise(texte);
    q->propositions = NULL;
    q->suivant     = NULL;
    return q;
}

/* Crée une proposition vraie ou fausse. */
Proposition* creer_proposition(const char *texte, int vrai) {
    Proposition *p = malloc(sizeof(Proposition));
    p->texte    = strdup_securise(texte);
    p->est_vraie = vrai;
    p->suivant  = NULL;
    return p;
}

/* Ajoute une proposition en fin de liste dans une question. */
void ajouter_proposition(Question *q, const char *texte, int vrai) {
    Proposition *nv = creer_proposition(texte, vrai);
    if (!q->propositions) { q->propositions = nv; return; }
    Proposition *tmp = q->propositions;
    while (tmp->suivant) tmp = tmp->suivant;
    tmp->suivant = nv;
}

/* Ajoute une question en fin de liste dans une leçon. */
void ajouter_question(Lecon *l, Question *q) {
    if (!l->questions) { l->questions = q; return; }
    Question *tmp = l->questions;
    while (tmp->suivant) tmp = tmp->suivant;
    tmp->suivant = q;
}

/* Supprime une question à partir de son numéro dans la liste. */
void supprimer_question(Lecon *l, int index) {
    Question *prev = NULL, *curr = l->questions;
    int i = 1;
    while (curr) {
        if (i == index) {
            if (prev) prev->suivant = curr->suivant;
            else      l->questions  = curr->suivant;
            /* Libérer seulement cette question */
            Proposition *p = curr->propositions;
            while (p) { Proposition *t=p; p=p->suivant; free(t->texte); free(t); }
            free(curr->texte); free(curr);
            ui_succes("Question supprimée.");
            return;
        }
        prev = curr; curr = curr->suivant; i++;
    }
    ui_erreur("Numéro de question invalide.");
}

/* Affiche les questions et les propositions enregistrées pour une leçon. */
void afficher_questions(Lecon *l) {
    if (!l->questions) { ui_info("Aucune question QCM."); return; }
    Question *q = l->questions;
    int idx = 1;
    while (q) {
        printf("  %d. %s\n", idx++, q->texte);
        Proposition *p = q->propositions;
        int pi = 1;
        while (p) {
            const char *sym = p->est_vraie
                ? "V"
                : "F";
            printf("       %s %d. %s\n", sym, pi++, p->texte);
            p = p->suivant;
        }
        q = q->suivant;
    }
}

/* Libère toutes les questions et leurs propositions. */
void liberer_questions(Question *q) {
    while (q) {
        Proposition *p = q->propositions;
        while (p) { Proposition *t=p; p=p->suivant; free(t->texte); free(t); }
        Question *t = q; q = q->suivant;
        free(t->texte); free(t);
    }
}

/* Pose une question, retourne 1 si la réponse choisie est correcte. */
int repondre_qcm(Question *q) {
    int nb = 0;
    for (Proposition *p = q->propositions; p; p = p->suivant) nb++;
    if (nb == 0) { ui_erreur("Question sans propositions."); return 0; }

    printf("\n  Question: %s\n", q->texte);
    Proposition *p = q->propositions;
    int idx = 1;
    while (p) {
        printf("    %d. %s\n", idx++, p->texte);
        p = p->suivant;
    }
    printf("  Votre réponse (1-%d) ", nb);
    int choix = lire_entier(1, nb);

    p = q->propositions;
    for (int i = 1; i < choix; i++) p = p->suivant;

    if (p->est_vraie) {
        ui_succes("Bonne réponse !");
        return 1;
    } else {
        ui_erreur("Mauvaise réponse.");
        /* Révéler la bonne réponse */
        p = q->propositions; int bi = 1;
        while (p) {
            if (p->est_vraie) {
                printf("  Réponse correcte : %d. %s\n", bi, p->texte);
                break;
            }
            p = p->suivant; bi++;
        }
        return 0;
    }
}

/* Passe toutes les questions de la leçon et retourne le score obtenu. */
int passer_qcm_lecon(Lecon *l) {
    if (!l->questions) { ui_info("Aucune question pour cette leçon."); return 0; }
    ui_sous_titre("QCM - Répondez aux questions");
    int score = 0, total = 0;
    for (Question *q = l->questions; q; q = q->suivant) {
        total++;
        score += repondre_qcm(q);
    }
    printf("\n");
    ui_ligne(44);
    printf("  Score final : ");
    printf("%d / %d (%.0f%%)\n",
           score, total, total ? (score*100.0/total) : 0.0);
    ui_barre_progression(score, total, 20);
    return score;
}
