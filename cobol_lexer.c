/*
 * cobol_lexer.c - Analisador léxico para COBOL (Fixed Format)
 *
 * Baseado na "Especificação Textual do Compilador COBOL", seção 2.
 *
 * Compilar:  gcc -Wall -Wextra -o cobol_lexer cobol_lexer.c
 * Usar:      ./cobol_lexer [-d] programa.cob
 *              -d  inclui linhas de depuração (indicador 'D' na coluna 7)
 *
 * Para usar como biblioteca (com seu parser), compile com -DLEXER_NO_MAIN
 * e use lexer_init / lexer_next / token_free / lexer_close.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

/* ------------------------------------------------------------------ */
/* Tipos                                                               */
/* ------------------------------------------------------------------ */
typedef enum {
    TK_EOF, TK_KEYWORD, TK_IDENT, TK_NUMBER, TK_STRING, TK_PICTURE,
    TK_PERIOD, TK_COMMA, TK_SEMICOLON, TK_LPAREN, TK_RPAREN,
    TK_OPERATOR, TK_ERROR
} TokenType;

typedef struct {
    TokenType type;
    char *text;     /* lexema (palavras em maiúsculas; literais sem aspas) */
    char prefix;    /* prefixo de literal: X, B, N, Z, G ou 0 */
    int line, col;  /* posição física no arquivo (base 1) */
    char area;      /* 'A' (colunas 8-11) ou 'B' (colunas 12-72) */
} Token;

/* Severidades da seção 6 da especificação */
typedef enum { SEV_I, SEV_W, SEV_E, SEV_S } Severity;

typedef struct {
    const char *filename;
    char *src;          /* texto "lógico": só colunas 8-72, já com continuações */
    int *ln, *col;      /* linha/coluna física de cada caractere de src */
    size_t len, cap, pos;
    int include_debug;
    TokenType prev;     /* tipo do token anterior (para decidir sinal) */
    int pic_state;      /* 1 = próximo token é a cadeia PICTURE */
    int counts[4];      /* contagem de mensagens por severidade */
} Lexer;

/* ------------------------------------------------------------------ */
/* Palavras reservadas                                                 */
/* ------------------------------------------------------------------ */
static const char *KEYWORDS[] = {
    "IDENTIFICATION", "ENVIRONMENT", "DATA", "PROCEDURE", "DIVISION",
    "SECTION", "PROGRAM-ID", "AUTHOR", "CONFIGURATION", "SOURCE-COMPUTER",
    "OBJECT-COMPUTER", "INPUT-OUTPUT", "FILE-CONTROL", "SELECT", "ASSIGN",
    "TO", "FILE", "FD", "WORKING-STORAGE", "LOCAL-STORAGE", "LINKAGE",
    "PIC", "PICTURE", "IS", "VALUE", "VALUES", "OF", "IN", "REDEFINES",
    "OCCURS", "TIMES", "USAGE", "COMP", "COMP-3", "BINARY",
    "ORGANIZATION", "SEQUENTIAL", "ACCESS", "MODE", "STATUS", "RECORD",
    "DISPLAY", "ACCEPT", "PERFORM", "UNTIL", "VARYING", "FROM", "BY",
    "STOP", "RUN", "GOBACK", "EXIT", "CONTINUE",
    "IF", "ELSE", "END-IF", "THEN", "EVALUATE", "WHEN", "OTHER",
    "END-EVALUATE", "END-PERFORM",
    "MOVE", "ADD", "SUBTRACT", "MULTIPLY", "DIVIDE", "COMPUTE", "GIVING",
    "REMAINDER", "ROUNDED", "GO", "CALL", "USING", "RETURNING", "COPY",
    "OPEN", "INPUT", "OUTPUT", "EXTEND", "I-O", "READ", "WRITE", "REWRITE",
    "CLOSE", "AT", "END", "INTO", "NEXT", "SENTENCE",
    "NOT", "AND", "OR", "EQUAL", "GREATER", "LESS", "THAN",
    "ZERO", "ZEROS", "ZEROES", "SPACE", "SPACES", "HIGH-VALUE",
    "HIGH-VALUES", "LOW-VALUE", "LOW-VALUES", "QUOTE", "QUOTES", "ALL",
    NULL
};

