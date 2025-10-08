//
// Created by Will Ballantine on 10/8/25.
//

#include <blobit/blobit.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef int (*cmd_func_t)(int argc, char **argv);

typedef struct {
    const char* name;
    int min_args; // including command name
    int max_args; // including command name
    cmd_func_t func;
    const char* help;
} command_t;


static void print_usage(const char *prog) {
    printf("Usage: %s <command> <file> [outfile]\n", prog);
    printf("Commands:\n");
    printf("  print <file>           Pretty-print a blobit file\n");
    printf("  dump <file>            Dump raw bytes of a blobit file in hex\n");
    printf("  check <file>           Validate a blobit file\n");
    printf("  deserialize <in> <out> Deserialize a blobit file and pretty-print to a text file\n");
    printf("  help                   Show this help message\n");
}

static int cmd_print(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: print <file>\n");
        return 1;
    }
    const char *filename = argv[1];
    blobit_value_t *val = NULL;
    int res = blobit_deserialize_from_file(filename, &val);
    if (res != BLOBIT_SUCCESS) {
        fprintf(stderr, "Failed to deserialize '%s': %s\n", filename, blobit_error_message(res));
        return 1;
    }
    blobit_value_print(val, stdout);
    printf("\n");
    blobit_value_destroy(val);
    return 0;
}

static int cmd_dump(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: dump <file>\n");
        return 1;
    }
    const char *filename = argv[1];
    FILE *f = fopen(filename, "rb");
    if (!f) {
        fprintf(stderr, "Failed to open '%s'\n", filename);
        return 1;
    }
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (sz < 0) {
        fclose(f);
        fprintf(stderr, "Failed to get file size\n");
        return 1;
    }
    uint8_t *buf = malloc(sz);
    if (!buf) {
        fclose(f);
        fprintf(stderr, "Out of memory\n");
        return 1;
    }
    size_t n = fread(buf, 1, sz, f);
    fclose(f);
    if (n != (size_t)sz) {
        free(buf);
        fprintf(stderr, "Failed to read file\n");
        return 1;
    }
    for (size_t i = 0; i < n; ++i) {
        printf("%02X%s", buf[i], ((i+1)%16==0)?"\n":" ");
    }
    if (n % 16) printf("\n");
    free(buf);
    return 0;
}

static int cmd_check(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: check <file>\n");
        return 1;
    }
    const char *filename = argv[1];
    blobit_value_t *val = NULL;
    int res = blobit_deserialize_from_file(filename, &val);
    if (res == BLOBIT_SUCCESS) {
        printf("%s: OK\n", filename);
        blobit_value_destroy(val);
        return 0;
    }
    printf("%s: INVALID (%s)\n", filename, blobit_error_message(res));
    return 1;
}

static int cmd_deserialize(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: deserialize <infile> <outfile>\n");
        return 1;
    }
    const char *infile = argv[1];
    const char *outfile = argv[2];
    blobit_value_t *val = NULL;
    int res = blobit_deserialize_from_file(infile, &val);
    if (res != BLOBIT_SUCCESS) {
        fprintf(stderr, "Failed to deserialize '%s': %s\n", infile, blobit_error_message(res));
        return 1;
    }
    FILE *f = fopen(outfile, "w");
    if (!f) {
        fprintf(stderr, "Failed to open '%s' for writing\n", outfile);
        blobit_value_destroy(val);
        return 1;
    }
    blobit_value_print(val, f);
    fprintf(f, "\n");
    fclose(f);
    blobit_value_destroy(val);
    printf("Deserialized and pretty-printed to '%s'\n", outfile);
    return 0;
}

static int cmd_help(int argc, char **argv) {
    (void)argv;
    print_usage("blobit");
    return 0;
}

static command_t commands[] = {
    {"print", 2, 2, cmd_print, "Pretty-print a blobit file"},
    {"dump", 2, 2, cmd_dump, "Dump raw bytes of a blobit file in hex"},
    {"check", 2, 2, cmd_check, "Validate a blobit file"},
    {"deserialize", 3, 3, cmd_deserialize, "Deserialize a blobit file and pretty-print to a text file"},
    {"help", 1, 2, cmd_help, "Show this help message"},
};

int main(int argc, char *argv[])
{
    if (argc < 2) {
        print_usage(argv[0]);
        return 0;
    }
    for (size_t i = 0; i < sizeof(commands)/sizeof(commands[0]); i++) {
        if (strcmp(argv[1], commands[i].name) == 0 && argc-1 >= commands[i].min_args && argc-1 <= commands[i].max_args) {
            return commands[i].func(argc-1, argv+1);
        }
    }

    fprintf(stderr, "Unknown command or wrong number of arguments: %s\n", argv[1]);
    print_usage(argv[0]);
    return 1;
}