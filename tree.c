
#include <unistd.h>
#include <stdio.h>
#include <libgen.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>


#include "./config.h"

#define EQU   0
#define FALSE 0
#define TRUE  1

#define DEFAULT_FLAG(flag) ((flag) & DEFAULT_FLAGS) ? "(default)" : ""
#define DEFAULT_FLAG_INVERTED(flag) ((flag) & DEFAULT_FLAGS) ? "" : "(default)"

typedef unsigned char bool;

// behaviour
Flag_Pole c_flags = DEFAULT_FLAGS;
// search
const char* c_path = ".";
unsigned char c_depth = DEFAULT_DEPTH;
unsigned char c_childs = DEFAULT_CHILDS;
// regex
typedef enum {
    IS = 1,
    EXCLUDE = 2,
    STARTS = 4,
    ENDS = 8,
    CONTAINS = 16
} Reg_Type;
typedef struct Regex_Node {
    Reg_Type type;
    unsigned int size;
    const char * str;
    struct Regex_Node * next;
} Regex_LL;
Regex_LL * c_regex = 0;


void init(int argc, const char *argv[]);
void tree_gen(void);
struct Regex_Node * regex_generator(const char * regex);

int main(int argc, const char *argv[]) {
    init(argc, argv);
    tree_gen();
    return 0;
}