static int is_keyword(const char *w) {
    for (int i = 0; KEYWORDS[i]; i++)
        if (strcmp(KEYWORDS[i], w) == 0) return 1;
    return 0;
}

/* ------------------------------------------------------------------ */
/* Utilitários                                                         */
/* ------------------------------------------------------------------ */
static void fatal(void) {
    fprintf(stderr, "S: memória insuficiente\n");
    exit(2);
}
static void *xmalloc(size_t n) { void *p = malloc(n); if (!p) fatal(); return p; }
static void *xrealloc(void *q, size_t n) { void *p = realloc(q, n); if (!p) fatal(); return p; }

static char *dup_range(const char *s, size_t n, int upper) {
    char *r = xmalloc(n + 1);
    for (size_t i = 0; i < n; i++)
        r[i] = upper ? (char)toupper((unsigned char)s[i]) : s[i];
    r[n] = '\0';
    return r;
}

static void report(Lexer *lx, Severity sev, int line, int col, const char *fmt, ...) {
    static const char tag[] = "IWES";
    va_list ap;
    lx->counts[sev]++;
    fprintf(stderr, "%s:%d:%d: %c: ", lx->filename, line, col, tag[sev]);
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputc('\n', stderr);
}

/* ------------------------------------------------------------------ */
/* Pré-processamento do formato fixo (seção 2.1)                       */
/*   col 1-6  : sequência (ignorada)                                   */
/*   col 7    : indicador (* / - D)                                    */
/*   col 8-72 : código                                                 */
/*   col 73-80: ignorada                                               */
/* ------------------------------------------------------------------ */
static void push(Lexer *lx, char c, int line, int col) {
    if (lx->len + 2 > lx->cap) {
        size_t n = lx->cap ? lx->cap * 2 : 4096;
        lx->src = xrealloc(lx->src, n);
        lx->ln  = xrealloc(lx->ln,  n * sizeof(int));
        lx->col = xrealloc(lx->col, n * sizeof(int));
        lx->cap = n;
    }
    lx->src[lx->len] = c;
    lx->ln[lx->len]  = line;
    lx->col[lx->len] = col;
    lx->len++;
    lx->src[lx->len] = '\0';
}

/* Substitui o getline() (POSIX) por uma versão portável (Windows/MinGW).
   Lê uma linha inteira, de qualquer tamanho; retorna -1 no fim do arquivo. */
static long read_line(char **buf, size_t *cap, FILE *f) {
    size_t n = 0;
    int c;
    if (!*buf) { *cap = 256; *buf = xmalloc(*cap); }
    while ((c = fgetc(f)) != EOF) {
        if (n + 2 > *cap) { *cap *= 2; *buf = xrealloc(*buf, *cap); }
        (*buf)[n++] = (char)c;
        if (c == '\n') break;
    }
    if (n == 0) return -1;
    (*buf)[n] = '\0';
    return (long)n;
}

static void load_source(Lexer *lx, FILE *f) {
    char *line = NULL;
    size_t lcap = 0;
    long n;
    int lineno = 0;
    char open_q = 0;    /* aspa de literal ainda aberto (0 = nenhum) */

    while ((n = read_line(&line, &lcap, f)) != -1) {
        lineno++;
        while (n > 0 && (line[n-1] == '\n' || line[n-1] == '\r')) line[--n] = '\0';
        for (long i = 0; i < n; i++) if (line[i] == '\t') line[i] = ' ';

        if (n < 7) {
            for (long i = 0; i < n; i++)
                if (line[i] != ' ') {
                    report(lx, SEV_W, lineno, 1, "linha curta (< 7 colunas) ignorada");
                    break;
                }
            continue;
        }

        char ind = line[6];
        if (ind == '*' || ind == '/') continue;                       /* comentário */
        if ((ind == 'D' || ind == 'd') && !lx->include_debug) continue; /* depuração */

        int cont = (ind == '-');
        if (!cont && ind != ' ' && ind != 'D' && ind != 'd')
            report(lx, SEV_E, lineno, 7, "indicador inválido '%c' na coluna 7", ind);

        int start = 7;
        if (cont) {
            while (start < 72 && start < n && line[start] == ' ') start++;
            if (open_q) {
                if (start < n && line[start] == open_q) {
                    start++;   /* aspa de continuação: não faz parte do literal */
                    if (lx->len > 0 && lx->src[lx->len - 1] == '\n') lx->len--;
                    lx->src[lx->len] = '\0';
                } else {
                    report(lx, SEV_E, lineno, start + 1,
                           "continuação de literal deve começar com aspas");
                }
            }
            /* continuação fora de literal: tratada como linha normal */
        } else {
            open_q = 0;
        }

        /* colunas 8-72; preenche com espaço até a coluna 72 para que um
           literal continuado preserve os brancos finais */
        for (int i = start; i < 72; i++) {
            char c = (i < n) ? line[i] : ' ';
            if (!open_q) { if (c == '\'' || c == '"') open_q = c; }
            else if (c == open_q) open_q = 0;
            push(lx, c, lineno, i + 1);
        }
        push(lx, '\n', lineno, 73);
    }
    free(line);
}

