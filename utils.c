#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TAILLE_BUFFER 1024

/* Supprime le '\n' ajouté par fgets en fin de chaîne. */
void supprimer_saut_ligne(char *str) {
    if (!str) return;
    size_t len = strlen(str);
    if (len > 0 && str[len-1] == '\n') str[len-1] = '\0';
}

/* Vide les caractères restants dans stdin après une saisie trop longue. */
void vider_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

/* Remplace strdup, qui n'est pas toujours disponible en C standard. */
char* strdup_securise(const char *src) {
    if (!src) return NULL;
    char *dest = malloc(strlen(src) + 1);
    if (dest) strcpy(dest, src);
    return dest;
}

/* Lit une ligne complète et retourne une chaîne allouée dynamiquement. */
char* lire_chaine_saisie(void) {
    char buffer[TAILLE_BUFFER];
    if (fgets(buffer, TAILLE_BUFFER, stdin) == NULL) return NULL;
    supprimer_saut_ligne(buffer);
    if (strlen(buffer) == TAILLE_BUFFER - 1 && buffer[TAILLE_BUFFER-2] != '\n')
        vider_buffer();
    return strdup_securise(buffer);
}

/* Lit un entier compris entre min et max, avec validation de la saisie. */
int lire_entier(int min, int max) {
    while (1) {
        printf("  > ");
        char *s = lire_chaine_saisie();
        if (!s) continue;
        int ok = 1;
        for (int i = 0; s[i]; i++) {
            if (i == 0 && s[i] == '-') continue;
            if (s[i] < '0' || s[i] > '9') { ok = 0; break; }
        }
        int n = ok ? atoi(s) : min - 1;
        free(s);
        if (n >= min && n <= max) return n;
        printf("  Entrez un nombre entre %d et %d.\n", min, max);
    }
}

/* Efface l'écran et replace le curseur en haut du terminal. */
void clean(void) {
    printf("\033[2J\033[H");
    fflush(stdout);
}

/* Attend que l'utilisateur appuie sur Entrée avant de continuer. */
void pause_console(void) {
    printf("\n  Appuyez sur Entree pour continuer...");
    char *tmp = lire_chaine_saisie();
    free(tmp);
}

/* Affiche une ligne horizontale. */
void ui_ligne(int largeur) {
    for (int i = 0; i < largeur; i++) putchar('-');
    printf("\n");
}

/* Affiche un titre encadré pour séparer les grands menus. */
void ui_titre(const char *txt, int largeur) {
    int len = (int)strlen(txt);
    int pad = (largeur - len - 2) / 2;
    if (pad < 1) pad = 1;

    putchar('+');
    for (int i = 0; i < largeur - 2; i++) putchar('=');
    putchar('+');
    printf("\n|");
    for (int i = 0; i < pad; i++) putchar(' ');
    printf("%s", txt);
    int reste = largeur - 2 - pad - len;
    for (int i = 0; i < reste; i++) putchar(' ');
    printf("|\n+");
    for (int i = 0; i < largeur - 2; i++) putchar('=');
    printf("+\n");
}

/* Affiche un sous-titre simple. */
void ui_sous_titre(const char *txt) {
    printf("\n  %s\n", txt);
    printf("  ");
    for (int i = 0; txt[i]; i++) putchar('-');
    printf("\n");
}

/* Affiche la bordure horizontale d'un cadre de menu. */
void ui_cadre_ligne(int largeur) {
    putchar('+');
    for (int i = 0; i < largeur - 2; i++) putchar('-');
    printf("+\n");
}

/* Affiche une ligne de texte dans un cadre. */
void ui_menu_texte(const char *txt, int largeur) {
    int dedans = largeur - 4;
    if (dedans < 1) dedans = 1;
    printf("| %-*.*s |\n", dedans, dedans, txt ? txt : "");
}

/* Affiche une option numérotée dans un cadre de menu. */
void ui_menu_option(int num, const char *txt, int largeur) {
    char ligne[256];
    snprintf(ligne, sizeof(ligne), "%2d. %s", num, txt);
    ui_menu_texte(ligne, largeur);
}

/* Affiche un message de réussite. */
void ui_succes(const char *msg) {
    printf("  OK: %s\n", msg);
}

/* Affiche un message d'erreur. */
void ui_erreur(const char *msg) {
    printf("  ERREUR: %s\n", msg);
}

/* Affiche un message d'information. */
void ui_info(const char *msg) {
    printf("  INFO: %s\n", msg);
}

/* Affiche une barre de progression en caractères ASCII. */
void ui_barre_progression(int val, int max, int largeur) {
    if (max == 0) { printf("  [aucune]\n"); return; }
    int rempli = (val * largeur) / max;
    int pct    = (val * 100) / max;

    printf("  [");
    for (int i = 0; i < rempli; i++) printf("#");
    for (int i = rempli; i < largeur; i++) printf(".");
    printf("] %d/%d (%d%%)\n", val, max, pct);
}

/* Demande une confirmation oui/non et retourne 1 si la réponse est oui. */
int ui_confirmer(const char *question) {
    printf("  ? %s (o/n) : ", question);
    char *r = lire_chaine_saisie();
    int res = r && (r[0] == 'o' || r[0] == 'O');
    free(r);
    return res;
}

/* Affiche une liste numérotée et retourne l'indice choisi, ou -1 si annulé. */
int ui_choisir_dans_liste(const char **items, int nb, const char *titre) {
    if (nb == 0) { ui_erreur("Aucun élément disponible."); return -1; }
    ui_titre(titre, 50);
    ui_cadre_ligne(50);
    for (int i = 0; i < nb; i++)
        ui_menu_option(i + 1, items[i], 50);
    ui_menu_option(0, "Annuler", 50);
    ui_cadre_ligne(50);
    int c = lire_entier(0, nb);
    return c - 1;
}
