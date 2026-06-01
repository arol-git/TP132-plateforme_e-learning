/*
 * main.c - Point d'entrée de la plateforme C-LMS.
 * Gère les menus Professeur et Étudiant.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "syllabus.h"
#include "qcm.h"
#include "contenu.h"
#include "progression.h"
#include "persistence.h"
#include "utils.h"

/* État global partagé entre les menus. */
static Cours      *liste_cours   = NULL;
static Cours      *cours_courant = NULL;
static Progression *prog         = NULL;

/* Prototypes des menus utilisés uniquement dans ce fichier. */
static void menu_principal(void);
static void menu_professeur(void);
static void menu_etudiant(void);
static void menu_edition_cours(Cours *c);
static void menu_gestion_mots_cles(Lecon *l);
static void menu_gestion_qcm(Lecon *l);

/* ════════════════════════════════════════════════════════
  SÉLECTEURS INTERACTIFS
   ════════════════════════════════════════════════════════ */

/* Affiche les cours et retourne celui choisi par l'utilisateur. */
static Cours* selectionner_cours(void) {
    int nb = compter_cours(liste_cours);
    if (nb == 0) { ui_erreur("Aucun cours disponible. Créez-en un d'abord."); return NULL; }
    const char **noms = malloc((size_t)nb * sizeof(char*));
    Cours *c = liste_cours;
    for (int i = 0; i < nb; i++) { noms[i] = c->titre; c = c->suivant; }
    int idx = ui_choisir_dans_liste(noms, nb, "Choisir un cours");
    free(noms);
    if (idx < 0) return NULL;
    c = liste_cours;
    for (int i = 0; i < idx; i++) c = c->suivant;
    return c;
}

/* Affiche les chapitres d'un cours et retourne celui choisi. */
static Chapitre* selectionner_chapitre(Cours *c) {
    int nb = compter_chapitres(c);
    if (nb == 0) { ui_erreur("Ce cours ne contient aucun chapitre."); return NULL; }
    const char **noms = malloc((size_t)nb * sizeof(char*));
    Chapitre *ch = c->premier_chapitre;
    for (int i = 0; i < nb; i++) { noms[i] = ch->titre; ch = ch->suivant; }
    int idx = ui_choisir_dans_liste(noms, nb, "Choisir un chapitre");
    free(noms);
    if (idx < 0) return NULL;
    return chapitre_par_index(c, idx);
}

/* Affiche les leçons d'un chapitre et retourne celle choisie. */
static Lecon* selectionner_lecon(Chapitre *ch) {
    int nb = compter_lecons(ch);
    if (nb == 0) { ui_erreur("Ce chapitre ne contient aucune leçon."); return NULL; }
    const char **noms = malloc((size_t)nb * sizeof(char*));
    Lecon *l = ch->lecons;
    for (int i = 0; i < nb; i++) { noms[i] = l->titre; l = l->suivant; }
    int idx = ui_choisir_dans_liste(noms, nb, "Choisir une leçon");
    free(noms);
    if (idx < 0) return NULL;
    return lecon_par_index(ch, idx);
}

/* ════════════════════════════════════════════════════════
   MAIN
   ════════════════════════════════════════════════════════ */

int main(void) {
    clean();
    ui_titre("C-LMS - Plateforme Pedagogique", 50);

    /* Chargement des données sauvegardées au démarrage. */
    liste_cours = charger_tous_les_cours("database.json");
    if (!liste_cours)
        ui_info("Aucune sauvegarde trouvée. Base vide.");
    else
        ui_succes("Base de données chargée.");

    prog = charger_progression("database.json");
    menu_principal();

    /* Sauvegarde finale et libération de la mémoire avant de quitter. */
    printf("\n");
    if (sauvegarder_base(liste_cours, prog, "database.json"))
        ui_succes("Cours sauvegardés dans database.json.");
    else
        ui_erreur("Erreur lors de la sauvegarde !");

    liberer_tous_les_cours(liste_cours);
    liberer_progression(prog);
    return 0;
}

/* ════════════════════════════════════════════════════════
   MENU PRINCIPAL
   ════════════════════════════════════════════════════════ */