/* ------------------------------------------------------------------ */
/* API                                                                 */
/* ------------------------------------------------------------------ */
int lexer_init(Lexer *lx, const char *path, int include_debug) {
    memset(lx, 0, sizeof *lx);
    lx->filename = path;
    lx->include_debug = include_debug;
    lx->prev = TK_EOF;
    FILE *f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, "S: não foi possível abrir '%s'\n", path);
        return -1;
    }
    push(lx, '\n', 0, 0);   /* garante buffer alocado */
    lx->len = 0; lx->src[0] = '\0';
    load_source(lx, f);
    fclose(f);
    return 0;
}

void lexer_close(Lexer *lx) {
    free(lx->src); free(lx->ln); free(lx->col);
    memset(lx, 0, sizeof *lx);
}

void token_free(Token *t) { free(t->text); t->text = NULL; }

static int peekc(const Lexer *lx, size_t off) {
    return (lx->pos + off < lx->len) ? (unsigned char)lx->src[lx->pos + off] : 0;
}

static void skip_ws(Lexer *lx) {
    while (lx->pos < lx->len) {
        char c = lx->src[lx->pos];
        if (isspace((unsigned char)c)) lx->pos++;
        else if (c == '*' && peekc(lx, 1) == '>') {          /* comentário inline */
            while (lx->pos < lx->len && lx->src[lx->pos] != '\n') lx->pos++;
        } else break;
    }
}

static int prev_is_operand(const Lexer *lx) {
    return lx->prev == TK_IDENT || lx->prev == TK_NUMBER ||
           lx->prev == TK_STRING || lx->prev == TK_RPAREN ||
           lx->prev == TK_PICTURE;
}

/* [+|-] dígitos [. dígitos]  — ponto decimal só se seguido de dígito */
static void read_number(Lexer *lx, Token *t) {
    size_t start = lx->pos, p = start;
    int digits = 0;
    if (lx->src[p] == '+' || lx->src[p] == '-') p++;
    while (isdigit((unsigned char)lx->src[p])) { p++; digits++; }
    if (lx->src[p] == '.' && isdigit((unsigned char)lx->src[p + 1])) {
        p++;
        while (isdigit((unsigned char)lx->src[p])) { p++; digits++; }
    }
    if (digits > 18)
        report(lx, SEV_W, t->line, t->col, "literal numérico com mais de 18 dígitos");
    t->type = TK_NUMBER;
    t->text = dup_range(lx->src + start, p - start, 0);
    lx->pos = p;
}

/* literal alfanumérico com ' ou "; aspa dobrada = aspa literal */
static void read_string(Lexer *lx, Token *t, char prefix) {
    char q = lx->src[lx->pos++];
    size_t cap = 64, n = 0;
    char *buf = xmalloc(cap);
    int closed = 0;

    while (lx->pos < lx->len) {
        char c = lx->src[lx->pos];
        if (c == '\n') break;
        lx->pos++;
        if (c == q) {
            if (peekc(lx, 0) == (unsigned char)q) lx->pos++;   /* '' -> ' */
            else { closed = 1; break; }
        }
        if (n + 1 >= cap) { cap *= 2; buf = xrealloc(buf, cap); }
        buf[n++] = c;
    }
    buf[n] = '\0';
    if (!closed)
        report(lx, SEV_E, t->line, t->col, "literal alfanumérico não terminado");
    t->type = TK_STRING;
    t->text = buf;
    t->prefix = prefix;
}

