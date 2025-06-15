#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>
#include <time.h>
#include <errno.h>

#define MAX_FILES 128
#define BUFFER_SIZE 4096

typedef enum {
    NO_TIMESTAMP,
    ABSOLUTE_TIMESTAMP,
    RELATIVE_TIMESTAMP
} TimestampType;

int ignore_interrupts = 0;
int append_mode = 0;
TimestampType timestamp_type = NO_TIMESTAMP;
struct timespec program_start;

// Signal handler
void handle_signal(int sig) {
    if (ignore_interrupts) {
        // Ignore SIGINT
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

// Get timestamp depending on type
void get_timestamp(char *buffer, size_t size) {
    if (timestamp_type == ABSOLUTE_TIMESTAMP) {
        get_absolute_timestamp(buffer, size);
    } else if (timestamp_type == RELATIVE_TIMESTAMP) {
        get_relative_timestamp(buffer, size);
    } else {
        buffer[0] = '\0';  // No timestamp
    }
}

int main(int argc, char *argv[]) {
    FILE *outputs[MAX_FILES];
    int output_count = 0;
    int opt;
    int long_index = 0;

    // Store program start time for relative timestamps
    clock_gettime(CLOCK_MONOTONIC, &program_start);

    struct option long_options[] = {
        {"append", no_argument, 0, 'a'},
        {"ignore-interrupts", no_argument, 0, 'i'},
        {"timestamp", required_argument, 0, 't'},
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "ait:", long_options, &long_index)) != -1) {
        switch (opt) {
            case 'a':
                append_mode = 1;
                break;
            case 'i':
                ignore_interrupts = 1;
                break;
            case 't':
                if (strcmp(optarg, "absolute") == 0) {
                    timestamp_type = ABSOLUTE_TIMESTAMP;
                } else if (strcmp(optarg, "relative") == 0) {
                    timestamp_type = RELATIVE_TIMESTAMP;
                } else {
                    fprintf(stderr, "Unknown timestamp type: %s\n", optarg);
                    exit(EXIT_FAILURE);
                }
                break;
            default:
                fprintf(stderr, "Usage: %s [-a] [-i] [--timestamp=absolute|relative] [file...]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (ignore_interrupts) {
        signal(SIGINT, handle_signal);
    }

    // Open files
    for (int i = optind; i < argc; ++i) {
        FILE *fp = fopen(argv[i], append_mode ? "a" : "w");
        if (!fp) {
            perror(argv[i]);
            exit(EXIT_FAILURE);
        }
        outputs[output_count++] = fp;
    }

    // Always write to stdout too
    outputs[output_count++] = stdout;

    // Read from stdin and write
    char line[BUFFER_SIZE];
    while (fgets(line, sizeof(line), stdin)) {
        char timestamp[64];
        char output_line[BUFFER_SIZE + 128];

        get_timestamp(timestamp, sizeof(timestamp));
        snprintf(output_line, sizeof(output_line), "%s%s", timestamp, line);

        for (int i = 0; i < output_count; ++i) {
            fputs(output_line, outputs[i]);
            fflush(outputs[i]);
        }
    }

    // Close files (except stdout)
    for (int i = 0; i < output_count - 1; ++i) {
        fclose(outputs[i]);
    }

    return 0;
}