static void menu_principal(void) {
    int choix;
    do {
        clean();
        ui_titre("MENU PRINCIPAL", 50);
        if (cours_courant) {
            char ligne[256];
            snprintf(ligne, sizeof(ligne), "Cours actif : %s", cours_courant->titre);
            ui_menu_texte(ligne, 50);
        }
        ui_cadre_ligne(50);
        ui_menu_option(1, "Espace Professeur", 50);
        ui_menu_option(2, "Espace Etudiant", 50);
        ui_menu_option(0, "Quitter", 50);
        ui_cadre_ligne(50);
        choix = lire_entier(0, 2);

        /* Redirection vers l'espace demandé. */
        switch (choix) {
            case 1: menu_professeur(); break;
            case 2: menu_etudiant();   break;
        }
    } while (choix != 0);
}

/* ════════════════════════════════════════════════════════
   ESPACE PROFESSEUR
   ════════════════════════════════════════════════════════ */

static void menu_professeur(void) {
    int choix;
    do {
        clean();
        ui_titre("ESPACE PROFESSEUR", 50);
        ui_cadre_ligne(50);
        ui_menu_option(1, "Creer un nouveau cours", 50);
        ui_menu_option(2, "Supprimer un cours", 50);
        ui_menu_option(3, "Selectionner un cours a editer", 50);
        ui_menu_option(4, "Editer le cours selectionne", 50);
        ui_menu_option(5, "Visualiser tous les cours", 50);
        ui_menu_option(6, "Sauvegarder maintenant", 50);
        ui_menu_option(0, "Retour", 50);
        ui_cadre_ligne(50);
        choix = lire_entier(0, 6);

        /* Création d'un nouveau cours. */
        if (choix == 1) {
            ui_sous_titre("Nouveau cours");
            printf("  Titre du cours : ");
            char *t = lire_chaine_saisie();
            if (t && strlen(t) > 0) ajouter_cours(&liste_cours, t);
            free(t);

        /* Suppression complète d'un cours après confirmation. */
        } else if (choix == 2) {
            ui_sous_titre("Supprimer un cours");
            Cours *c = selectionner_cours();
            if (c && ui_confirmer("Supprimer ce cours et tout son contenu ?")) {
                if (cours_courant == c) cours_courant = NULL;
                supprimer_cours(&liste_cours, c->titre);
            }

        /* Sélection du cours qui sera modifié dans le menu d'édition. */
        } else if (choix == 3) {
            cours_courant = selectionner_cours();
            if (cours_courant) {
                char msg[300];
                snprintf(msg, sizeof(msg), "Cours \"%s\" sélectionné.", cours_courant->titre);
                ui_succes(msg);
            }

        /* Ouverture du sous-menu d'édition du cours sélectionné. */
        } else if (choix == 4) {
            if (!cours_courant) {
                ui_erreur("Aucun cours sélectionné. Utilisez l'option 3.");
            } else {
                menu_edition_cours(cours_courant);
            }

        /* Affichage simple de tous les cours enregistrés. */
        } else if (choix == 5) {
            afficher_liste_cours(liste_cours);

        /* Sauvegarde manuelle sans attendre la fermeture du programme. */
        } else if (choix == 6) {
            if (sauvegarder_base(liste_cours, prog, "database.json"))
                ui_succes("Sauvegardé.");
            else
                ui_erreur("Erreur de sauvegarde.");
        }
        if (choix != 0) pause_console();
    } while (choix != 0);
}

/* ════════════════════════════════════════════════════════
   ÉDITION D'UN COURS
   ════════════════════════════════════════════════════════ */