/* Retorna 1 se produziu token, 0 em fim de arquivo */
int lexer_next(Lexer *lx, Token *t) {
    memset(t, 0, sizeof *t);
    skip_ws(lx);

    if (lx->pos >= lx->len) {
        t->type = TK_EOF;
        t->text = dup_range("<EOF>", 5, 0);
        if (lx->len) { t->line = lx->ln[lx->len - 1]; t->col = 1; }
        return 0;
    }

    size_t start = lx->pos;
    char c = lx->src[start];
    t->line = lx->ln[start];
    t->col  = lx->col[start];
    t->area = (t->col <= 11) ? 'A' : 'B';

    int was_pic = lx->pic_state;
    lx->pic_state = 0;

    /* ---- Cadeia PICTURE: PIC [IS] 9(5)V99. ------------------------ */
    if (was_pic) {
        int is_word = (toupper((unsigned char)c) == 'I' &&
                       toupper(peekc(lx, 1)) == 'S' &&
                       isspace(peekc(lx, 2)));
        if (is_word) {
            lx->pic_state = 1;           /* mantém: a cadeia vem depois do IS */
            was_pic = 0;                 /* o IS sai como palavra-chave normal */
        } else {
            size_t p = start;
            while (p < lx->len && !isspace((unsigned char)lx->src[p])) p++;
            size_t e = p;
            if (e > start && lx->src[e - 1] == '.') e--;   /* ponto final da sentença */
            if (e > start) {
                t->type = TK_PICTURE;
                t->text = dup_range(lx->src + start, e - start, 1);
                lx->pos = e;
                lx->prev = t->type;
                return 1;
            }
        }
    }

    /* ---- Literal alfanumérico ------------------------------------- */
    if (c == '\'' || c == '"') {
        read_string(lx, t, 0);
        goto done;
    }

    /* ---- Palavra (identificador, palavra reservada ou número) ----- */
    if (isalnum((unsigned char)c)) {
        size_t p = start;
        while (p < lx->len && (isalnum((unsigned char)lx->src[p]) || lx->src[p] == '-')) p++;

        /* prefixo de literal: X"1F", N"..." etc. */
        if (p - start == 1 && strchr("XxBbNnZzGg", c) &&
            (lx->src[p] == '\'' || lx->src[p] == '"')) {
            lx->pos = p;
            read_string(lx, t, (char)toupper((unsigned char)c));
            goto done;
        }

        int all_digits = 1;
        for (size_t i = start; i < p; i++)
            if (!isdigit((unsigned char)lx->src[i])) { all_digits = 0; break; }
        if (all_digits) {
            read_number(lx, t);
            goto done;
        }

        size_t q = p;
        while (q > start && lx->src[q - 1] == '-') q--;
        if (q < p)
            report(lx, SEV_E, t->line, t->col, "identificador não pode terminar com hífen");
        if (q - start > 30)
            report(lx, SEV_E, t->line, t->col,
                   "identificador com mais de 30 caracteres (%zu)", q - start);

        t->text = dup_range(lx->src + start, q - start, 1);
        t->type = is_keyword(t->text) ? TK_KEYWORD : TK_IDENT;
        lx->pos = p;

        if (t->type == TK_KEYWORD) {
            if (!strcmp(t->text, "PIC") || !strcmp(t->text, "PICTURE")) lx->pic_state = 1;
            else if (!strcmp(t->text, "IS") && was_pic) lx->pic_state = 1;
        }
        goto done;
    }

    /* ---- Número com sinal ou iniciado por ponto ------------------- */
    if ((c == '+' || c == '-' || c == '.') && !prev_is_operand(lx)) {
        int n1 = peekc(lx, 1), n2 = peekc(lx, 2);
        int starts_num;
        if (c == '.') starts_num = isdigit(n1);
        else starts_num = isdigit(n1) || (n1 == '.' && isdigit(n2));
        if (starts_num) {
            read_number(lx, t);
            goto done;
        }
    }

    /* ---- Símbolos e operadores ------------------------------------ */
    lx->pos++;
    switch (c) {
    case '.': t->type = TK_PERIOD;    t->text = dup_range(".", 1, 0); break;
    case ',': t->type = TK_COMMA;     t->text = dup_range(",", 1, 0); break;
    case ';': t->type = TK_SEMICOLON; t->text = dup_range(";", 1, 0); break;
    case '(': t->type = TK_LPAREN;    t->text = dup_range("(", 1, 0); break;
    case ')': t->type = TK_RPAREN;    t->text = dup_range(")", 1, 0); break;
    case '+': case '-': case '/': case '=': case ':':
        t->type = TK_OPERATOR; t->text = dup_range(&c, 1, 0); break;
    case '*':
        t->type = TK_OPERATOR;
        if (peekc(lx, 0) == '*') { lx->pos++; t->text = dup_range("**", 2, 0); }
        else t->text = dup_range("*", 1, 0);
        break;
    case '<': case '>':
        t->type = TK_OPERATOR;
        if (peekc(lx, 0) == '=' || (c == '<' && peekc(lx, 0) == '>')) {
            lx->pos++;
            t->text = dup_range(lx->src + start, 2, 0);
        } else t->text = dup_range(&c, 1, 0);
        break;
    default:
        t->type = TK_ERROR;
        t->text = dup_range(&c, 1, 0);
        report(lx, SEV_E, t->line, t->col, "caractere inválido '%c' (0x%02X)",
               isprint((unsigned char)c) ? c : '?', (unsigned char)c);
    }

done:
    lx->prev = t->type;
    return 1;
}

