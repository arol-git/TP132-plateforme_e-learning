/*
 * syllabus.c - Gestion des structures de données (Cours, Chapitres, Leçons)
 * Utilise des listes chaînées dynamiques + récursivité pour l'affichage.
 */
#include "syllabus.h"
#include "qcm.h"      /* pour liberer_mots_cles, liberer_questions */
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Compteur global pour les IDs de leçons. */
static int g_id_lecon = 0;

/* Retourne un nouvel identifiant de leçon. */
int prochain_id_lecon(void) { return ++g_id_lecon; }

/* Remet le compteur au bon niveau après un chargement depuis le JSON. */
void synchroniser_id_lecon(int id) {
    if (id > g_id_lecon) g_id_lecon = id;
}

/* ════════════════════════════════════════════════════════
   GESTION DES COURS
   ════════════════════════════════════════════════════════ */

Cours* creer_cours(const char *titre) {
    Cours *c = malloc(sizeof(Cours));
    if (!c) return NULL;
    c->titre             = strdup_securise(titre);
    c->premier_chapitre  = NULL;
    c->suivant           = NULL;
    return c;
}

/* Ajoute un cours en fin de liste si son titre n'existe pas déjà. */
void ajouter_cours(Cours **liste, const char *titre) {
    if (trouver_cours(*liste, titre)) {
        ui_erreur("Un cours avec ce nom existe déjà.");
        return;
    }
    Cours *nv = creer_cours(titre);
    if (!nv) return;
    if (!*liste) { *liste = nv; return; }
    Cours *tmp = *liste;
    while (tmp->suivant) tmp = tmp->suivant;
    tmp->suivant = nv;
    char msg[300];
    snprintf(msg, sizeof(msg), "Cours \"%s\" créé.", titre);
    ui_succes(msg);
}

/* Supprime un cours et libère tous ses chapitres, leçons, mots-clés et QCM. */
void supprimer_cours(Cours **liste, const char *titre) {
    Cours *prev = NULL, *curr = *liste;
    while (curr) {
        if (strcmp(curr->titre, titre) == 0) {
            if (prev) prev->suivant = curr->suivant;
            else      *liste        = curr->suivant;
            /* Libération complète */
            Chapitre *ch = curr->premier_chapitre;
            while (ch) {
                Lecon *l = ch->lecons;
                while (l) {
                    Lecon *tl = l; l = l->suivant;
                    liberer_mots_cles(tl->mots_cles);
                    liberer_questions(tl->questions);
                    free(tl->titre); free(tl->contenu); free(tl);
                }
                Chapitre *tc = ch; ch = ch->suivant;
                free(tc->titre); free(tc);
            }
            free(curr->titre); free(curr);
            ui_succes("Cours supprimé.");
            return;
        }
        prev = curr; curr = curr->suivant;
    }
    ui_erreur("Cours introuvable.");
}

/* Recherche un cours par son titre. */
Cours* trouver_cours(Cours *liste, const char *titre) {
    for (Cours *c = liste; c; c = c->suivant)
        if (strcmp(c->titre, titre) == 0) return c;
    return NULL;
}

/* Compte le nombre de cours dans la liste chaînée. */
int compter_cours(Cours *liste) {
    int n = 0;
    for (Cours *c = liste; c; c = c->suivant) n++;
    return n;
}

/* Affiche les cours disponibles avec le nombre de chapitres. */
void afficher_liste_cours(Cours *liste) {
    if (!liste) { ui_info("Aucun cours disponible."); return; }
    ui_sous_titre("Liste des cours disponibles");
    int i = 1;
    for (Cours *c = liste; c; c = c->suivant) {
        int nch = compter_chapitres(c);
        printf("  %2d. %-30s  %d chapitre(s)\n", i++, c->titre, nch);
    }
}

