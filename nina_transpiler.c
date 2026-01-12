#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct {
    const char* input_file;
    const char* output_file;
} CompilerOptions;

CompilerOptions parse_args(int argc, char** argv);
void transpile_w_to_c(const char* input_file, const char* output_file);

int main(int argc, char** argv) {
    CompilerOptions opts = parse_args(argc, argv);
    transpile_w_to_c(opts.input_file, opts.output_file);
    return 0;
}

// ----------------------
// Argument Parser
// ----------------------
CompilerOptions parse_args(int argc, char** argv) {
    CompilerOptions opts = {0};

    if (argc < 2) {
        fprintf(stderr, "Usage: nina <file.w> [-o output.c]\n");
        exit(1);
    }

    opts.input_file = argv[1];
    opts.output_file = "out.c";

    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            opts.output_file = argv[++i];
        }
    }
    return opts;
}

// ----------------------
// Transpiler WALDBRAND -> C
// ----------------------
void transpile_w_to_c(const char* input_file, const char* output_file) {
    FILE* fin = fopen(input_file, "r");
    if (!fin) { perror("fopen input"); exit(1); }

    FILE* fout = fopen(output_file, "w");
    if (!fout) { perror("fopen output"); exit(1); }

    // Basis includes
    fprintf(fout, "#include <stdio.h>\n#include <stdlib.h>\n#include <stdbool.h>\n#include <string.h>\n\n");

    char line[1024];
    char current_object[256] = "";
    char method_object[256] = "";
    char method_name[256] = "";

    // main-Code sammeln
    char main_code[65536] = "";
    strcat(main_code, "int main() {\n");

    while (fgets(line, sizeof(line), fin)) {
        // Zeilenumbruch entfernen
        char* nl = strchr(line, '\n');
        if (nl) *nl = 0;

        // ------------------------
        // Module import
        // ------------------------
        if (strncmp(line, "#import ", 8) == 0) {
            char modname[256];
            if (sscanf(line, "#import %s", modname) == 1) {
                fprintf(fout, "#include \"%s.h\"\n", modname);
            }
        }
        // ------------------------
        // Object definition
        // ------------------------
        else if (strncmp(line, "object ", 7) == 0) {
            char objname[256];
            if (sscanf(line, "object %s {}", objname) == 1) {
                strcpy(current_object, objname);
                fprintf(fout, "typedef struct %s {\n    // Object Fields hier\n} %s;\n\n", objname, objname);
            }
        }
        // ------------------------
        // Object Method
        // ------------------------
        else if (strncmp(line, "function ", 9) == 0 && strstr(line, ".")) {
            if (sscanf(line, "function %[^.].%[^(\n]", method_object, method_name) == 2) {
                fprintf(fout, "void %s_%s(%s* this) {\n", method_object, method_name, method_object);
            }
        }
        // ------------------------
        // Normal function
        // ------------------------
        else if (strncmp(line, "function ", 9) == 0 && !strstr(line, ".")) {
            char fname[256];
            if (sscanf(line, "function %s()", fname) == 1) {
                fprintf(fout, "void %s() {\n", fname);
            }
        }
        // ------------------------
        // End Block
        // ------------------------
        else if (strcmp(line, "}") == 0) {
            fprintf(fout, "}\n");
        }
        // ------------------------
        // if / elif / else
        // ------------------------
        else if (strncmp(line, "if ", 3) == 0) {
            strcat(main_code, "    if (");
            strcat(main_code, line + 3);
            strcat(main_code, ") {\n");
        }
        else if (strncmp(line, "elif ", 5) == 0) {
            strcat(main_code, "    } else if (");
            strcat(main_code, line + 5);
            strcat(main_code, ") {\n");
        }
        else if (strncmp(line, "else", 4) == 0) {
            strcat(main_code, "    } else {\n");
        }
        // ------------------------
        // kill()
        // ------------------------
        else if (strstr(line, "kill()")) {
            strcat(main_code, "    exit(1);\n");
        }
        // ------------------------
        // print()
        // ------------------------
        else if (strncmp(line, "print(", 6) == 0) {
            strcat(main_code, "    printf(");
            // Entferne abschließendes ')', falls doppelt
            char* end = strrchr(line + 6, ')');
            if (end) *end = 0;
            strcat(main_code, line + 6);
            strcat(main_code, ");\n");
        }
        // ------------------------
        // input()
        // ------------------------
        else if (strstr(line, "input(")) {
            char varname[256], prompt[256];
            if (sscanf(line, "str %s = input(\"%[^\"]\")", varname, prompt) == 2) {
                char tmp[512];
                snprintf(tmp, sizeof(tmp), "    char %s[256]; printf(\"%s\"); fgets(%s, 256, stdin);\n", varname, prompt, varname);
                strcat(main_code, tmp);
            }
        }
        // ------------------------
        // Variablen
        // ------------------------
        else if (strncmp(line, "int ", 4) == 0) {
            char tmp[512];
            snprintf(tmp, sizeof(tmp), "    int %s;\n", line + 4);
            strcat(main_code, tmp);
        }
        else if (strncmp(line, "str ", 4) == 0) {
            char tmp[512];
            snprintf(tmp, sizeof(tmp), "    char %s[256];\n", line + 4);
            strcat(main_code, tmp);
        }
        else if (strncmp(line, "bool ", 5) == 0) {
            char tmp[512];
            snprintf(tmp, sizeof(tmp), "    bool %s;\n", line + 5);
            strcat(main_code, tmp);
        }
        // ------------------------
        // ptr
        // ------------------------
        else if (strncmp(line, "ptr ", 4) == 0) {
            char ptrname[256], adrname[256];
            if (sscanf(line, "ptr %s = adr(%[^)])", ptrname, adrname) == 2) {
                // FIX: keine extra Klammer mehr
                char tmp[256];
                snprintf(tmp, sizeof(tmp), "    void* %s = &%s;\n", ptrname, adrname);
                strcat(main_code, tmp);
            }
        }
        // ------------------------
        // Inline ASM
        // ------------------------
        else if (strncmp(line, "asm ", 4) == 0) {
            char tmp[256];
            snprintf(tmp, sizeof(tmp), "    __asm__(\"%s\");\n", line + 4);
            strcat(main_code, tmp);
        }
        // ------------------------
        // Sonstige Zeilen
        // ------------------------
        else {
            char tmp[512];
            snprintf(tmp, sizeof(tmp), "    // TODO: %s\n", line);
            strcat(main_code, tmp);
        }
    }

    // main-Code ans Ende schreiben
    strcat(main_code, "    return 0;\n}\n");
    fputs(main_code, fout);

    fclose(fin);
    fclose(fout);

    printf("Transpilation finished: %s -> %s\n", input_file, output_file);
}
