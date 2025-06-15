#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_FILES 128
#define BUFFER_SIZE 4096

typedef enum {
    NO_TIMESTAMP,
    ABSOLUTE_TIMESTAMP,
    RELATIVE_TIMESTAMP
} TimestampType;

typedef enum {
    TARGET_BOTH,
    TARGET_STDOUT,
    TARGET_FILES
} TimestampTarget;

typedef enum {
    COLOR_AUTO,
    COLOR_ALWAYS,
    COLOR_NEVER
} ColorMode;

int ignore_interrupts = 0;
int append_mode = 0;
TimestampType timestamp_type = NO_TIMESTAMP;
TimestampTarget timestamp_target = TARGET_BOTH;
ColorMode color_mode = COLOR_AUTO;

struct timespec program_start;

// Signal handler
void handle_signal(int sig) {
    if (ignore_interrupts) {
        // Do nothing
    } else {
        exit(1);
    }
}

// Get absolute timestamp
void get_absolute_timestamp(char *buffer, size_t size) {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(buffer, size, "[%Y-%m-%d %H:%M:%S] ", tm_info);
}

// Get relative timestamp
void get_relative_timestamp(char *buffer, size_t size) {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);

    double seconds = (now.tv_sec - program_start.tv_sec) +
                     (now.tv_nsec - program_start.tv_nsec) / 1e9;

    snprintf(buffer, size, "[+%08.3f] ", seconds);
}

bool is_stdout(FILE *fp) {
    return fp == stdout;
}

bool should_add_timestamp(FILE *fp) {
    if (timestamp_type == NO_TIMESTAMP)
        return false;
    if (timestamp_target == TARGET_BOTH)
        return true;
    if (timestamp_target == TARGET_STDOUT && is_stdout(fp))
        return true;
    if (timestamp_target == TARGET_FILES && !is_stdout(fp))
        return true;
    return false;
}

bool should_colorize(FILE *fp) {
    if (!is_stdout(fp))
        return false;

    switch (color_mode) {
        case COLOR_ALWAYS:
            return true;
        case COLOR_NEVER:
            return false;
        case COLOR_AUTO:
            return isatty(fileno(stdout));
    }
    return false;
}

void get_timestamp_for_output(char *buffer, size_t size, FILE *fp) {
    if (timestamp_type == NO_TIMESTAMP) {
        buffer[0] = '\0';
        return;
    }

    char ts[64];
    if (timestamp_type == ABSOLUTE_TIMESTAMP)
        get_absolute_timestamp(ts, sizeof(ts));
    else if (timestamp_type == RELATIVE_TIMESTAMP)
        get_relative_timestamp(ts, sizeof(ts));

    if (should_colorize(fp)) {
        snprintf(buffer, size, "\033[36m%s\033[0m", ts);  // Cyan
    } else {
        snprintf(buffer, size, "%s", ts);
    }
}

int main(int argc, char *argv[]) {
    FILE *outputs[MAX_FILES];
    int output_count = 0;
    int opt;
    int long_index = 0;

    clock_gettime(CLOCK_MONOTONIC, &program_start);

    struct option long_options[] = {
        {"append", no_argument, 0, 'a'},
        {"ignore-interrupts", no_argument, 0, 'i'},
        {"timestamp", required_argument, 0, 't'},
        {"timestamp-target", required_argument, 0, 0},
        {"color", optional_argument, 0, 0},
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "ait:", long_options, &long_index)) != -1) {
        if (opt == 0) {
            const char *opt_name = long_options[long_index].name;

            if (strcmp(opt_name, "timestamp-target") == 0) {
                if (strcmp(optarg, "stdout") == 0)
                    timestamp_target = TARGET_STDOUT;
                else if (strcmp(optarg, "files") == 0)
                    timestamp_target = TARGET_FILES;
                else if (strcmp(optarg, "both") == 0)
                    timestamp_target = TARGET_BOTH;
                else {
                    fprintf(stderr, "Invalid --timestamp-target value: %s\n", optarg);
                    exit(EXIT_FAILURE);
                }
            } else if (strcmp(opt_name, "color") == 0) {
                if (!optarg || strcmp(optarg, "auto") == 0)
                    color_mode = COLOR_AUTO;
                else if (strcmp(optarg, "always") == 0)
                    color_mode = COLOR_ALWAYS;
                else if (strcmp(optarg, "never") == 0)
                    color_mode = COLOR_NEVER;
                else {
                    fprintf(stderr, "Invalid --color value: %s\n", optarg);
                    exit(EXIT_FAILURE);
                }
            }

        } else {
            switch (opt) {
                case 'a':
                    append_mode = 1;
                    break;
                case 'i':
                    ignore_interrupts = 1;
                    break;
                case 't':
                    if (strcmp(optarg, "absolute") == 0)
                        timestamp_type = ABSOLUTE_TIMESTAMP;
                    else if (strcmp(optarg, "relative") == 0)
                        timestamp_type = RELATIVE_TIMESTAMP;
                    else {
                        fprintf(stderr, "Invalid --timestamp type: %s\n", optarg);
                        exit(EXIT_FAILURE);
                    }
                    break;
                default:
                    fprintf(stderr, "Usage: %s [-a] [-i] [--timestamp=absolute|relative] [--timestamp-target=...] [--color[=...]] [files...]\n", argv[0]);
                    exit(EXIT_FAILURE);
            }
        }
    }

    if (ignore_interrupts) {
        signal(SIGINT, handle_signal);
    }

    // Open output files
    for (int i = optind; i < argc; ++i) {
        FILE *fp = fopen(argv[i], append_mode ? "a" : "w");
        if (!fp) {
            perror(argv[i]);
            exit(EXIT_FAILURE);
        }
        outputs[output_count++] = fp;
    }

    outputs[output_count++] = stdout;

    // Read and write lines
    char line[BUFFER_SIZE];
    while (fgets(line, sizeof(line), stdin)) {
        for (int i = 0; i < output_count; ++i) {
            char ts[128];
            char output_line[BUFFER_SIZE + 128];

            if (should_add_timestamp(outputs[i])) {
                get_timestamp_for_output(ts, sizeof(ts), outputs[i]);
            } else {
                ts[0] = '\0';
            }

            snprintf(output_line, sizeof(output_line), "%s%s", ts, line);
            fputs(output_line, outputs[i]);
            fflush(outputs[i]);
        }
    }

    // Close file outputs
    for (int i = 0; i < output_count - 1; ++i) {
        fclose(outputs[i]);
    }

    return 0;
}
