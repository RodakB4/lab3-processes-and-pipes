// C program to demonstrate 2-way communication using fork() and pipe()
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<string.h>
#include<sys/wait.h>

int main()
{
    // fd1: P1 (parent) -> P2 (child)
    // fd2: P2 (child)  -> P1 (parent)
    int fd1[2];
    int fd2[2];

    char fixed_str[]  = "howard.edu";
    char fixed_str2[] = "gobison.org";
    char input_str[100];
    pid_t p;

    if (pipe(fd1) == -1)
    {
        fprintf(stderr, "Pipe Failed");
        return 1;
    }
    if (pipe(fd2) == -1)
    {
        fprintf(stderr, "Pipe Failed");
        return 1;
    }

    printf("Enter a string to concatenate:");
    scanf("%99s", input_str);
    fflush(stdout);   // flush before fork so the prompt isn't duplicated

    p = fork();

    if (p < 0)
    {
        fprintf(stderr, "fork Failed");
        return 1;
    }

    // Parent process (P1)
    else if (p > 0)
    {
        close(fd1[0]);  // P1 only writes to fd1
        close(fd2[1]);  // P1 only reads from fd2

        // Send the original string to P2
        write(fd1[1], input_str, strlen(input_str) + 1);
        close(fd1[1]);

        // Receive the string P2 sends back
        char result[300];
        read(fd2[0], result, 300);
        close(fd2[0]);

        // Append "gobison.org" and print
        strcat(result, fixed_str2);
        printf("Concatenated string %s\n", result);

        wait(NULL);
    }

    // Child process (P2)
    else
    {
        close(fd1[1]);  // P2 only reads from fd1
        close(fd2[0]);  // P2 only writes to fd2

        // Read the string from P1
        char concat_str[300];
        read(fd1[0], concat_str, 100);
        close(fd1[0]);

        // Append "howard.edu" and print
        strcat(concat_str, fixed_str);
        printf("Concatenated string %s\n", concat_str);

        // Prompt for a second input and append it
        char second_str[100];
        printf("Enter a string to concatenate:");
        fflush(stdout);
        scanf("%99s", second_str);
        strcat(concat_str, second_str);

        // Send the result back to P1
        write(fd2[1], concat_str, strlen(concat_str) + 1);
        close(fd2[1]);

        exit(0);
    }

    return 0;
}