/* ------------------------------------------------------------------ */
/* Programa de teste: imprime a tabela de tokens                       */
/* ------------------------------------------------------------------ */
#ifndef LEXER_NO_MAIN
static const char *type_name(TokenType t) {
    switch (t) {
    case TK_EOF:       return "EOF";
    case TK_KEYWORD:   return "KEYWORD";
    case TK_IDENT:     return "IDENT";
    case TK_NUMBER:    return "NUMBER";
    case TK_STRING:    return "STRING";
    case TK_PICTURE:   return "PICTURE";
    case TK_PERIOD:    return "PERIOD";
    case TK_COMMA:     return "COMMA";
    case TK_SEMICOLON: return "SEMICOLON";
    case TK_LPAREN:    return "LPAREN";
    case TK_RPAREN:    return "RPAREN";
    case TK_OPERATOR:  return "OPERATOR";
    default:           return "ERROR";
    }
}

int main(int argc, char **argv) {
    int dbg = 0;
    const char *path = NULL;
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-d")) dbg = 1;
        else path = argv[i];
    }
    if (!path) {
        fprintf(stderr, "uso: %s [-d] programa.cob\n", argv[0]);
        return 2;
    }

    Lexer lx;
    if (lexer_init(&lx, path, dbg) != 0) return 2;

    Token t;
    printf("%-5s %-4s %-4s %-10s %s\n", "LINHA", "COL", "AREA", "TIPO", "LEXEMA");
    while (lexer_next(&lx, &t)) {
        printf("%-5d %-4d %-4c %-10s ", t.line, t.col, t.area, type_name(t.type));
        if (t.type == TK_STRING) {
            if (t.prefix) putchar(t.prefix);
            printf("\"%s\"\n", t.text);
        } else printf("%s\n", t.text);
        token_free(&t);
    }
    token_free(&t);

    fprintf(stderr, "\n-- %d informativa(s), %d aviso(s), %d erro(s), %d severo(s)\n",
            lx.counts[SEV_I], lx.counts[SEV_W], lx.counts[SEV_E], lx.counts[SEV_S]);
    int fail = lx.counts[SEV_E] + lx.counts[SEV_S];
    lexer_close(&lx);
    return fail ? 1 : 0;   /* erro (E/S) => compilação falha */
}
#endif