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

int ignore_interrupts = 0;
int append_mode = 0;
int timestamp_lines = 0;

// Signal handler to ignore SIGINT if needed
void handle_signal(int sig) {
    if (ignore_interrupts) {
        // Just ignore
    } else {
        exit(1);
    }
}

// Get current timestamp as string
void get_timestamp(char *buffer, size_t size) {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(buffer, size, "[%Y-%m-%d %H:%M:%S] ", tm_info);
}

int main(int argc, char *argv[]) {
    FILE *outputs[MAX_FILES];
    int output_count = 0;
    int opt;
    int long_index = 0;

    struct option long_options[] = {
        {"append", no_argument, 0, 'a'},
        {"ignore-interrupts", no_argument, 0, 'i'},
        {"timestamp", no_argument, 0, 't'},
        {0, 0, 0, 0}
    };

    // Parse command-line options
    while ((opt = getopt_long(argc, argv, "ait", long_options, &long_index)) != -1) {
        switch (opt) {
            case 'a':
                append_mode = 1;
                break;
            case 'i':
                ignore_interrupts = 1;
                break;
            case 't':
                timestamp_lines = 1;
                break;
            default:
                fprintf(stderr, "Usage: %s [-a] [-i] [-t] [file...]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    // Set signal handler if -i is used
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

    // Always include stdout
    outputs[output_count++] = stdout;

    // Read from stdin line by line
    char line[BUFFER_SIZE];
    while (fgets(line, sizeof(line), stdin)) {
        char output_line[BUFFER_SIZE + 64];
        if (timestamp_lines) {
            char timestamp[64];
            get_timestamp(timestamp, sizeof(timestamp));
            snprintf(output_line, sizeof(output_line), "%s%s", timestamp, line);
        } else {
            strncpy(output_line, line, sizeof(output_line));
        }

        // Write to all outputs
        for (int i = 0; i < output_count; ++i) {
            fputs(output_line, outputs[i]);
            fflush(outputs[i]);
        }
    }

    // Close all file outputs except stdout
    for (int i = 0; i < output_count - 1; ++i) {
        fclose(outputs[i]);
    }

    return 0;
}
