#ifndef CONFIG_H
#define CONFIG_H

#define VERSION "v1.0.0"

typedef enum {
    NONE = 0,
    HIDDEN = 1 << 1,
    DIRS = 1 << 2,
    S_DIRS_FIRST = 1 << 4,
    S_ASCENDING = 1 << 5,
    EXTRA = 1 << 8,
    ICONS = 1 << 9,
    COLORS = 1 << 10,
} Flag_Pole;

#define DEFAULT_FLAGS     (S_DIRS_FIRST | S_ASCENDING)
// #define DEFAULT_FLAGS     (S_DIRS_FIRST | S_ASCENDING | HIDDEN | ICONS | COLORS | EXTRA)

#define DEFAULT_DEPTH     4
#define DEFAULT_CHILDS    12

#define CUTRE_LINE          "|"
#define CUTRE_NONE          "Ø"
#define CUTRE_FILE_ICON     "×"
#define CUTRE_DIR_NONE      "#"
#define COOL_LINE           "│"
#define COOL_NONE           "󰟢"
#define COOL_B_FILE_ICON    "󰈔"    // uf0214
#define COOL_H_FILE_ICON    ""    // uea7b
#define COOL_B_DIR_ICON     ""    // uf07b
#define COOL_H_DIR_ICON     ""    // uf114

// https://en.wikipedia.org/wiki/ANSI_escape_code
#define COLOR_REGULAR_FILE      ""
#define COLOR_DIR               "\033[1;94m"
#define COLOR_INFO              "\033[0;2;96m"
#define COLOR_EXECUTABLE_FILE   "\033[92m"
#define COLOR_LINKED_FILE       "\033[96m"

typedef const char *const icon;
icon file_extension_icons[][2] = {
    {".c",         ""},    // ue61e
    {".cpp",       ""},    // ue61d
    {".h",         ""},    // uf0fd
    {".html",      ""},    // ueac4
    {".java",      ""},    // ue256
    {".jpeg",      "󰈟"},    // uf021f
    {".jpg",       "󰈟"},    // uf021f
    {".json",      ""},    // ue60b
    {".lua",       ""},    // ue620
    {".mp3",       "󰈣"},    // uf0223
    {".mp4",       "󰈫"},    // uf022b
    {".pdf",       ""},    // ueaeb
    {".png",       "󰈟"},    // uf021f
    {".py",        ""},    // ue606
    {".sh",        ""},    // ue795
    {".txt",       "󰈙"},
    {".wav",       "󰈣"},    // uf0223
    {".zip",       ""},    // uf1c6
    {"",            ""}     // always keep one of these
};
icon file_icons[][2] = {
    {".gitignore",  ""},   // ue702
    {"Makefile",    ""},   // ue673
    {"",            ""}     // always keep one of these
};
icon directory_icons[][2] = {
    {"",            ""}     // always keep one of these
};

#endif  // CONFIG_H