static void menu_edition_cours(Cours *c) {
    int choix;
    do {
        clean();
        ui_titre("EDITION DU COURS", 50);
        {
            char ligne[256];
            snprintf(ligne, sizeof(ligne), "Cours : %s", c->titre);
            ui_menu_texte(ligne, 50);
        }
        ui_cadre_ligne(50);
        ui_menu_option(1, "Visualiser (arborescence)", 50);
        ui_menu_option(2, "Ajouter un chapitre", 50);
        ui_menu_option(3, "Ajouter une lecon", 50);
        ui_menu_option(4, "Modifier le contenu d'une lecon", 50);
        ui_menu_option(5, "Renommer un chapitre", 50);
        ui_menu_option(6, "Renommer une lecon", 50);
        ui_menu_option(7, "Supprimer un chapitre", 50);
        ui_menu_option(8, "Supprimer une lecon", 50);
        ui_menu_option(9, "Gerer les mots-cles d'une lecon", 50);
        ui_menu_option(10, "Gerer le QCM d'une lecon", 50);
        ui_menu_option(0, "Retour", 50);
        ui_cadre_ligne(50);
        choix = lire_entier(0, 10);

        /* Affichage récursif de l'arborescence du cours. */
        if (choix == 1) {
            afficher_cours_complet(c);

        /* Ajout d'un chapitre dans le cours. */
        } else if (choix == 2) {
            ui_sous_titre("Ajouter un chapitre");
            printf("  Titre : ");
            char *t = lire_chaine_saisie();
            if (t && strlen(t) > 0) ajouter_chapitre(c, t);
            free(t);

        /* Ajout d'une leçon dans un chapitre choisi. */
        } else if (choix == 3) {
            ui_sous_titre("Ajouter une leçon");
            Chapitre *ch = selectionner_chapitre(c);
            if (!ch) continue;
            printf("  Titre de la leçon : ");
            char *lt = lire_chaine_saisie();
            printf("  Contenu           : ");
            char *lc = lire_chaine_saisie();
            if (lt && lc && strlen(lt) > 0) ajouter_lecon(ch, lt, lc);
            free(lt); free(lc);

        /* Modification du texte d'une leçon. */
        } else if (choix == 4) {
            ui_sous_titre("Modifier le contenu");
            Chapitre *ch = selectionner_chapitre(c);
            if (!ch) continue;
            Lecon *l = selectionner_lecon(ch);
            if (!l) continue;
            printf("  Contenu actuel : %s\n\n", l->contenu);
            printf("  Nouveau contenu : ");
            char *nc = lire_chaine_saisie();
            if (nc) modifier_contenu_lecon(l, nc);
            free(nc);

        /* Renommage d'un chapitre existant. */
        } else if (choix == 5) {
            ui_sous_titre("Renommer un chapitre");
            Chapitre *ch = selectionner_chapitre(c);
            if (!ch) continue;
            printf("  Nouveau nom : ");
            char *nv = lire_chaine_saisie();
            if (nv && strlen(nv) > 0) renommer_chapitre(ch, nv);
            free(nv);

        /* Renommage d'une leçon existante. */
        } else if (choix == 6) {
            ui_sous_titre("Renommer une leçon");
            Chapitre *ch = selectionner_chapitre(c);
            if (!ch) continue;
            Lecon *l = selectionner_lecon(ch);
            if (!l) continue;
            printf("  Nouveau nom : ");
            char *nv = lire_chaine_saisie();
            if (nv && strlen(nv) > 0) renommer_lecon(l, nv);
            free(nv);

        /* Suppression d'un chapitre et de ses leçons. */
        } else if (choix == 7) {
            ui_sous_titre("Supprimer un chapitre");
            Chapitre *ch = selectionner_chapitre(c);
            if (!ch) continue;
            if (ui_confirmer("Supprimer ce chapitre et toutes ses leçons ?"))
                supprimer_chapitre(c, ch->titre);
            else
                ui_info("Annulé.");

        /* Suppression d'une seule leçon. */
        } else if (choix == 8) {
            ui_sous_titre("Supprimer une leçon");
            Chapitre *ch = selectionner_chapitre(c);
            if (!ch) continue;
            Lecon *l = selectionner_lecon(ch);
            if (!l) continue;
            if (ui_confirmer("Supprimer cette leçon ?"))
                supprimer_lecon(ch, l->titre);
            else
                ui_info("Annulé.");

        /* Gestion de la liste des mots-clés d'une leçon. */
        } else if (choix == 9) {
            ui_sous_titre("Mots-clés");
            Chapitre *ch = selectionner_chapitre(c);
            if (!ch) continue;
            Lecon *l = selectionner_lecon(ch);
            if (!l) continue;
            menu_gestion_mots_cles(l);

        /* Gestion des questions QCM d'une leçon. */
        } else if (choix == 10) {
            ui_sous_titre("QCM");
            Chapitre *ch = selectionner_chapitre(c);
            if (!ch) continue;
            Lecon *l = selectionner_lecon(ch);
            if (!l) continue;
            menu_gestion_qcm(l);
        }
        if (choix != 0) pause_console();
    } while (choix != 0);
}

/* ════════════════════════════════════════════════════════
   GESTION DES MOTS-CLÉS
   ════════════════════════════════════════════════════════ */

