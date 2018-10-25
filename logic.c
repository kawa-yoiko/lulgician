#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EXPR_LEN_LIM    1024
#define highbit(__x) (1 << (8 * sizeof (__x) - __builtin_clz(__x) - 1))

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

/*
 * Ensures that the error info returned implies no error,
 * or exits the program with code 1 otherwise.
 */
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

/*
 * For the sake of simplicity, this function assumes
 * that there is sufficient memory allocated at `s`.
 * 4 times the input length will always work, though
 * a tighter lower bound exists.
 * Returns the actual displayed width in characters.
 */
int dump_tokens(char *s, enum op_type *list)
{
    int w = 0;
    for (; *list != OP_INVALID; ++list) switch (*list) {
        case OP_LBRACKET: *(s++) = '('; ++w; break;
        case OP_RBRACKET: *(s++) = ')'; ++w; break;
        case OP_NOT: s = stpcpy(s, "¬"); ++w; break;
        case OP_AND: s = stpcpy(s, " ∧ "); w += 3; break;
        case OP_OR: s = stpcpy(s, " ∨ "); w += 3; break;
        case OP_IMPLY: s = stpcpy(s, " → "); w += 3; break;
        case OP_MUTIMPLY: s = stpcpy(s, " ↔ "); w += 3; break;
        default:
            if (*list >= OP_VAR && *list < OP_VAR_END) {
                *(s++) = 'A' + (*list - OP_VAR);
            } else {
                *(s++) = '?';
            }
            ++w;
    }
    return w;
}

/*
 * Evaluates an expression tree with variables'
 * respective values stored in the bit vector `vars`.
 * Uses short-circuit evaluation.
 */
_Bool expr_tree_eval(struct expr_tree_node *u, int vars)
{
    switch (u->op) {
    case OP_NOT:
        return !expr_tree_eval(u->lch, vars);
    case OP_AND:
        return expr_tree_eval(u->lch, vars) && expr_tree_eval(u->rch, vars);
    case OP_OR:
        return expr_tree_eval(u->lch, vars) || expr_tree_eval(u->rch, vars);
    case OP_IMPLY:
        return !expr_tree_eval(u->lch, vars) || expr_tree_eval(u->rch, vars);
    case OP_MUTIMPLY:
        return expr_tree_eval(u->lch, vars) == expr_tree_eval(u->rch, vars);
    default:
        if (u->op >= OP_VAR && u->op < OP_VAR_END) {
            return (vars >> (25 - (u->op - OP_VAR))) & 1;
        } else {
            return 0;
        }
    }
}

/*
 * Parses an expression and generate related information.
 * `tokens`: a list of tokens used for further display.
 * `tree_root`: a pointer to the root of the expression tree.
 * `var_mask`: a bit vector indicating
 *             whether each letter occurs in the expression.
 * In case of errors, `pos` and `msg` are populated
 * with the position of error in the expression and an
 * error message, respectively.
 */
void expr_parse(const char *s,
    int *pos, const char **msg,
    enum op_type **tokens, struct expr_tree_node **tree_root, int *var_mask)
{
    /* Maybe we shall assert? */
    if (!s || !pos || !msg || !tokens || !tree_root) return;

    int expr_len = strlen(s);
    /* The list of tokens */
    enum op_type *tok = (enum op_type *)malloc((expr_len + 1) * sizeof tok[0]);
    /* The operator stack, and their positions in the original expression */
    enum op_type *stk = (enum op_type *)malloc(expr_len * sizeof tok[0]);
    int *stk_pos = (int *)malloc(expr_len * sizeof stk_pos[0]);
    /* The tree node stack, and their positions in the original expression */
    struct expr_tree_node **sfx =
        (struct expr_tree_node **)malloc(expr_len * sizeof sfx[0]);
    int *sfx_pos = (int *)malloc(expr_len * sizeof sfx_pos[0]);
    if (tok == NULL || stk == NULL || sfx == NULL) {
        *pos = 0;
        *msg = "Insufficient memory [ENOMEM]";
        free(tok); free(stk); free(sfx);
        return;
    }

