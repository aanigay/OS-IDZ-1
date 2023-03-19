#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>

#define BUFFER_SIZE 5000

int main(int argc, char *argv[])
{
    int fd1, fd2, fd3;
    pid_t pid1, pid2, pid3;
    char buf[BUFFER_SIZE];

    unlink("pipe1");
    unlink("pipe2");

    if (argc != 3)
    {
        fprintf(stderr, "incorrect input\n");
        return 1;
    }

    if (mkfifo("pipe1", 0666) == -1 || mkfifo("pipe2", 0666) == -1)
    {
        perror("pipe");
        return 1;
    }

    pid1 = fork();

    if (pid1 == -1)
    {
        perror("fork");
        return 1;
    }

    if (pid1 == 0)
    { // процесс 1
        int fd = open(argv[1], O_RDONLY);
        if (fd == -1)
        {
            perror("open");
            return 1;
        }
        fd1 = open("pipe1", O_WRONLY);
        int n;
        n = read(fd, buf, BUFFER_SIZE);
        printf("Read %d bytes from file\n", n);
        if (write(fd1, buf, n) != n)
        {
            perror("write");
            return 1;
        }
        close(fd);
        close(fd1);
        exit(0);
    }

    pid2 = fork();

    if (pid2 == -1)
    {
        perror("fork");
        return 1;
    }

    if (pid2 == 0)
    { // процесс 2
        fd1 = open("pipe1", O_RDONLY);
        fd2 = open("pipe2", O_WRONLY);
        int n;
        n = read(fd1, buf, BUFFER_SIZE);
        char *result = (char *)malloc((n * 3 + 1) * sizeof(char));
        int j = 0;

        for (int i = 0; i < n; i++)
        {
            if (!isalpha(buf[i]) || strchr("AEIOUaeiou", buf[i]))
            {
                result[j] = buf[i];
                j++;
            }
            else
            {
                int count = 0, num = (int)buf[i], res = 0;
                sprintf(&result[j], "%d", num);
                if (num == 0)
                {
                    res = 1;
                }
                while (num != 0)
                {
                    count++;
                    num /= 10;
                }
                j += count;
            }
        }
        result[j] = '\0';

        if (write(fd2, result, j) != j)
        {
            perror("write");
            return 1;
        }
        printf("Wrote %d bytes to channel 2\n", j);
        printf("Ended 1\n");

        free(result);
        close(fd1);
        close(fd2);
        exit(0);
    }

    pid3 = fork();

    if (pid3 == -1)
    {
        perror("fork");
        return 1;
    }

    if (pid3 == 0)
    { // процесс 3
        int fd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (fd == -1)
        {
            perror("open");
            return 1;
        }
        fd2 = open("pipe2", O_RDONLY);
        int n;
        n = read(fd2, buf, BUFFER_SIZE);
        printf("Read %d bytes from channel 3\n", n);
        if (write(fd, buf, n) != n)
        {
            perror("write");
            return 1;
        }
        close(fd);
        close(fd2);
        exit(0);
    }

    wait(NULL); wait(NULL); wait(NULL);
    unlink("pipe1"); unlink("pipe2");
    return 0;
}