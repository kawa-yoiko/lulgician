#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

// usage: ./test [<program> [<cases-file>]]

static const char *prog_default = "./a.out";
static const char *cases_file_default = "testcases.txt";

static void rtrim(char *s)
{
    char *t = s + strlen(s) - 1;
    while (t >= s && isspace(*t)) *(t--) = '\0';
}

/* `c` is an an FG/BG code */
static void change_colour(int c)
{
    printf("\x1B[%dm", c);
}

int main(int argc, char *argv[])
{
    change_colour(0);
    const char *prog;
    const char *cases_file;

    prog = (argc >= 2 ? argv[1] : prog_default);
    cases_file = (argc >= 3 ? argv[2] : cases_file_default);

    /* Temporary use only */
    FILE *fc = fopen(prog, "r");
    if (!fc) {
        printf("Cannot verify that the program [%s] exists > <\n", prog);
        return 1;
    }
    fclose(fc);
    /* Real stuff happens here */
    fc = fopen(cases_file, "r");
    if (!fc) {
        printf("Cannot open test cases file [%s] > <\n", cases_file);
        return 1;
    }

    char s[1024];
    _Bool has_expr = false;
    char expr[1024];

    char cmd[1024];
    while (!feof(fc)) {
        fgets(s, sizeof s, fc);
        rtrim(s);
        if (s[0] == '#') continue;
        if (!has_expr) {
            strcpy(expr, s);
        } else {
            /* Run a test */
            sprintf(cmd, "echo \"%s\" | %s d", expr, prog);
            FILE *pipe = popen(cmd, "r");
            /* Reuse buffer `cmd` */
            while (!feof(pipe)) fgets(cmd, sizeof cmd, pipe);
            pclose(pipe);
            rtrim(cmd);
            change_colour(34);
            printf("%s", expr);
            change_colour(0);
            printf(" - ");
            change_colour(36);
            printf("%s", cmd);
            change_colour(0);
            putchar('\n');
            if (strcmp(cmd, s) != 0) {
                change_colour(31);
                printf("!!! Expected ");
                change_colour(33);
                printf("%s", s);
                change_colour(31);
                printf(" !!!\n");
            } else {
                change_colour(32);
                printf("*** Correct ***\n");
            }
            change_colour(0);
            putchar('\n');
        }
        has_expr ^= 1;
    }

    fclose(fc);
    return 0;
}