    int tok_sz = 0, stk_top = 0, sfx_top = 0, vmask = 0;
    int i;
    _Bool following_var = 0, following_lbracket = 0;

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
            free(stk_pos); free(sfx_pos);
            return;
        }
        tok[tok_sz++] = cur_op;

        /* XXX: Is there a way to check these in the tree building process? */
        if (following_var && cur_op == OP_LBRACKET) {
            *pos = i;
            *msg = "Unexpected opening bracket";
            free(tok); free(stk); free(sfx);
            free(stk_pos); free(sfx_pos);
            return;
        }
        if (following_lbracket && cur_op == OP_RBRACKET) {
            *pos = i;
            *msg = "Isolated bracket pair";
            free(tok); free(stk); free(sfx);
            free(stk_pos); free(sfx_pos);
            return;
        }
        following_var = (cur_op >= OP_VAR && cur_op < OP_VAR_END);
        following_lbracket = (cur_op == OP_LBRACKET);

        /* Manipulate the stack */
        _Bool is_var = (cur_op >= OP_VAR && cur_op < OP_VAR_END);
        _Bool is_bracket = (cur_op == OP_LBRACKET || cur_op == OP_RBRACKET);
        _Bool is_rightassoc = (cur_op == OP_NOT);

        /* (1) Pop */
        if (!is_var && cur_op != OP_LBRACKET) {
            /* < for right associative operators; <= for left */
            while (stk_top > 0 && stk[stk_top - 1] < cur_op + !is_rightassoc) {
                enum op_type last_op = stk[--stk_top];
                int last_op_pos = stk_pos[stk_top];
                /* This happens only at the very last round of popping */
                if (last_op == OP_LBRACKET && cur_op != OP_RBRACKET) {
                    *pos = last_op_pos;
                    *msg = "Unexpected or unbalanced bracket";
                    free(tok); free(stk); free(sfx);
                    return;
                }
                int arity =
                    (last_op == OP_LBRACKET || last_op == OP_RBRACKET ? 0 :
                    (last_op == OP_NOT ? 1 : 2));
                /* Pop `arity` subtrees and build a new tree node */
                if (sfx_top < arity) {
                    *pos = last_op_pos;
                    *msg = "Missing operand";
                    free(tok); free(stk); free(sfx);
                    free(stk_pos); free(sfx_pos);
                    return;
                }
                struct expr_tree_node *u =
                    expr_tree_node_create(last_op, NULL, NULL);
                if (arity >= 2) u->rch = sfx[--sfx_top];
                if (arity >= 1) u->lch = sfx[--sfx_top];
                sfx[sfx_top] = u;
                sfx_pos[sfx_top++] = last_op_pos;
            }
        }
        /* Closing bracket needs another pop for the opening one */
        if (cur_op == OP_RBRACKET) {
            if (stk_top < 1 || stk[stk_top - 1] != OP_LBRACKET) {
                *pos = i;
                *msg = "Unbalanced bracket";
                free(tok); free(stk); free(sfx);
                free(stk_pos); free(sfx_pos);
                return;
            }
            --stk_top;
        }

        /* (2) Push when necessary */
        if ((cur_op >= OP_NOT && cur_op < OP_COUNT) || cur_op == OP_LBRACKET) {
            stk[stk_top] = cur_op;
            stk_pos[stk_top++] = i;
        } else if (is_var) {
            sfx[sfx_top] = expr_tree_node_create(cur_op, NULL, NULL);
            sfx_pos[sfx_top++] = i;
            vmask |= (1 << (25 - cur_op + OP_VAR));
        }

        if (s[i] == '\0') break;
    }

    if (sfx_top == 0) {
        *pos = expr_len;
        *msg = "Empty expression";
        free(tok); free(stk); free(sfx);
        free(stk_pos); free(sfx_pos);
        return;
    }
    /* XXX: Should this be moved to the tokenization stage? */
    if (sfx_top > 1) {
        *pos = sfx_pos[1];
        *msg = "Redundant variable occurrence";
        free(tok); free(stk); free(sfx);
        free(stk_pos); free(sfx_pos);
        return;
    }
    *pos = -1;
    *msg = "Success";
    *tokens = tok;
    *tree_root = sfx[0];
    *var_mask = vmask;
    free(stk_pos); free(sfx_pos);
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
    int var_mask;

    expr_parse(s, &err_pos, &err_msg, &tokens, &root, &var_mask);
    ensure_sane(s, err_pos, err_msg);

    char *expr_str = (char *)malloc(len * 4);
    if (expr_str == NULL) {
        fputs("Insufficient memory [ENOMEM]\n", stderr);
        return 1;
    }
    int expr_str_len = dump_tokens(expr_str, tokens);
    int rspace = (expr_str_len - 1) >> 1,
        lspace = (expr_str_len - 1) - rspace;

    /* Special ones: T and F */
    int special = (1 << (25 - 'T' + 'A')) | (1 << (25 - 'F' + 'A'));
    var_mask &= ~special;
    /* For later use in variable assignments */
    special = (1 << (25 - 'T' + 'A'));

    int num_rows = 1 << __builtin_popcount(var_mask);
    _Bool *results = (_Bool *)malloc(num_rows * sizeof results[0]);
    if (results == NULL) {
        fputs("Insufficient memory [ENOMEM]\n", stderr);
        return 1;
    }

    int i, j, vars;
    for (i = 0; i < 26; ++i)
        if (var_mask & (1 << (25 - i))) printf("| %c ", 'A' + i);
    printf("| %s |\n", expr_str);
    /* Iterate through all subsets of `var_mask` */
    for (i = 0, vars = var_mask; ; ++i, vars = (vars - 1) & var_mask) {
        /* As we're iterating `vars` from largest to smallest,
         * `var_mask ^ vars` goes from smallest to largest */
        _Bool result = expr_tree_eval(root, (var_mask ^ vars) | special);
        for (j = var_mask; j > 0; j -= highbit(j))
            printf("| %c ", !(vars & highbit(j)) ? 'T' : 'F');
        putchar('|');
        for (j = 0; j <= lspace; ++j) putchar(' ');
        putchar(result ? 'T' : 'F');
        for (j = 0; j <= rspace; ++j) putchar(' ');
        printf("|\n");

        results[i] = result;
        if (vars == 0) break;
    }

    _Bool first;
    printf("CNF: ∧_{");
    for (i = 0, first = 1; i < num_rows; ++i) if (!results[(num_rows - 1) ^ i]) {
        if (first) first = 0; else printf(", ");
        printf("%d", i);
    }
    printf("}\nDNF: ∨_{");
    for (i = 0, first = 1; i < num_rows; ++i) if (results[i]) {
        if (first) first = 0; else printf(", ");
        printf("%d", i);
    }
    printf("}\n");

    return 0;
}