static void menu_gestion_mots_cles(Lecon *l) {
    int choix;
    do {
        clean();
        ui_titre("MOTS-CLES", 44);
        afficher_mots_cles(l);
        ui_cadre_ligne(44);
        ui_menu_option(1, "Ajouter", 44);
        ui_menu_option(2, "Supprimer", 44);
        ui_menu_option(0, "Retour", 44);
        ui_cadre_ligne(44);
        choix = lire_entier(0, 2);

        /* Ajout d'un mot-clé à la leçon. */
        if (choix == 1) {
            printf("  Nouveau mot-clé : ");
            char *m = lire_chaine_saisie();
            if (m && strlen(m) > 0) ajouter_mot_cle(l, m);
            free(m);

        } else if (choix == 2) {
            /* Construction d'un tableau temporaire pour réutiliser le sélecteur. */
            int nb = 0;
            for (MotCle *m = l->mots_cles; m; m = m->suivant) nb++;
            if (nb == 0) { ui_erreur("Aucun mot-clé à supprimer."); continue; }
            const char **noms = malloc((size_t)nb * sizeof(char*));
            MotCle *m = l->mots_cles;
            for (int i = 0; i < nb; i++) { noms[i] = m->mot; m = m->suivant; }
            int idx = ui_choisir_dans_liste(noms, nb, "Mot-clé à supprimer");
            if (idx >= 0) supprimer_mot_cle(l, noms[idx]);
            free(noms);
        }
        if (choix != 0) pause_console();
    } while (choix != 0);
}

/* ════════════════════════════════════════════════════════
   GESTION DU QCM
   ════════════════════════════════════════════════════════ */

static void menu_gestion_qcm(Lecon *l) {
    int choix;
    do {
        clean();
        ui_titre("QCM DE LA LECON", 44);
        afficher_questions(l);
        ui_cadre_ligne(44);
        ui_menu_option(1, "Ajouter une question", 44);
        ui_menu_option(2, "Supprimer une question", 44);
        ui_menu_option(3, "Tester le QCM", 44);
        ui_menu_option(0, "Retour", 44);
        ui_cadre_ligne(44);
        choix = lire_entier(0, 3);

        /* Création d'une question avec exactement quatre propositions. */
        if (choix == 1) {
            ui_sous_titre("Nouvelle question");
            printf("  Texte de la question : ");
            char *tq = lire_chaine_saisie();
            if (!tq || strlen(tq) == 0) { free(tq); continue; }
            Question *q = creer_question(tq); free(tq);

            printf("  Entrez les 4 propositions.\n");
            for (int i = 0; i < 4; i++) {
                printf("  Proposition %d : ", i + 1);
                char *tp = lire_chaine_saisie();
                printf("  Correcte ? (1=oui / 0=non) ");
                int vrai = lire_entier(0, 1);
                if (tp && strlen(tp) > 0) ajouter_proposition(q, tp, vrai);
                free(tp);
            }
            ajouter_question(l, q);
            ui_succes("Question ajoutée.");

        /* Suppression d'une question choisie dans la liste. */
        } else if (choix == 2) {
            int nb = 0;
            for (Question *q = l->questions; q; q = q->suivant) nb++;
            if (nb == 0) { ui_erreur("Aucune question."); continue; }
            const char **textes = malloc((size_t)nb * sizeof(char*));
            Question *q = l->questions;
            for (int i = 0; i < nb; i++) { textes[i] = q->texte; q = q->suivant; }
            int idx = ui_choisir_dans_liste(textes, nb, "Question à supprimer");
            free(textes);
            if (idx >= 0 && ui_confirmer("Supprimer cette question ?"))
                supprimer_question(l, idx + 1);
            else if (idx >= 0)
                ui_info("Annulé.");

        /* Test du QCM côté professeur et mise à jour de la progression. */
        } else if (choix == 3) {
            int score = passer_qcm_lecon(l);
            int total = 0;
            for (Question *q = l->questions; q; q = q->suivant) total++;
            ajouter_score(prog, score, total);
            if (total > 0 && score == total && !est_terminee(prog, l->titre)) {
                marquer_terminee(prog, l->titre);
                ui_succes("Score parfait ! Leçon marquée comme terminée.");
            }
        }
        if (choix != 0) pause_console();
    } while (choix != 0);
}

/* ════════════════════════════════════════════════════════
   ESPACE ÉTUDIANT
   ════════════════════════════════════════════════════════ */

