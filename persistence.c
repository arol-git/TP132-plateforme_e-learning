/*
 * persistence.c - Sauvegarde et chargement des données au format JSON.
 */
#include "persistence.h"
#include "qcm.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* SAUVEGARDE */

/* Ajoute n espaces pour rendre le fichier JSON lisible. */
static void ind(FILE *f, int n) { for (int i=0;i<n;i++) fputc(' ',f); }

/* Écrit une chaîne au format JSON en échappant les caractères spéciaux. */
static void json_str(FILE *f, const char *s) {
    if (!s) { fprintf(f,"\"\""); return; }
    fputc('"', f);
    for (const char *p = s; *p; p++) {
        switch (*p) {
            case '"':  fprintf(f,"\\\""); break;
            case '\\': fprintf(f,"\\\\"); break;
            case '\n': fprintf(f,"\\n");  break;
            case '\r': fprintf(f,"\\r");  break;
            case '\t': fprintf(f,"\\t");  break;
            default:   fputc(*p, f);
        }
    }
    fputc('"', f);
}

/* Sauvegarde la liste des mots-clés d'une leçon. */
static void save_mots(FILE *f, MotCle *m, int d) {
    fprintf(f,"[\n");
    while (m) {
        ind(f,d+2); json_str(f,m->mot);
        if (m->suivant) fputc(',',f);
        fputc('\n',f);
        m = m->suivant;
    }
    ind(f,d); fputc(']',f);
}

/* Sauvegarde les propositions d'une question. */
static void save_props(FILE *f, Proposition *p) {
    fprintf(f,"[\n");
    while (p) {
        fprintf(f,"          {\"texte\":"); json_str(f,p->texte);
        fprintf(f,",\"vrai\":%d}", p->est_vraie);
        if (p->suivant) fputc(',',f);
        fputc('\n',f);
        p = p->suivant;
    }
    fprintf(f,"        ]");
}

/* Sauvegarde les questions QCM d'une leçon. */
static void save_qcm(FILE *f, Question *q, int d) {
    fprintf(f,"[\n");
    while (q) {
        ind(f,d+2); fprintf(f,"{\n");
        ind(f,d+4); fprintf(f,"\"texte\":"); json_str(f,q->texte); fprintf(f,",\n");
        ind(f,d+4); fprintf(f,"\"props\":");
        save_props(f, q->propositions);
        fprintf(f,"\n");
        ind(f,d+2); fprintf(f,"}");
        if (q->suivant) fputc(',',f);
        fputc('\n',f);
        q = q->suivant;
    }
    ind(f,d); fputc(']',f);
}

/* Sauvegarde les leçons d'un chapitre. */
static void save_lecons(FILE *f, Lecon *l, int d) {
    fprintf(f,"[\n");
    while (l) {
        ind(f,d+2); fprintf(f,"{\n");
        ind(f,d+4); fprintf(f,"\"id\":%d,\n", l->id);
        ind(f,d+4); fprintf(f,"\"titre\":"); json_str(f,l->titre); fprintf(f,",\n");
        ind(f,d+4); fprintf(f,"\"contenu\":"); json_str(f,l->contenu); fprintf(f,",\n");
        ind(f,d+4); fprintf(f,"\"mots\":"); save_mots(f,l->mots_cles,d+4); fprintf(f,",\n");
        ind(f,d+4); fprintf(f,"\"qcm\":"); save_qcm(f,l->questions,d+4); fprintf(f,"\n");
        ind(f,d+2); fprintf(f,"}");
        if (l->suivant) fputc(',',f);
        fputc('\n',f);
        l = l->suivant;
    }
    ind(f,d); fputc(']',f);
}

/* Sauvegarde les chapitres d'un cours. */
static void save_chapitres(FILE *f, Chapitre *ch, int d) {
    fprintf(f,"[\n");
    while (ch) {
        ind(f,d+2); fprintf(f,"{\n");
        ind(f,d+4); fprintf(f,"\"titre\":"); json_str(f,ch->titre); fprintf(f,",\n");
        ind(f,d+4); fprintf(f,"\"lecons\":"); save_lecons(f,ch->lecons,d+4); fprintf(f,"\n");
        ind(f,d+2); fprintf(f,"}");
        if (ch->suivant) fputc(',',f);
        fputc('\n',f);
        ch = ch->suivant;
    }
    ind(f,d); fputc(']',f);
}

