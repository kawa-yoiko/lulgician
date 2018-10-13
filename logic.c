#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EXPR_LEN_LIM    1024

enum op_type {
    OP_LBRACKET = -2,
    OP_RBRACKET = -1,
    OP_NOT = 0,
    OP_AND,
    OP_OR,
    OP_IMPLY,
    OP_MUTIMPLY,
    OP_COUNT,
    OP_VAR,
    OP_VAR_END = OP_VAR + 26,
    OP_INVALID = 0xff
};

typedef enum op_type *token_list;

struct expr_tree_node {
    enum op_type op;
    struct expr_tree_node *lch, *rch;
};

void ensure_sane(const char *subroutine, const char *input, int pos, const char *msg)
{
    if (pos != -1) {
        puts(input);
        int i;
        for (int i = 0; i < pos; ++i) putchar(' ');
        putchar('^');
        putchar('\n');
        printf("%s | %s\n", subroutine, msg);
    }
}

void dump_tokens(FILE *f, token_list list)
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

token_list tokenize(const char *s, int *pos, const char **msg)
{
    int sz = 0, cap = 6;
    token_list ret = (token_list)malloc(cap * sizeof ret[0]);
    if (ret == NULL) {
        *pos = 0;
        *msg = "Insufficient memory [ENOMEM]";
        free(ret);
        return NULL;
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
            free(ret);
            return NULL;
        }
        if (++sz == cap) {
            cap >>= 1;
            ret = (token_list)realloc(ret, cap * sizeof ret[0]);
            if (ret == NULL) {
                *pos = i;
                *msg = "Insufficient memory [ENOMEM]";
                free(ret);
                return NULL;
            }
        }
        ret[sz - 1] = cur_op;
    }
    if (sz == 0) {
        *pos = 0;
        *msg = "Empty expression";
        free(ret);
        return NULL;
    }
    ret[sz++] = OP_INVALID;
    *pos = -1;
    *msg = "Success";
    return ret;
}

token_list sfx_expr_build(token_list tokens)
{
}

struct expr_tree_node *expr_tree_build(token_list sfx_expr)
{
}

int main()
{
    char s[EXPR_LEN_LIM];
    fgets(s, sizeof s, stdin);
    int len = strlen(s);
    if (s[len - 1] == '\n') s[len - 1] = '\0';

    int err_pos;
    const char *err_msg;

    token_list tokens = tokenize(s, &err_pos, &err_msg);
    ensure_sane("Tokenizer", s, err_pos, err_msg);
    dump_tokens(stdout, tokens);

    token_list sfx_expr = sfx_expr_build(tokens);
    struct expr_tree_node *root = expr_tree_build(sfx_expr);

    return 0;
}
