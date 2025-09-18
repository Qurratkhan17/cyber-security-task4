//
//  main.c
//  task4
//
//  Created by Qurrat ULAIN KHAN on 19/09/25.
//

/*
 Simple terminal-based key logger for educational/testing purposes.
 - Only records keys while this program runs and the terminal window has focus.
 - Press ESC (ASCII 27) to exit cleanly.
 - Compile: gcc keylogger.c -o keylogger
 - Run: ./keylogger
*/

#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

static struct termios orig_term;

/* Restore original terminal settings on exit */
void restore_terminal(void) {
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_term);
}

/* Put terminal in raw mode so we can read characters immediately */
int set_raw_mode(void) {
    struct termios raw;
    if (tcgetattr(STDIN_FILENO, &orig_term) == -1) return -1;
    atexit(restore_terminal);

    raw = orig_term;
    raw.c_lflag &= ~(ECHO | ICANON); /* disable echo and canonical mode */
    raw.c_cc[VMIN] = 1;   /* read() returns after 1 byte */
    raw.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) == -1) return -1;
    return 0;
}

int main(void) {
    FILE *logf;

    if (set_raw_mode() == -1) {
        perror("set_raw_mode");
        return 1;
    }

    logf = fopen("keylog.txt", "a");
    if (!logf) {
        perror("fopen");
        return 1;
    }

    printf("Logging keys to keylog.txt. Press ESC to exit.\n");

    while (1) {
        unsigned char ch;
        ssize_t n = read(STDIN_FILENO, &ch, 1);
        if (n <= 0) break;

        /* If ESC pressed (ASCII 27), exit loop */
        if (ch == 27) {
            printf("\nESC detected — exiting.\n");
            break;
        }

        /* For readability, write printable characters as-is,
           and replace newline with explicit representation. */
        if (ch == '\r' || ch == '\n') {
            fputs("[ENTER]\n", logf);
            fflush(logf);
            putchar('\n');
        } else if (ch >= 32 && ch <= 126) {
            fputc(ch, logf);
            fflush(logf);
            putchar(ch); /* echo to terminal so user sees typing */
        } else {
            /* Non-printable / control keys: log hex code */
            fprintf(logf, "[0x%02X]", ch);
            fflush(logf);
            /* Optionally echo nothing or a placeholder */
        }
    }

    fclose(logf);
    /* term restored automatically via atexit */
    return 0;
}
