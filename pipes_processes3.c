#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

/**
 * Executes the pipeline: cat scores | grep <arg> | sort
 * 
 * Process Hierarchy:
 *   P1 (Parent):         cat scores      -> writes to pipe1
 *   P2 (Child):          grep <arg>      -> reads from pipe1, writes to pipe2
 *   P3 (Child's Child):  sort            -> reads from pipe2, writes to stdout
 */

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <search_term>\n", argv[0]);
        exit(1);
    }

    int pipe1[2]; // Pipe between cat (P1) and grep (P2)
    int pipe2[2]; // Pipe between grep (P2) and sort (P3)

    if (pipe(pipe1) == -1) {
        perror("pipe1 failed");
        exit(1);
    }

    pid_t pid1 = fork();

    if (pid1 < 0) {
        perror("fork 1 failed");
        exit(1);
    }

    if (pid1 == 0) {
        // --- CHILD (P2: grep) ---
        if (pipe(pipe2) == -1) {
            perror("pipe2 failed");
            exit(1);
        }

        pid_t pid2 = fork();

        if (pid2 < 0) {
            perror("fork 2 failed");
            exit(1);
        }

        if (pid2 == 0) {
            // --- CHILD'S CHILD (P3: sort) ---
            // Read input from pipe2
            dup2(pipe2[0], STDIN_FILENO);

            // Close unused ends
            close(pipe1[0]);
            close(pipe1[1]);
            close(pipe2[0]);
            close(pipe2[1]);

            char *sort_args[] = {"sort", NULL};
            execvp("sort", sort_args);
            perror("execvp sort failed");
            exit(1);
        } else {
            // --- BACK IN P2: grep ---
            // Input comes from pipe1
            dup2(pipe1[0], STDIN_FILENO);
            // Output goes to pipe2
            dup2(pipe2[1], STDOUT_FILENO);

            // Close all pipe ends in this process
            close(pipe1[0]);
            close(pipe1[1]);
            close(pipe2[0]);
            close(pipe2[1]);

            char *grep_args[] = {"grep", argv[1], NULL};
            execvp("grep", grep_args);
            perror("execvp grep failed");
            exit(1);
        }
    } else {
        // --- PARENT (P1: cat scores) ---
        // Output goes to pipe1
        dup2(pipe1[1], STDOUT_FILENO);

        // Close unused pipe ends
        close(pipe1[0]);
        close(pipe1[1]);

        char *cat_args[] = {"cat", "scores", NULL};
        execvp("cat", cat_args);
        perror("execvp cat failed");
        exit(1);
    }

    return 0;
}