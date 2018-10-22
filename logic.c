#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EXPR_LEN_LIM    1024

enum op_type {
    OP_RBRACKET = -1,
    OP_NOT = 0,
    OP_AND,
    OP_OR,
    OP_IMPLY,
    OP_MUTIMPLY,
    OP_COUNT,
    OP_LBRACKET,
    OP_VAR,
    OP_VAR_END = OP_VAR + 26,
    OP_INVALID = 0xff
};

struct expr_tree_node {
    enum op_type op;
    struct expr_tree_node *lch, *rch;
};

void ensure_sane(const char *input, int pos, const char *msg)
{
    if (pos != -1) {
        puts(input);
        int i;
        for (int i = 0; i < pos; ++i) putchar(' ');
        putchar('^');
        putchar('\n');
        printf("%s\n", msg);
    }
}

void dump_tokens(FILE *f, enum op_type *list)
{
    for (; *list != OP_INVALID; ++list) switch (*list) {
        case OP_LBRACKET: fputc('(', f); break;
        case OP_RBRACKET: fputc(')', f); break;
        case OP_NOT: fprintf(f, "¬"); break;
        case OP_AND: fprintf(f, " ∧ "); break;
        case OP_OR: fprintf(f, " ∨ "); break;
        case OP_IMPLY: fprintf(f, " → "); break;
        case OP_MUTIMPLY: fprintf(f, " ↔ "); break;
        default:
            if (*list >= OP_VAR && *list < OP_VAR_END) {
                fputc('A' + (*list - OP_VAR), f);
            } else {
                fputc('?', f);
            }
    }
    fputc('\n', f);
}

void tokenize(const char *s,
    int *pos, const char **msg,
    enum op_type **tokens, struct expr_tree_node **tree_root)
{
    int sz = 0, len = strlen(s);
    enum op_type *tok = (enum op_type *)malloc((len + 1) * sizeof tok[0]);
    if (tok == NULL) {
        *pos = 0;
        *msg = "Insufficient memory [ENOMEM]";
        free(tok);
        return;
    }

    int i;
    for (i = 0; s[i] != '\0'; ++i) if (!isspace(s[i])) {
        enum op_type cur_op = OP_INVALID;
        switch (s[i]) {
            case '(': cur_op = OP_LBRACKET; break;
            case ')': cur_op = OP_RBRACKET; break;
            case '!': cur_op = OP_NOT; break;
            case '&': cur_op = OP_AND; break;
            case '|': cur_op = OP_OR; break;
            case '>': case '^': cur_op = OP_IMPLY; break;
            case '=': case '~': cur_op = OP_MUTIMPLY; break;
            default:
                if (isalpha(s[i])) cur_op = OP_VAR + (toupper(s[i]) - 'A');
        }
        if (cur_op == OP_INVALID) {
            *pos = i;
            *msg = "Invalid character";
            free(tok);
            return;
        }
        tok[sz++] = cur_op;
    }
    if (sz == 0) {
        *pos = 0;
        *msg = "Empty expression";
        free(tok);
        return;
    }
    tok[sz++] = OP_INVALID;
    *pos = -1;
    *msg = "Success";
    *tokens = tok;
}

int main()
{
    char s[EXPR_LEN_LIM];
    fgets(s, sizeof s, stdin);
    int len = strlen(s);
    if (s[len - 1] == '\n') s[len - 1] = '\0';

    int err_pos;
    const char *err_msg;
    int list_len;

    enum op_type *tokens;
    struct expr_tree_node *root;
    tokenize(s, &err_pos, &err_msg, &tokens, &root);
    ensure_sane(s, err_pos, err_msg);
    dump_tokens(stdout, tokens);

    return 0;
}
