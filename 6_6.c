#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUFFER_SIZE 5000

int main(int argc, char *argv[])
{
    int fd1[2], fd2[2];
    pid_t pid1, pid2, pid3;
    char buf[BUFFER_SIZE];

    if (argc != 3)
    {
        fprintf(stderr, "incorrect input\n");
        return 1;
    }

    if (pipe(fd1) == -1 || pipe(fd2) == -1)
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
        close(fd1[0]);
        int fd = open(argv[1], O_RDONLY);
        if (fd == -1)
        {
            perror("open");
            return 1;
        }
        int n;
        n = read(fd, buf, BUFFER_SIZE);
        if (write(fd1[1], buf, n) != n)
        {
            perror("write");
            return 1;
        }
        close(fd);
        close(fd1[1]);

        wait(NULL);

        close(fd2[1]);                                          
        fd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0666); 
        if (fd == -1)
        {
            perror("open");
            return 1;
        }
        n = read(fd2[0], buf, BUFFER_SIZE);

        if (write(fd, buf, n) != n)
        {
            perror("write");
            return 1;
        }
        close(fd);
    }

    pid2 = fork();

    if (pid2 == -1)
    {
        perror("fork");
        return 1;
    }

    if (pid2 == 0)
    { // процесс 2
        close(fd1[1]);
        close(fd2[0]);
        int n;
        n = read(fd1[0], buf, BUFFER_SIZE);
        char *result = (char *)malloc((n * 3 + 1) * sizeof(char));
        int j = 0;

        for (int i = 0; i < n; i++)
        {
            if (!isalpha(buf[i]) || strchr("aeiouAEIOU", buf[i]))
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

        if (write(fd2[1], result, j) != j)
        {
            perror("write");
            return 1;
        }
        free(result);
        close(fd1[0]);
        close(fd2[1]);
        exit(0);
    }

    close(fd1[0]); close(fd1[1]); close(fd2[0]); close(fd2[1]);
    wait(NULL); wait(NULL);

    return 0;
}