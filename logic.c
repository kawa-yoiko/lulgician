#include <stdio.h>

#define EXPR_LEN_LIM    1024

enum op_type {
    OP_NOT = 0,
    OP_AND,
    OP_OR,
    OP_IMPLY,
    OP_MUTIMPLY,
    OP_VAR
};

struct expr_tree_node {
    enum op_type op;
    struct expr_tree_node *lch, *rch;
};

struct expr_tree_node *expr_tree_build(char *s)
{
}

int main()
{
    char s[EXPR_LEN_LIM];
    fgets(s, sizeof s, stdin);

    struct expr_tree_node *root = expr_tree_build(s);

    return 0;
}