/* Libère toute la structure des cours avant la fin du programme. */
void liberer_tous_les_cours(Cours *liste) {
    Cours *c = liste;
    while (c) {
        Chapitre *ch = c->premier_chapitre;
        while (ch) {
            Lecon *l = ch->lecons;
            while (l) {
                Lecon *tl = l; l = l->suivant;
                liberer_mots_cles(tl->mots_cles);
                liberer_questions(tl->questions);
                free(tl->titre); free(tl->contenu); free(tl);
            }
            Chapitre *tc = ch; ch = ch->suivant;
            free(tc->titre); free(tc);
        }
        Cours *tc = c; c = c->suivant;
        free(tc->titre); free(tc);
    }
}

/* ════════════════════════════════════════════════════════
   GESTION DES CHAPITRES
   ════════════════════════════════════════════════════════ */

Chapitre* creer_chapitre(const char *titre) {
    Chapitre *ch = malloc(sizeof(Chapitre));
    if (!ch) return NULL;
    ch->titre   = strdup_securise(titre);
    ch->lecons  = NULL;
    ch->suivant = NULL;
    return ch;
}

/* Ajoute un chapitre en fin de liste dans le cours donné. */
void ajouter_chapitre(Cours *c, const char *titre) {
    if (trouver_chapitre(c, titre)) {
        ui_erreur("Un chapitre avec ce nom existe déjà.");
        return;
    }
    Chapitre *nv = creer_chapitre(titre);
    if (!nv) return;
    if (!c->premier_chapitre) { c->premier_chapitre = nv; }
    else {
        Chapitre *tmp = c->premier_chapitre;
        while (tmp->suivant) tmp = tmp->suivant;
        tmp->suivant = nv;
    }
    ui_succes("Chapitre ajouté.");
}

/* Supprime un chapitre et toutes les leçons qu'il contient. */
void supprimer_chapitre(Cours *c, const char *titre) {
    Chapitre *prev = NULL, *curr = c->premier_chapitre;
    while (curr) {
        if (strcmp(curr->titre, titre) == 0) {
            if (prev) prev->suivant         = curr->suivant;
            else      c->premier_chapitre   = curr->suivant;
            Lecon *l = curr->lecons;
            while (l) {
                Lecon *tl = l; l = l->suivant;
                liberer_mots_cles(tl->mots_cles);
                liberer_questions(tl->questions);
                free(tl->titre); free(tl->contenu); free(tl);
            }
            free(curr->titre); free(curr);
            ui_succes("Chapitre supprimé.");
            return;
        }
        prev = curr; curr = curr->suivant;
    }
    ui_erreur("Chapitre introuvable.");
}

/* Remplace le titre d'un chapitre. */
void renommer_chapitre(Chapitre *ch, const char *nv) {
    free(ch->titre);
    ch->titre = strdup_securise(nv);
    ui_succes("Chapitre renommé.");
}

/* Recherche un chapitre dans un cours. */
Chapitre* trouver_chapitre(Cours *c, const char *titre) {
    for (Chapitre *ch = c->premier_chapitre; ch; ch = ch->suivant)
        if (strcmp(ch->titre, titre) == 0) return ch;
    return NULL;
}

/* Accès par index 0-based utilisé par les menus de sélection. */
Chapitre* chapitre_par_index(Cours *c, int idx) {
    Chapitre *ch = c->premier_chapitre;
    for (int i = 0; ch && i < idx; i++) ch = ch->suivant;
    return ch;
}

/* Compte les chapitres d'un cours. */
int compter_chapitres(Cours *c) {
    int n = 0;
    for (Chapitre *ch = c->premier_chapitre; ch; ch = ch->suivant) n++;
    return n;
}

/* ════════════════════════════════════════════════════════
   GESTION DES LEÇONS
   ════════════════════════════════════════════════════════ */

Lecon* creer_lecon(const char *titre, const char *contenu) {
    Lecon *l = malloc(sizeof(Lecon));
    if (!l) return NULL;
    l->id        = prochain_id_lecon();
    l->titre     = strdup_securise(titre);
    l->contenu   = strdup_securise(contenu);
    l->mots_cles = NULL;
    l->questions = NULL;
    l->suivant   = NULL;
    return l;
}