/* Sauvegarde les informations de progression de l'étudiant. */
static void save_progression(FILE *f, Progression *prog) {
    fprintf(f,"  \"progression\": {\n");
    fprintf(f,"    \"score_total\": %d,\n", prog ? prog->score_total : 0);
    fprintf(f,"    \"qcm_passes\": %d,\n", prog ? prog->qcm_passes : 0);
    fprintf(f,"    \"terminees\": [\n");
    if (prog) {
        ActiviteTerminee *a = prog->liste_terminees;
        while (a) {
            fprintf(f,"      ");
            json_str(f, a->titre_lecon);
            if (a->suivant) fputc(',', f);
            fputc('\n', f);
            a = a->suivant;
        }
    }
    fprintf(f,"    ]\n  }\n");
}

/* Sauvegarde toute la base : cours, chapitres, leçons, QCM et progression. */
int sauvegarder_base(Cours *liste, Progression *prog, const char *fichier) {
    FILE *f = fopen(fichier, "w");
    if (!f) { perror("fopen"); return 0; }
    fprintf(f,"{\n  \"cours\": [\n");
    Cours *c = liste;
    while (c) {
        fprintf(f,"    {\n");
        fprintf(f,"      \"titre\":"); json_str(f,c->titre); fprintf(f,",\n");
        fprintf(f,"      \"chapitres\":"); save_chapitres(f,c->premier_chapitre,6); fprintf(f,"\n");
        fprintf(f,"    }");
        if (c->suivant) fputc(',',f);
        fputc('\n',f);
        c = c->suivant;
    }
    fprintf(f,"  ],\n");
    save_progression(f, prog);
    fprintf(f,"}\n");
    fclose(f);
    return 1;
}

/* Sauvegarde les cours sans progression. */
int sauvegarder_tous_les_cours(Cours *liste, const char *fichier) {
    return sauvegarder_base(liste, NULL, fichier);
}

/* CHARGEMENT - parseur JSON minimaliste, sans bibliothèque externe. */

/* Avance le pointeur après les espaces, tabulations et retours à la ligne. */
static void skip_ws(const char **p) {
    while (**p && isspace((unsigned char)**p)) (*p)++;
}

/* Lit une chaîne JSON entre guillemets et la retourne allouée dynamiquement. */
static char* read_str(const char **p) {
    skip_ws(p);
    if (**p != '"') return NULL;
    (*p)++;
    /* Premier parcours : calcul de la taille utile de la chaîne. */
    size_t sz = 0;
    const char *sc = *p;
    while (*sc && *sc != '"') {
        if (*sc == '\\') { sc++; if (*sc) sc++; } else sc++;
        sz++;
    }
    char *r = malloc(sz + 1);
    char *d = r;
    while (**p && **p != '"') {
        if (**p == '\\') {
            (*p)++;
            switch (**p) {
                case '"': *d++='"'; break; case '\\': *d++='\\'; break;
                case 'n': *d++='\n'; break; case 'r': *d++='\r'; break;
                case 't': *d++='\t'; break; default: *d++=**p;
            }
        } else { *d++ = **p; }
        (*p)++;
    }
    *d = '\0';
    if (**p == '"') (*p)++;
    return r;
}

/* Lit un entier JSON. */
static int read_int(const char **p) {
    skip_ws(p);
    int v = 0, neg = 0;
    if (**p == '-') { neg = 1; (*p)++; }
    while (**p >= '0' && **p <= '9') { v = v*10 + (**p - '0'); (*p)++; }
    return neg ? -v : v;
}

/* Cherche une clé JSON et place le pointeur au début de sa valeur. */
static int seek_key(const char **p, const char *key) {
    char needle[256];
    snprintf(needle, sizeof(needle), "\"%s\"", key);
    const char *pos = strstr(*p, needle);
    if (!pos) return 0;
    *p = pos + strlen(needle);
    skip_ws(p);
    if (**p == ':') { (*p)++; skip_ws(p); }
    return 1;
}