void help(void) {
    printf("\033[1mNAME\033[0m\n");
    printf("        tree - show directory tree - %s\n", VERSION);
    printf("\033[1mSYNOPSIS\033[0m\n");
    printf("        tree [OPTIONS] [-y NUM] [-x NUM] [-p PATH] [-r REGEX]\n");
    printf("\033[1mDESCRIPTION\033[0m\n");
    printf("        try it\n");
    printf("\033[1mOPTIONS\033[0m\n");
    printf("    SEARCH:\n");
    printf("        -n, --not-all\n");
    printf("                dont show hidden directories %s\n",
                                        DEFAULT_FLAG_INVERTED(HIDDEN));
    printf("        -a, --all\n");
    printf("                show hidden directories %s\n",
                                        DEFAULT_FLAG(HIDDEN));
    printf("        -f, --files-too\n");
    printf("                show directories and files %s\n",
                                        DEFAULT_FLAG_INVERTED(DIRS));
    printf("        -d, --dir\n");
    printf("                only show directories %s\n",
                                        DEFAULT_FLAG(DIRS));
    printf("        -x, --depth NUM\n");
    printf("                set max depth (default: x=%u)\n",
                                        c_depth);
    printf("        -y, --entities NUM\n");
    printf("                set max number of childs (default: y=%u)\n",
                                        c_childs);
    printf("        -p, --path PATH\n");
    printf("                change path\n");
    printf("        -r, --regex \"REGEX\"\n");
    printf("                only sow files with this regex\n");
    printf("    SORTING:\n");
    printf("        -A, --ascending\n");
    printf("                sort ascending order %s\n",
                                        DEFAULT_FLAG(S_ASCENDING));
    printf("        -R, --descending\n");
    printf("                sort descending order %s\n",
                                        DEFAULT_FLAG_INVERTED(S_ASCENDING));
    printf("        -D, --directories-first\n");
    printf("                show directories first %s\n",
                                        DEFAULT_FLAG(S_DIRS_FIRST));
    printf("        -F, --files-first\n");
    printf("                show files before directories %s\n",
                                        DEFAULT_FLAG_INVERTED(S_DIRS_FIRST));
    printf("    EXTRA:\n");
    printf("        -s, --simple\n");
    printf("                dont show extra info about dir contents %s\n",
                                        DEFAULT_FLAG_INVERTED(EXTRA));
    printf("        -e, --extra\n");
    printf("                show extra info about dir contents %s\n",
                                        DEFAULT_FLAG(EXTRA));
    printf("        -m, --monocromatic\n");
    printf("                draw tree without colors %s\n",
                                        DEFAULT_FLAG_INVERTED(COLORS));
    printf("        -c, --colors\n");
    printf("                draw tree with colors %s\n",
                                        DEFAULT_FLAG(COLORS));
    printf("        -t, --text-only\n");
    printf("                dont draw the tree using icons (only ASCII) %s\n",
                                        DEFAULT_FLAG_INVERTED(ICONS));
    printf("        -i, --icons\n");
    printf("                draw the tree using icons %s\n",
                                        DEFAULT_FLAG(ICONS));
    printf("        -h, --help\n");
    printf("                help\n");
    printf("      Regex:\n");
    printf("        is: KEYWORD\n");
    printf("            e.g. tasks.txt; script.sh; main.py;\n");
    printf("        not: -KEYWORD\n");
    printf("            e.g. -*.txt; -script.sh; -*new*;\n");
    printf("        ends-with: *KEYWORD\n");
    printf("            e.g. *.java; *2024.txt; *_TODO;\n");
    printf("        starts-with: KEYWORD*\n");
    printf("            e.g. main*; 2024-01*; module0_*;\n");
    printf("        contains: *KEYWORD*\n");
    printf("            e.g. *main*; *work*; *log*;\n");
    printf("        scaping: \\*; \\-;\n");
    printf("            e.g. *pics*vids\\*; -\\*(1)*; \\-important*\n");
    printf("        multiple:\n");
    printf("            e.g. -r *.png -r *.jpg; -r md* -r -*.txt -r -md.pdf\n");
    printf("        rules:\n");
    printf("            - '-' always at the start of the string\n");
    printf("            - '*' always at the start and/or end of the string\n");
    printf("            - if '-' or '*' not at the start and/or end of the ");
    printf("              string, they count as normal characters\n");
}
unsigned char get_num(const char* str) {
    unsigned int num = 0;

    int i;
    for (i = 0; str[i] != '\0' && i < 3; i++) {
        if ( str[i] < '0' || '9' < str[i] ) {
            fprintf(stderr,
                    "tree: number argument has invalid characters '%s'\n",
                    str);
            exit(1);
        }
        num *= 10;
        num += str[i] - '0';
    }

    if ( i == 0 ) {
        fprintf(stderr, "tree: number argument is empty\n");
        exit(1);
    }
    if ( num <= 0 ) {
        fprintf(stderr, "tree: number must be greater thant 0\n");
        exit(1);
    }
    if ( num >= 256 ) {
        fprintf(stderr,
                "tree: number argument is to big '%s', must be less than 256\n",
                str);
        exit(1);
    }

    return (unsigned char)num;
}

void concatenated_flags(const char* str) {
    int i;
    for ( i = 1; str[i] != '\0'; i++ ) {
        switch (str[i]) {
            case 'n': c_flags &= ~HIDDEN;       break;
            case 'a': c_flags |= HIDDEN;        break;
            case 'f': c_flags &= ~DIRS;         break;
            case 'd': c_flags |= DIRS;          break;
            case 'A': c_flags |= S_ASCENDING;   break;
            case 'R': c_flags &= ~S_ASCENDING;  break;
            case 'D': c_flags |= S_DIRS_FIRST;  break;
            case 'F': c_flags &= ~S_DIRS_FIRST; break;
            case 's': c_flags &= ~EXTRA;        break;
            case 'e': c_flags |= EXTRA;         break;
            case 'm': c_flags &= ~COLORS;       break;
            case 'c': c_flags |= COLORS;        break;
            case 't': c_flags &= ~ICONS;        break;
            case 'i': c_flags |= ICONS;         break;
            default: {
                fprintf(stderr, "tree: unkown flag '%c' in '%s'\n",
                        str[i], str);
                exit(1);
            }
        }
    }
    if ( i == 1 ) {
        fprintf(stderr, "tree: unkown flag '%s'\n", str);
        exit(1);
    }
}

