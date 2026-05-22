#include "../include/my_argp.h"

static char doc[] = "my own loader for isos project, load a shared lib and call its functions"
                    "\v"
                    "Examples:\n"
                    "./isos_loader foo bar ...";
static char args_doc[] = "\nFunction [FUNCTION...]";

static struct argp_option options[] =
    {
        {0}};

static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
    struct arguments *args = state->input;

    switch (key)
    {

    case ARGP_KEY_ARG:
        args->functions[args->fcount++] = arg;
        break;

    case ARGP_KEY_NO_ARGS:
        // Project requires at least one function name
        argp_usage(state);
        break;

    case ARGP_KEY_END:
        // double-check: we must have parsed >=1 positional.
        if (state->arg_num < 1)
        {

            argp_usage(state);
        }
        break;

    default:
        return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

static struct argp argp =
    {
        options, parse_opt, args_doc, doc, NULL, NULL, NULL};

// Public wrapper
void parsing(int argc, char **argv, struct arguments *args)
{
    argp_parse(&argp, argc, argv, 0, 0, args);
};