/* Extrait un bloc {...} ou [...] complet, en tenant compte des imbrications. */
static char* extract_block(const char **p, char open, char close) {
    skip_ws(p);
    if (**p != open) return NULL;
    int depth = 0;
    const char *start = *p;
    const char *s = start;
    int in_str = 0;
    while (*s) {
        if (!in_str) {
            if (*s == '"') in_str = 1;
            else if (*s == open)  depth++;
            else if (*s == close) { depth--; if (depth==0){s++;break;} }
        } else {
            if (*s == '\\') { s++; if (*s) s++; continue; }
            if (*s == '"') in_str = 0;
        }
        s++;
    }
    size_t len = (size_t)(s - start);
    char *buf = malloc(len + 1);
    memcpy(buf, start, len);
    buf[len] = '\0';
    *p = s;
    return buf;
}

/* Reconstruit la liste chaînée des cours à partir du contenu JSON. */
static Cours* parse_tous_les_cours(const char *json) {
    Cours *liste = NULL;
    const char *p = json;

    if (!seek_key(&p, "cours")) return NULL;
    if (*p != '[') return NULL;
    p++; /* [ */

    while (*p) {
        skip_ws(&p);
        if (*p == ']') break;
        if (*p == ',') { p++; continue; }
        if (*p != '{') { p++; continue; }

        char *bloc_cours = extract_block(&p, '{', '}');
        if (!bloc_cours) break;

        const char *bp = bloc_cours;
        char *titre_c = NULL;
        if (seek_key(&bp, "titre")) titre_c = read_str(&bp);
        if (!titre_c) { free(bloc_cours); continue; }

        /* Création directe pour éviter les messages pendant le chargement. */
        Cours *c = creer_cours(titre_c);
        free(titre_c);

        /* Ajout du cours en fin de liste. */
        if (!liste) liste = c;
        else { Cours *t=liste; while(t->suivant) t=t->suivant; t->suivant=c; }

        /* Lecture des chapitres du cours courant. */
        bp = bloc_cours;
        if (seek_key(&bp, "chapitres") && *bp == '[') {
            bp++;
            while (*bp) {
                skip_ws(&bp);
                if (*bp == ']') break;
                if (*bp == ',') { bp++; continue; }
                if (*bp != '{') { bp++; continue; }

                char *bloc_ch = extract_block(&bp, '{', '}');
                if (!bloc_ch) break;

                const char *cp = bloc_ch;
                char *titre_ch = NULL;
                if (seek_key(&cp, "titre")) titre_ch = read_str(&cp);
                if (titre_ch) {
                    /* Création directe du chapitre. */
                    Chapitre *ch = creer_chapitre(titre_ch);
                    free(titre_ch);
                    if (!c->premier_chapitre) c->premier_chapitre = ch;
                    else { Chapitre *t=c->premier_chapitre; while(t->suivant) t=t->suivant; t->suivant=ch; }

                    /* Lecture des leçons du chapitre courant. */
                    cp = bloc_ch;
                    if (seek_key(&cp, "lecons") && *cp == '[') {
                        cp++;
                        while (*cp) {
                            skip_ws(&cp);
                            if (*cp == ']') break;
                            if (*cp == ',') { cp++; continue; }
                            if (*cp != '{') { cp++; continue; }

                            char *bloc_l = extract_block(&cp, '{', '}');
                            if (!bloc_l) break;

                            const char *lp = bloc_l;
                            int lid = 0;
                            char *tl = NULL, *cl_str = NULL;

                            if (seek_key(&lp, "id"))      lid    = read_int(&lp);
                            lp = bloc_l;
                            if (seek_key(&lp, "titre"))   tl     = read_str(&lp);
                            lp = bloc_l;
                            if (seek_key(&lp, "contenu")) cl_str = read_str(&lp);

                            if (tl && cl_str) {
                                Lecon *l = creer_lecon(tl, cl_str);
                                if (lid > 0) l->id = lid; /* restauration de l'id sauvegardé */
                                synchroniser_id_lecon(lid);

                                /* Ajout de la leçon en fin de liste. */
                                if (!ch->lecons) ch->lecons = l;
                                else { Lecon *t=ch->lecons; while(t->suivant) t=t->suivant; t->suivant=l; }

                                /* Lecture des mots-clés de la leçon. */
                                lp = bloc_l;
                                if (seek_key(&lp, "mots") && *lp == '[') {
                                    lp++;
                                    while (*lp) {
                                        skip_ws(&lp);
                                        if (*lp==']') break;
                                        if (*lp==',') { lp++; continue; }
                                        if (*lp=='"') {
                                            char *mot = read_str(&lp);
                                            if (mot) { ajouter_mot_cle(l,mot); free(mot); }
                                        } else lp++;
                                    }
                                }

                                /* Lecture des questions QCM de la leçon. */
                                lp = bloc_l;
                                if (seek_key(&lp, "qcm") && *lp == '[') {
                                    lp++;
                                    while (*lp) {
                                        skip_ws(&lp);
                                        if (*lp==']') break;
                                        if (*lp==',') { lp++; continue; }
                                        if (*lp!='{') { lp++; continue; }
                                        char *bq = extract_block(&lp,'{','}');
                                        if (!bq) break;
                                        const char *qp = bq;
                                        char *tq = NULL;
                                        if (seek_key(&qp,"texte")) tq = read_str(&qp);
                                        if (tq) {
                                            Question *q = creer_question(tq); free(tq);
                                            /* Lecture des propositions de la question. */
                                            qp = bq;
                                            if (seek_key(&qp,"props") && *qp=='[') {
                                                qp++;
                                                while (*qp) {
                                                    skip_ws(&qp);
                                                    if (*qp==']') break;
                                                    if (*qp==',') { qp++; continue; }
                                                    if (*qp!='{') { qp++; continue; }
                                                    char *bp2 = extract_block(&qp,'{','}');
                                                    if (!bp2) break;
                                                    const char *pp = bp2;
                                                    char *tp = NULL; int vrai = 0;
                                                    if (seek_key(&pp,"texte")) tp = read_str(&pp);
                                                    pp = bp2;
                                                    if (seek_key(&pp,"vrai")) { skip_ws(&pp); vrai = (*pp=='1'); }
                                                    if (tp) { ajouter_proposition(q,tp,vrai); free(tp); }
                                                    free(bp2);
                                                }
                                            }
                                            ajouter_question(l, q);
                                        }
                                        free(bq);
                                    }
                                }
                            }
                            free(tl); free(cl_str); free(bloc_l);
                        }
                    }
                }
                free(bloc_ch);
            }
        }
        free(bloc_cours);
    }
    return liste;
}