struct Regex_Node * regex_generator(const char * regex) {
    if ( regex[0] == '\0' ) return NULL;

    struct Regex_Node * node = calloc(1, sizeof(struct Regex_Node));
    if ( node == 0 ) {
        fprintf(stderr, "tree: error malloc, wtf\n");
        exit(1);
    }
    if ( regex[0] == '-' ) {
        node->type = EXCLUDE;
        regex++;
    }

    int size = strlen(regex);
    int k = 0;
    if ( size < 3 ) {
        for ( int i = 0; i < size && k < 2; i++ ) k += regex[i] == '*';
        if ( k == size ) return NULL;
    }

    if ( regex[0] == '*' ) {
        if ( regex[size-1] == '*' ) {
            bool escaped = FALSE;
            for ( int i = size-2; 0 <= i && regex[i] == '\\'; i-- ) {
                escaped = !escaped;
            }
            node->type |= (escaped) ? ENDS : CONTAINS;
            node->size = size - !escaped - 1;
            node->str = &regex[1];
        } else {
            node->type |= ENDS;
            node->size = size - 1;
            node->str = &regex[1];
        }
    } else if ( regex[size-1] == '*' ) {
        bool escaped = FALSE;
        for ( int i = size-2; 0 <= i && regex[i] == '\\'; i-- ) {
            escaped = !escaped;
        }
        if ( escaped ) {
            ((char *)regex)[size-1] = (char)'\0';
            ((char *)regex)[size-2] = (char)'*';
            size--;
        }
        node->type |= (escaped) ? IS : STARTS;
        node->size = size - !escaped;
        node->str = regex;
    } else {
        if ( regex[0] == '\\' ) {
            regex++;
            size--;
        }
        node->type |= IS;
        node->size = size;
        node->str = regex;
    }
    // printf("raw='%s' type=0x%02x; size=%d; start=%c; end=%c\n",
    //        node->str, node->type, node->size, node->str[0], node->str[node->size-1]);
    return ( (node->type & ~EXCLUDE) == 0) ? NULL : node;
}
bool regex_check(const char * name, Regex_LL * regex) {
    bool exclude, match;
    for ( struct Regex_Node * hook = regex; hook != 0; hook = hook->next ) {
        exclude = hook->type & EXCLUDE;
        switch (hook->type & ~EXCLUDE) {
            case IS: {
                if ( strlen(name) != hook->size ) return exclude;
                match = TRUE;
                for ( int i = 0; i < hook->size; i++ ) {
                    if ( name[i] != hook->str[i] ) {
                        match = FALSE;
                        break;
                    }
                }
                return (exclude) ? !match : match;
            }
            case STARTS: {
                if ( strlen(name) < hook->size ) return exclude;
                match = TRUE;
                for ( int i = 0; i < hook->size; i++ ) {
                    if ( name[i] != hook->str[i] ) {
                        match = FALSE;
                        break;
                    }
                }
                return (exclude) ? !match : match;
            }
            case ENDS: {
                unsigned int size = strlen(name);
                if ( size < hook->size ) return exclude;
                match = TRUE;
                const char * ptr0 = &name[size-1];
                const char * ptr1 = &hook->str[hook->size-1];
                for ( int i = 0; i < hook->size; i++ ) {
                    if ( *(ptr0--) != *(ptr1--) ) {
                        match = FALSE;
                        break;
                    }
                }
                return (exclude) ? !match : match;
            }
            case CONTAINS: {
                int size = strlen(name)+1;
                match = FALSE;
                for ( int j = 0; j < (signed int)(size-hook->size); j++ ) {
                    match = TRUE;
                    for ( int i = 0; i < hook->size; i++ ) {
                        if ( name[j+i] != hook->str[i] ) {
                            match = FALSE;
                            break;
                        }
                    }
                    if ( match ) break;
                }
                return (exclude) ? !match : match;
            }
        }
    }
    return TRUE;
}
void init(int argc, const char* argv[]) {
    for (int i = 1; i < argc; i++) {
        if ( argv[i][0] != '-' ) {
            if ( argv[i][0] == '\0' ) {
                fprintf(stderr, "tree: path is empty\n");
                exit(1);
            }
            c_path = (void*)argv[i];
        } else if ( strcmp("-h", argv[i]) == EQU ) {
            help(); exit(0);
        } else if ( strcmp("--help", argv[i]) == EQU ) {
            help(); exit(0);
        } else if ( strcmp("--not-all", argv[i]) == EQU ) {
            c_flags &= ~HIDDEN;
        } else if ( strcmp("--all", argv[i]) == EQU ) {
            c_flags |= HIDDEN;
        } else if ( strcmp("--files-too", argv[i]) == EQU ) {
            c_flags &= ~DIRS;
        } else if ( strcmp("--dir", argv[i]) == EQU ) {
            c_flags |= DIRS;
        } else if ( strcmp("--simple", argv[i]) == EQU ) {
            c_flags &= ~EXTRA;
        } else if ( strcmp("--extra", argv[i]) == EQU ) {
            c_flags |= EXTRA;
        } else if ( strcmp("--monocromatic", argv[i]) == EQU ) {
            c_flags &= ~COLORS;
        } else if ( strcmp("--colors", argv[i]) == EQU ) {
            c_flags |= COLORS;
        } else if ( strcmp("--text-only", argv[i]) == EQU ) {
            c_flags &= ~ICONS;
        } else if ( strcmp("--icon", argv[i]) == EQU ) {
            c_flags |= ICONS;
        } else if ( strcmp("--ascending", argv[i]) == EQU ) {
            c_flags |= S_ASCENDING;
        } else if ( strcmp("--descending", argv[i]) == EQU ) {
            c_flags &= ~S_ASCENDING;
        } else if ( strcmp("--directories-first", argv[i]) == EQU ) {
            c_flags |= S_DIRS_FIRST;
        } else if ( strcmp("--files-first", argv[i]) == EQU ) {
            c_flags &= ~S_DIRS_FIRST;
        } else if ( strcmp("-y", argv[i]) == EQU ||
                    strcmp("--entities", argv[i]) == EQU ) {
            if ( argc - i < 2 ) {
                fprintf(stderr, "tree: missing number argument after '%s'\n",
                        argv[i]);
                exit(1);
            }
            c_childs = get_num(argv[++i]);
        } else if ( strcmp("-x", argv[i]) == EQU ||
            strcmp("--depth", argv[i]) == EQU ) {
            if ( argc - i < 2 ) {
                fprintf(stderr, "tree: missing number argument after '%s'\n",
                        argv[i]);
                exit(1);
            }
            c_depth = get_num(argv[++i]);
        } else if ( strcmp("-p", argv[i]) == EQU ||
            strcmp("--path", argv[i]) == EQU ) {
            if ( argc - i < 2 ) {
                fprintf(stderr, "tree: missing path argument after '%s'",
                        argv[i]);
                exit(1);
            }
            if ( argv[++i][0] == '\0' ) {
                fprintf(stderr, "tree: path is empty\n");
                exit(1);
            }
            c_path = argv[i];
        } else if ( strcmp("-r", argv[i]) == EQU ||
            strcmp("--regex", argv[i]) == EQU ) {
            if ( argc - i < 2 ) {
                fprintf(stderr, "tree: missing regex argument after '%s'\n",
                        argv[i]);
                exit(1);
            }
            struct Regex_Node * hook = c_regex;
            c_regex = regex_generator(argv[++i]);
            if ( c_regex == 0 ) {
                fprintf(stderr, "tree: regex, not valid '%s'\n", argv[i]);
                exit(1);
            }
            c_regex->next = c_regex;
        } else if ( argv[i][1] != '-' ) {
            concatenated_flags(argv[i]);
        } else {
            fprintf(stderr, "tree: unkown flag '%s'\n", argv[i+1]);
            exit(1);
        }
    }
}

