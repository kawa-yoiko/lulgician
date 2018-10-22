#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EXPR_LEN_LIM    1024

enum op_type {
    OP_NOT = 0,
    OP_AND,
    OP_OR,
    OP_IMPLY,
    OP_MUTIMPLY,
    OP_COUNT,
    OP_RBRACKET,
    OP_LBRACKET,
    OP_VAR,
    OP_VAR_END = OP_VAR + 26,
    OP_INVALID = 0xff
};

struct expr_tree_node {
    enum op_type op;
    struct expr_tree_node *lch, *rch;
};

struct expr_tree_node *expr_tree_node_create(
    enum op_type op,
    struct expr_tree_node *lch,
    struct expr_tree_node *rch)
{
    struct expr_tree_node *ret = (struct expr_tree_node *)malloc(sizeof *ret);
    *ret = (struct expr_tree_node){op, lch, rch};
    return ret;
}

void ensure_sane(const char *input, int pos, const char *msg)
{
    if (pos != -1) {
        puts(input);
        int i;
        for (int i = 0; i < pos; ++i) putchar(' ');
        putchar('^');
        putchar('\n');
        printf("%s\n", msg);
        exit(1);
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

void dump_expr_tree(FILE *f, struct expr_tree_node *root)
{
    if (root == NULL) return;
    fputc('[', f);
    dump_expr_tree(f, root->lch);
    fputc(']', f);
    fputc('[', f);
    dump_expr_tree(f, root->rch);
    fputc(']', f);
    printf(" %d", root->op);
}

void expr_parse(const char *s,
    int *pos, const char **msg,
    enum op_type **tokens, struct expr_tree_node **tree_root)
{
    /* Maybe we shall assert? */
    if (!s || !pos || !msg || !tokens || !tree_root) return;

    int expr_len = strlen(s);
    /* The list of tokens */
    enum op_type *tok = (enum op_type *)malloc((expr_len + 1) * sizeof tok[0]);
    /* The operator stack */
    enum op_type *stk = (enum op_type *)malloc(expr_len * sizeof tok[0]);
    /* The tree node stack */
    struct expr_tree_node **sfx =
        (struct expr_tree_node **)malloc(expr_len * sizeof sfx[0]);
    if (tok == NULL || stk == NULL || sfx == NULL) {
        *pos = 0;
        *msg = "Insufficient memory [ENOMEM]";
        free(tok); free(stk); free(sfx);
        return;
    }

    int tok_sz = 0, stk_top = 0, sfx_top = 0;
    int i;

    for (i = 0; ; ++i) if (s[i] == '\0' || !isspace(s[i])) {
        /* Parse current token */
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
        if (s[i] != '\0' && cur_op == OP_INVALID) {
            *pos = i;
            *msg = "Invalid character";
            free(tok); free(stk); free(sfx);
            return;
        }
        tok[tok_sz++] = cur_op;

        /* Manipulate the stack */
        _Bool is_var = (cur_op >= OP_VAR && cur_op < OP_VAR_END);
        _Bool is_bracket = (cur_op == OP_LBRACKET || cur_op == OP_RBRACKET);
        _Bool is_rightassoc = (cur_op == OP_NOT);

        /* (1) Pop */
        if (!is_var && cur_op != OP_LBRACKET) {
            while (stk_top > 0 && stk[stk_top - 1] < cur_op + !is_rightassoc) {
                enum op_type last_op = stk[--stk_top];
                int arity =
                    (last_op == OP_LBRACKET || last_op == OP_RBRACKET ? 0 :
                    (last_op == OP_NOT ? 1 : 2));
                if (sfx_top < arity) {
                    *pos = i;
                    *msg = "Missing operand";
                    free(tok); free(stk); free(sfx);
                    return;
                }
                struct expr_tree_node *u =
                    expr_tree_node_create(last_op, NULL, NULL);
                if (arity >= 2) u->rch = sfx[--sfx_top];
                if (arity >= 1) u->lch = sfx[--sfx_top];
                sfx[sfx_top++] = u;
            }
        }
        if (cur_op == OP_RBRACKET) {
            if (stk_top < 1 || stk[stk_top - 1] != OP_LBRACKET) {
                *pos = i;
                *msg = "Mismatching brackets";
                free(tok); free(stk); free(sfx);
                return;
            }
            --stk_top;
        }

        /* (2) Push when necessary */
        if ((cur_op >= OP_NOT && cur_op < OP_COUNT) || cur_op == OP_LBRACKET) {
            stk[stk_top++] = cur_op;
        } else if (is_var) {
            sfx[sfx_top++] = expr_tree_node_create(cur_op, NULL, NULL);
        }

        if (s[i] == '\0') break;
    }
    dump_expr_tree(stdout, sfx[0]); putchar('\n');
    if (tok_sz == 1) {
        /* Exit if OP_INVALID is the only token */
        *pos = 0;
        *msg = "Empty expression";
        free(tok); free(stk); free(sfx);
        return;
    }
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
    enum op_type *tokens;
    struct expr_tree_node *root;
    expr_parse(s, &err_pos, &err_msg, &tokens, &root);
    ensure_sane(s, err_pos, err_msg);
    dump_tokens(stdout, tokens);

    return 0;
}
