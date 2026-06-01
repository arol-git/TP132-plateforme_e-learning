#ifndef QCM_H
#define QCM_H

#include "syllabus.h"

/* Une proposition appartient à une question et indique si elle est vraie. */
struct Proposition {
    char *texte;                    /* texte de la proposition */
    int   est_vraie;                /* 1 si vraie, 0 sinon */
    struct Proposition *suivant;    /* proposition suivante */
};

/* Une question contient une liste chaînée de propositions. */
struct Question {
    char *texte;                       /* énoncé de la question */
    struct Proposition *propositions;  /* première proposition */
    struct Question    *suivant;       /* question suivante */
};

/* Mot-clé associé à une leçon. */
struct MotCle {
    char *mot;                 /* contenu du mot-clé */
    struct MotCle *suivant;    /* mot-clé suivant */
};

typedef struct Proposition Proposition;
typedef struct Question    Question;
typedef struct MotCle      MotCle;

/* Gestion des mots-clés d'une leçon. */
void ajouter_mot_cle(Lecon *l, const char *mot);
void supprimer_mot_cle(Lecon *l, const char *mot);
int  rechercher_mot_cle_lecon(Lecon *l, const char *mot);
void rechercher_mot_cle_cours(Cours *c, const char *mot);
void afficher_mots_cles(Lecon *l);
void liberer_mots_cles(MotCle *m);

/* Gestion des questions et propositions de QCM. */
Question*    creer_question(const char *texte);
Proposition* creer_proposition(const char *texte, int vrai);
void         ajouter_question(Lecon *l, Question *q);
void         supprimer_question(Lecon *l, int index);
void         ajouter_proposition(Question *q, const char *texte, int vrai);
void         liberer_questions(Question *q);
int          repondre_qcm(Question *q);
int          passer_qcm_lecon(Lecon *l);
void         afficher_questions(Lecon *l);

#endif