char * concat(const char *s0, const char *s1) {
    char * result = malloc(strlen(s0) + strlen(s1) + 1 + 1);

    if ( result == 0 ) {
        fprintf(stderr, "tree: concat: malloc failed\n");
        exit(1);
    }

    int i;
    int k = 0;
    for ( i = 0; s0[i] != '\0'; i++ ) {
        result[k++] = s0[i];
    }result[k++] = '/';
    for ( i = 0; s1[i] != '\0'; i++ ) {
        result[k++] = s1[i];
    }result[k] = '\0';

    return result;
}
typedef struct Tree_Leave {
    ino_t           inode;
    unsigned char   type;
    bool            executable;
    char            name[256];
} Leave;

typedef struct Tree_Branch {
    char *          path;
    unsigned int    len;
    unsigned int    size;
    const Leave *   leaves;
} Branch;

void tree_branch_init(Branch * branch, const char * path) {
    int len = strlen(path) + 1;
    branch->path = malloc(sizeof(char) * len);
    memcpy((void *)branch->path, path, len);
    branch->len = 0;
    branch->size = 16;
    branch->leaves = malloc(sizeof(Leave) * branch->size);

    if ( branch->leaves == 0 ) {
        fprintf(stderr, "tree: malloc failed, wtf");
        exit(1);
    }
}
void tree_branch_free(Branch * branch) {
    free(branch->path);
    free((void*)branch->leaves);
}
void tree_branch_add(Branch * branch, const struct dirent * ent) {
    if ( branch->len < branch->size ) {
        Leave * leave = (Leave *)&branch->leaves[branch->len++];
        leave->inode = ent->d_ino;
        memcpy(&leave->name, &ent->d_name, sizeof(char) * 256);
        leave->type = ent->d_type;

        if ( leave->type == DT_DIR ) return;

        char * full_path = concat(branch->path, leave->name);
        struct stat statbuf;

        if ( stat(full_path, &statbuf) == -1 ) {
            fprintf(stderr, "tree: error reading stat of '%s'",
                    full_path);
            exit(1);
        }
        leave->executable = statbuf.st_mode & (S_ISVTX|S_IXUSR|S_IXGRP|S_IXOTH);
        free(full_path);
        return;
    }

    branch->size *= 2;
    const Leave * hook = malloc(sizeof(Leave) * branch->size);
    memcpy((void*)hook, branch->leaves, sizeof(Leave) * branch->len);
    free((void*)branch->leaves);
    branch->leaves = hook;

    tree_branch_add(branch, ent);
}
bool tree_branch_sort_rule(const Leave * l0, const Leave * l1) {
    if ( l0->type == DT_DIR ) {
        if ( l1->type != DT_DIR ) {
            return !(c_flags & S_DIRS_FIRST);
        }
    } else if ( l1->type == DT_DIR ) {
        return c_flags & S_DIRS_FIRST;
    }

    if ( c_flags & S_ASCENDING ) {
        return strcmp(l0->name, l1->name) > 0;
    }
    return strcmp(l0->name, l1->name) < 0;
}
void tree_branch_sort(Branch * branch) {
    // basic bubble
    Leave swap;
    for ( int i = 0; i < branch->len; i++ ) {
        for ( int j = i+1; j < branch->len; j++ ) {
            if ( tree_branch_sort_rule(&branch->leaves[i], &branch->leaves[j]) ) {
                memcpy(&swap, &branch->leaves[i], sizeof(Leave));
                memcpy((void *)&branch->leaves[i], &branch->leaves[j], sizeof(Leave));
                memcpy((void *)&branch->leaves[j], &swap, sizeof(Leave));
            }
        }
    }
}
icon get_file_extension(const char * name) {
    for ( int i = 0; *file_icons[i][0] != '\0'; i++ ) {
        if ( strcmp(name, file_icons[i][0]) == EQU ) {
            return file_icons[i][0];
        }
    }

    const char * extension = NULL;
    for ( const char * ptr = name; *ptr != '\0'; ptr++ ) {
        if ( *ptr == '.' ) {
            extension = ptr;
        }
    }
    if ( extension == NULL ) {
        return 0;
    }
    for ( int i = 0; *file_extension_icons[i][0] != '\0'; i++ ) {
        if ( strcmp(extension, file_extension_icons[i][0]) == EQU ) {
            return file_extension_icons[i][0];
        }
    }
    return 0;
}
icon get_file_icon(const char * name) {
    if ( !(c_flags & ICONS) ) return "×";
    if ( name[0] == '.' ) return COOL_H_FILE_ICON;

    for ( int i = 0; *file_icons[i][0] != '\0'; i++ ) {
        if ( strcmp(name, file_icons[i][0]) == EQU ) {
            return file_icons[i][1];
        }
    }

    const char * extension = NULL;
    for ( const char * ptr = name; *ptr != '\0'; ptr++ ) {
        if ( *ptr == '.' ) {
            extension = ptr;
        }
    }
    if ( extension == NULL ) {
        return COOL_B_FILE_ICON;
    }
    for ( int i = 0; *file_extension_icons[i][0] != '\0'; i++ ) {
        if ( strcmp(extension, file_extension_icons[i][0]) == EQU ) {
            return file_extension_icons[i][1];
        }
    }
    return COOL_B_FILE_ICON;
}
icon get_directory_icon(const char * name) {
    if ( !(c_flags & ICONS) ) return "#";
    if ( name[0] == '.' ) return COOL_H_DIR_ICON;

    for ( int i = 0; *directory_icons[i][0] != '\0'; i++ ) {
        if ( strcmp(name, directory_icons[i][0]) == EQU ) {
            return directory_icons[i][1];
        }
    }
    return COOL_B_DIR_ICON;
}
void tree_branch_expand(char * path, unsigned int depth) {
    // LOAD Branch
    DIR* dir = opendir(path);
    if ( dir == NULL ) {
        fprintf(stderr, "tree: error opening dir '%s'",
                path);
        exit(1);
    }

    struct dirent *ent;
    Branch branch;
    Leave leave;

    tree_branch_init(&branch, path);

    while ( (ent = readdir(dir)) != NULL ) {
        if ( !(strcmp("..", ent->d_name) && strcmp(".", ent->d_name))   ||
             (!(c_flags & HIDDEN) && ent->d_name[0] == '.'))
            continue;
        tree_branch_add(&branch, ent);
    }
    if ( closedir(dir) == -1 ) {
        fprintf(stderr, "tree: error closing dir");
        exit(1);
    }
    // pinting stuff, don't think it too mutch
    printf("%s - ",
           (c_flags & COLORS) ? COLOR_INFO : "\033[0;2m");
    if ( branch.len ) {
        if ( (c_flags & EXTRA) ) {
            int n = 0;
            int m = 0;
            const char * arr[branch.len];
            const char ** hook;
            arr[n] = NULL;
            const char * tmp;
            const char * new;
            for ( int i = 0; i < branch.len; i++ ) {
                if ( branch.leaves[i].type == DT_DIR ) {
                    m++;
                    continue;
                }
                if ( c_flags & ICONS ) {
                    new = get_file_icon(branch.leaves[i].name);
                } else {
                    new = get_file_extension(branch.leaves[i].name);
                }
                for ( int j = 0; j < n; j++ ) {
                    if ( arr[j] == new ) {
                        new = NULL;
                        break;
                    }
                }
                if ( new != NULL ) {
                    arr[n++] = new;
                    arr[n] = NULL;
                    new = NULL;
                }
            }

            if ( !(branch.len-m) ) {
                printf("d:%d", m);
            } else {
                if ( m == 0 ) {
                    printf("f:%d", branch.len-m);
                } else {
                    printf("d:%d f:%d", m, branch.len-m);
                }
                if ( n != 0 ) {
                    printf(" (");
                    for ( int i = 0; i < n; i++ ) {
                        printf("%s", arr[i]);
                        if ( i+1 < n ) {
                            printf(",");
                        }
                    }printf(")");
                }
            }
        } else {
            printf("%d", branch.len);
        }
    } else {
        printf("%s",
               (c_flags & ICONS) ? COOL_NONE : CUTRE_NONE);
    }printf("\033[0m\n");

    if ( depth > c_depth || branch.len > c_childs )
        goto end_branch_after_free_branch;

    tree_branch_sort(&branch);

    char tabs[256];
    int j = 0;
    tabs[j++] = ' ';
    char * line;
    int s0;
    if ( c_flags & ICONS ) {
        line = COOL_LINE;
        s0 = sizeof(COOL_LINE)-1;
    } else {
        line = CUTRE_LINE;
        s0 = sizeof(CUTRE_LINE)-1;
    }
    const int s1 = sizeof("  ")-1;
    for ( int i = 0; i < depth; i++ ) {
        memcpy(&tabs[j], line, s0);
        j += s0;
        memcpy(&tabs[j], "  ", s1);
        j += s1;
    } tabs[j] = '\0';

    // print the branch ( ·_·)b
    for ( int i = 0; i < branch.len; i++ ) {
        if ( branch.leaves[i].type == DT_DIR ) {
            printf("\033[0;2m%s\033[%dm%s%s %s",
                   tabs,
                   (branch.leaves[i].name[0] == '.') * 2,
                   (c_flags & COLORS) ? COLOR_DIR : "",
                   get_directory_icon(branch.leaves[i].name),
                   branch.leaves[i].name);
            tree_branch_expand(concat(path, branch.leaves[i].name), depth+1);
        } else if ( !(c_flags & DIRS) ) {
            if ( c_regex && !regex_check(branch.leaves[i].name, c_regex) )
                continue;
            char * color = "";
            if ( c_flags & COLORS ) {
                if ( branch.leaves[i].executable ) {
                    color = COLOR_EXECUTABLE_FILE;
                } else if ( branch.leaves[i].type == DT_LNK ) {
                    color = COLOR_LINKED_FILE;
                } else {
                    color = COLOR_REGULAR_FILE;
                }
            }
            printf("\033[0;2m%s\033[%dm%s%s %s\033[0m\n",
                   tabs,
                   (branch.leaves[i].name[0] == '.') * 2,
                   color,
                   get_file_icon(branch.leaves[i].name),
                   branch.leaves[i].name);
        }
    }

end_branch_after_free_branch:
    tree_branch_free(&branch);
    if ( strcmp(c_path, path) )
        free(path);
}

void tree_gen(void) {
    char cwd[256];
    char * base_path;
    if ( getcwd(cwd, sizeof(cwd)) == NULL ) {
        fprintf(stderr, "tree: error reading cwd");
        exit(1);
    }
    if ( c_path[0] != '/' ) {
        if ( c_path[0] == '.' ) {
            int len = strlen(cwd);
            base_path = malloc(sizeof(char)*len+1);
            memcpy(base_path, cwd, len);
        } else {
            base_path = concat(cwd, c_path);
        }
    } else {
        int len = strlen(c_path);
        base_path = malloc(sizeof(char)*len+1);
        memcpy(base_path, c_path, len);
    }
    DIR* dir = opendir(base_path);
    if ( dir == NULL ) {
        fprintf(stderr, "tree: error opening dir '%s', it doesn't exist",
                c_path);
        exit(1);
    }
    // printf("%s\n", base_path);
    printf("%s%s",
           (c_flags & COLORS) ? COLOR_DIR : "",
           basename(base_path));
    tree_branch_expand(base_path, 0);
}

