#ifndef UTILS_H
#define UTILS_H

/* Fonctions de saisie et de manipulation de chaînes. */
char* lire_chaine_saisie(void);
void  supprimer_saut_ligne(char *str);
char* strdup_securise(const char *src);
void  vider_buffer(void);
int   lire_entier(int min, int max);
void  clean(void);
void  pause_console(void);

/* Petites fonctions d'affichage utilisées par les menus console. */
void ui_ligne(int largeur);
void ui_titre(const char *txt, int largeur);
void ui_sous_titre(const char *txt);
void ui_cadre_ligne(int largeur);
void ui_menu_texte(const char *txt, int largeur);
void ui_menu_option(int num, const char *txt, int largeur);
void ui_succes(const char *msg);
void ui_erreur(const char *msg);
void ui_info(const char *msg);
void ui_barre_progression(int val, int max, int largeur);
int  ui_confirmer(const char *question);
int  ui_choisir_dans_liste(const char **items, int nb, const char *titre);

#endif
