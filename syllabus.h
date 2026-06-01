#ifndef SYLLABUS_H
#define SYLLABUS_H

/* Déclarations anticipées pour permettre aux structures de se référencer. */
typedef struct MotCle    MotCle;
typedef struct Proposition Proposition;
typedef struct Question  Question;
typedef struct Lecon     Lecon;
typedef struct Chapitre  Chapitre;
typedef struct Cours     Cours;

/* ══════════════════════════════════════════════
   STRUCTURES DE DONNÉES (listes chaînées)
   ══════════════════════════════════════════════ */

struct Lecon {
    int    id;           /* identifiant unique */
    char  *titre;        /* titre affiché dans le syllabus */
    char  *contenu;      /* texte de la leçon */
    MotCle   *mots_cles; /* liste chaînée des mots-clés */
    Question *questions; /* liste chaînée des questions du QCM */
    Lecon    *suivant;   /* leçon suivante dans le chapitre */
};

struct Chapitre {
    char     *titre;    /* nom du chapitre */
    Lecon    *lecons;   /* première leçon du chapitre */
    Chapitre *suivant;  /* chapitre suivant dans le cours */
};

struct Cours {
    char     *titre;             /* nom du cours */
    Chapitre *premier_chapitre;  /* début de la liste des chapitres */
    Cours    *suivant;           /* cours suivant dans la base */
};

/* Gestion de la liste chaînée des cours. */
Cours* creer_cours(const char *titre);
void   ajouter_cours(Cours **liste, const char *titre);
void   supprimer_cours(Cours **liste, const char *titre);
Cours* trouver_cours(Cours *liste, const char *titre);
void   liberer_tous_les_cours(Cours *liste);
int    compter_cours(Cours *liste);
void   afficher_liste_cours(Cours *liste);

/* Gestion des chapitres d'un cours. */
Chapitre* creer_chapitre(const char *titre);
void      ajouter_chapitre(Cours *c, const char *titre);
void      supprimer_chapitre(Cours *c, const char *titre);
void      renommer_chapitre(Chapitre *ch, const char *nv);
Chapitre* trouver_chapitre(Cours *c, const char *titre);
Chapitre* chapitre_par_index(Cours *c, int idx);   /* index 0-based */
int       compter_chapitres(Cours *c);

/* Gestion des leçons d'un chapitre. */
Lecon* creer_lecon(const char *titre, const char *contenu);
void   ajouter_lecon(Chapitre *ch, const char *titre, const char *contenu);
void   supprimer_lecon(Chapitre *ch, const char *titre);
void   renommer_lecon(Lecon *l, const char *nv);
Lecon* trouver_lecon(Chapitre *ch, const char *titre);
Lecon* lecon_par_index(Chapitre *ch, int idx);     /* index 0-based */
int    compter_lecons(Chapitre *ch);

/* Affichage récursif de l'arborescence du cours. */
void afficher_cours_complet(Cours *c);
void afficher_chapitre_recursif(Chapitre *ch, int niveau);
void afficher_lecon_recursive(Lecon *l, int niveau);

/* Compteur global utilisé pour donner un identifiant aux leçons. */
int prochain_id_lecon(void);
void synchroniser_id_lecon(int id);

#endif