/* Ajoute une leçon à la fin de la liste des leçons d'un chapitre. */
void ajouter_lecon(Chapitre *ch, const char *titre, const char *contenu) {
    if (trouver_lecon(ch, titre)) {
        ui_erreur("Une leçon avec ce nom existe déjà.");
        return;
    }
    Lecon *nv = creer_lecon(titre, contenu);
    if (!nv) return;
    if (!ch->lecons) { ch->lecons = nv; }
    else {
        Lecon *tmp = ch->lecons;
        while (tmp->suivant) tmp = tmp->suivant;
        tmp->suivant = nv;
    }
    ui_succes("Leçon ajoutée.");
}

/* Supprime une leçon et libère ses mots-clés et ses questions. */
void supprimer_lecon(Chapitre *ch, const char *titre) {
    Lecon *prev = NULL, *curr = ch->lecons;
    while (curr) {
        if (strcmp(curr->titre, titre) == 0) {
            if (prev) prev->suivant = curr->suivant;
            else      ch->lecons   = curr->suivant;
            liberer_mots_cles(curr->mots_cles);
            liberer_questions(curr->questions);
            free(curr->titre); free(curr->contenu); free(curr);
            ui_succes("Leçon supprimée.");
            return;
        }
        prev = curr; curr = curr->suivant;
    }
    ui_erreur("Leçon introuvable.");
}

/* Remplace le titre d'une leçon. */
void renommer_lecon(Lecon *l, const char *nv) {
    free(l->titre);
    l->titre = strdup_securise(nv);
    ui_succes("Leçon renommée.");
}

/* Recherche une leçon par son titre dans un chapitre. */
Lecon* trouver_lecon(Chapitre *ch, const char *titre) {
    for (Lecon *l = ch->lecons; l; l = l->suivant)
        if (strcmp(l->titre, titre) == 0) return l;
    return NULL;
}

/* Retourne la leçon située à un indice donné. */
Lecon* lecon_par_index(Chapitre *ch, int idx) {
    Lecon *l = ch->lecons;
    for (int i = 0; l && i < idx; i++) l = l->suivant;
    return l;
}

/* Compte les leçons d'un chapitre. */
int compter_lecons(Chapitre *ch) {
    int n = 0;
    for (Lecon *l = ch->lecons; l; l = l->suivant) n++;
    return n;
}

/* ════════════════════════════════════════════════════════
   AFFICHAGE RÉCURSIF
   ════════════════════════════════════════════════════════ */

/* Affiche récursivement les leçons d'un même chapitre. */
void afficher_lecon_recursive(Lecon *l, int niveau) {
    if (!l) return;   /* cas de base */
    for (int i = 0; i < niveau; i++) printf("  ");
    printf("  #%d %s\n", l->id, l->titre);
    afficher_lecon_recursive(l->suivant, niveau);   /* récursion */
}

/* Affiche récursivement les chapitres et leurs leçons. */
void afficher_chapitre_recursif(Chapitre *ch, int niveau) {
    if (!ch) return;  /* cas de base */
    for (int i = 0; i < niveau; i++) printf("  ");
    printf("- %s\n", ch->titre);
    if (ch->lecons)
        afficher_lecon_recursive(ch->lecons, niveau + 1);
    else {
        for (int i = 0; i < niveau + 1; i++) printf("  ");
        printf("(aucune leçon)\n");
    }
    afficher_chapitre_recursif(ch->suivant, niveau);  /* récursion */
}

/* Point d'entrée de l'affichage récursif d'un cours complet. */
void afficher_cours_complet(Cours *c) {
    if (!c) return;
    ui_sous_titre(c->titre);
    if (!c->premier_chapitre) {
        ui_info("Ce cours ne contient aucun chapitre.");
        return;
    }
    /* Appel récursif initial */
    afficher_chapitre_recursif(c->premier_chapitre, 1);
}