static void menu_etudiant(void) {
    int choix;
    do {
        clean();
        ui_titre("ESPACE ETUDIANT", 50);
        if (cours_courant) {
            char ligne[256];
            snprintf(ligne, sizeof(ligne), "Cours actif : %s", cours_courant->titre);
            ui_menu_texte(ligne, 50);
        } else {
            ui_menu_texte("Aucun cours sélectionné", 50);
        }
        ui_cadre_ligne(50);
        ui_menu_option(1, "Choisir un cours", 50);
        ui_menu_option(2, "Consulter le cours (arborescence)", 50);
        ui_menu_option(3, "Lire une lecon", 50);
        ui_menu_option(4, "Repondre a un QCM", 50);
        ui_menu_option(5, "Rechercher par mot-cle", 50);
        ui_menu_option(6, "Ma progression", 50);
        ui_menu_option(7, "Marquer une lecon comme terminee", 50);
        ui_menu_option(8, "Demarquer une lecon", 50);
        ui_menu_option(0, "Retour", 50);
        ui_cadre_ligne(50);
        choix = lire_entier(0, 8);

        /* Choix du cours à consulter. */
        if (choix == 1) {
            cours_courant = selectionner_cours();
            if (cours_courant) {
                char msg[300];
                snprintf(msg, sizeof(msg), "Cours \"%s\" sélectionné.", cours_courant->titre);
                ui_succes(msg);
            }

        /* Affichage de l'arborescence du cours. */
        } else if (choix == 2) {
            if (!cours_courant) { ui_erreur("Choisissez un cours (option 1)."); continue; }
            afficher_cours_complet(cours_courant);

        /* Lecture d'une leçon et proposition de marquage comme terminée. */
        } else if (choix == 3) {
            if (!cours_courant) { ui_erreur("Choisissez un cours (option 1)."); continue; }
            Chapitre *ch = selectionner_chapitre(cours_courant);
            if (!ch) continue;
            Lecon *l = selectionner_lecon(ch);
            if (!l) continue;
            afficher_contenu_lecon(l);
            /* Proposition de marquer la leçon comme terminée. */
            if (!est_terminee(prog, l->titre)) {
                if (ui_confirmer("Marquer cette leçon comme terminée ?"))
                    marquer_terminee(prog, l->titre);
            } else {
                ui_info("Leçon déjà marquée comme terminée.");
            }

        /* Passage d'un QCM par l'étudiant. */
        } else if (choix == 4) {
            if (!cours_courant) { ui_erreur("Choisissez un cours (option 1)."); continue; }
            Chapitre *ch = selectionner_chapitre(cours_courant);
            if (!ch) continue;
            Lecon *l = selectionner_lecon(ch);
            if (!l) continue;
            int total = 0;
            for (Question *q = l->questions; q; q = q->suivant) total++;
            int score = passer_qcm_lecon(l);
            ajouter_score(prog, score, total);
            if (total > 0 && score == total && !est_terminee(prog, l->titre)) {
                marquer_terminee(prog, l->titre);
                ui_succes("Score parfait ! Leçon marquée comme terminée.");
            }

        /* Recherche dans les titres et les mots-clés des leçons. */
        } else if (choix == 5) {
            if (!cours_courant) { ui_erreur("Choisissez un cours (option 1)."); continue; }
            ui_sous_titre("Recherche");
            printf("  Mot-clé : ");
            char *mot = lire_chaine_saisie();
            if (mot && strlen(mot) > 0) rechercher_mot_cle_cours(cours_courant, mot);
            free(mot);

        /* Affichage de la progression actuelle. */
        } else if (choix == 6) {
            afficher_progression(prog, cours_courant);

        /* Marquage manuel d'une leçon comme terminée. */
        } else if (choix == 7) {
            if (!cours_courant) { ui_erreur("Choisissez un cours (option 1)."); continue; }
            Chapitre *ch = selectionner_chapitre(cours_courant);
            if (!ch) continue;
            Lecon *l = selectionner_lecon(ch);
            if (!l) continue;
            marquer_terminee(prog, l->titre);
            if (est_terminee(prog, l->titre))
                ui_succes("Leçon marquée comme terminée.");

        /* Retrait du marquage d'une leçon terminée. */
        } else if (choix == 8) {
            if (!cours_courant) { ui_erreur("Choisissez un cours (option 1)."); continue; }
            Chapitre *ch = selectionner_chapitre(cours_courant);
            if (!ch) continue;
            Lecon *l = selectionner_lecon(ch);
            if (!l) continue;
            if (!est_terminee(prog, l->titre))
                ui_info("Cette leçon n'est pas marquée comme terminée.");
            else
                demarquer_terminee(prog, l->titre);
        }
        if (choix != 0) pause_console();
    } while (choix != 0);
}