/* Ouvre le fichier JSON et charge uniquement la partie cours. */
Cours* charger_tous_les_cours(const char *fichier) {
    FILE *f = fopen(fichier, "r");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f); rewind(f);
    if (sz <= 2) { fclose(f); return NULL; }
    char *buf = malloc((size_t)sz + 1);
    size_t lu = fread(buf, 1, (size_t)sz, f);
    buf[lu] = '\0';
    fclose(f);
    Cours *res = parse_tous_les_cours(buf);
    free(buf);
    return res;
}

/* Ouvre le fichier JSON et charge la partie progression. */
Progression* charger_progression(const char *fichier) {
    FILE *f = fopen(fichier, "r");
    if (!f) return creer_progression();
    fseek(f, 0, SEEK_END);
    long sz = ftell(f); rewind(f);
    if (sz <= 2) { fclose(f); return creer_progression(); }

    char *buf = malloc((size_t)sz + 1);
    if (!buf) { fclose(f); return creer_progression(); }
    size_t lu = fread(buf, 1, (size_t)sz, f);
    buf[lu] = '\0';
    fclose(f);

    Progression *prog = creer_progression();
    const char *p = buf;
    if (seek_key(&p, "progression")) {
        char *bloc = extract_block(&p, '{', '}');
        if (bloc) {
            const char *bp = bloc;
            if (seek_key(&bp, "score_total")) prog->score_total = read_int(&bp);
            bp = bloc;
            if (seek_key(&bp, "qcm_passes")) prog->qcm_passes = read_int(&bp);
            bp = bloc;
            if (seek_key(&bp, "terminees") && *bp == '[') {
                bp++;
                while (*bp) {
                    skip_ws(&bp);
                    if (*bp == ']') break;
                    if (*bp == ',') { bp++; continue; }
                    if (*bp == '"') {
                        char *titre = read_str(&bp);
                        if (titre) {
                            marquer_terminee(prog, titre);
                            free(titre);
                        }
                    } else {
                        bp++;
                    }
                }
            }
            free(bloc);
        }
    }
    free(buf);
    return prog;
}
