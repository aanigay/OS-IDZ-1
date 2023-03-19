# Нигай Александр БПИ-219
# Отчет по ИДЗ-1
## Вариант 6
## Условие задачи:
```
Разработать программу, заменяющую все согласные буквы в заданной ASCII-строке их ASCII кодами в десятичной системе счисления
```
### На 4 балла
## Схема взаимодействия процессов (если криво отображается, есть картинка схемы scheme.jpg)

```
Чтение из файла              Процесс 1              Запись в канал 1
input.txt --------> fd1[1] ------------> fd1[0] ------------> fd2[1] ------------> output.txt
                                           |  |   ^                   |
                                           |  |   |                   |
                                           v  v   |                   v
                                        Процесс 2 |             Процесс 3
                                             |    |              |    |
                                             v    |              |    v
                                        Замена букв        |  Запись в файл
                                        на ASCII коды      |  из канала 2
                                                           v
                                                    Закрытие каналов

```
## Код программы

```c
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
        close(fd1[1]);
        close(fd2[0]);
        int n;
        n = read(fd1[0], buf, BUFFER_SIZE);
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

    pid3 = fork();

    if (pid3 == -1)
    {
        perror("fork");
        return 1;
    }

    if (pid3 == 0)
    { // процесс 3
        close(fd2[1]);
        int fd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (fd == -1)
        {
            perror("open");
            return 1;
        }
        int n;
        n = read(fd2[0], buf, BUFFER_SIZE);
        if (write(fd, buf, n) != n)
        {
            perror("write");
            return 1;
        }
        close(fd);
        close(fd2[0]);
        exit(0);
    }

    close(fd1[0]); close(fd1[1]); close(fd2[0]); close(fd2[1]);
    wait(NULL); wait(NULL); wait(NULL);

    return 0;
}
```

## Результаты выполнения программы
Файл 1
```
peragoltamgoert ---> 112e114a103o108116a109103oe114116
```
Файл 2
```
dsfeoprviiiiaadte ---> 100115102eo112114118iiiiaa100116e
```
Файл 3
```
nvdsjerpoiujpo ---> 110118100115106e114112oiu106112o
```
Файл 4
```
asdkjfpioerjmvfd ---> a115100107106102112ioe114106109118102100
```
Файл 5
```
12h-1ng--=nbq=ein1=ng8n41nqoiyoorqwe ---> 12104-1110103--=11098113=ei1101=110103811041110113oi121oo114113119e
```

### На 5 баллов
## Cхема такая же как на 4 балла, только теперь каналы имеют названия pipe1 и pipe2.

## Код программы
```c 
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
```

## Результаты выполнения программы
Файл 1
```
peragoltamgoert ---> 112e114a103o108116a109103oe114116
```
Файл 2
```
dsfeoprviiiiaadte ---> 100115102eo112114118iiiiaa100116e
```
Файл 3
```
nvdsjerpoiujpo ---> 110118100115106e114112oiu106112o
```
Файл 4
```
asdkjfpioerjmvfd ---> a115100107106102112ioe114106109118102100
```
Файл 5
```
12h-1ng--=nbq=ein1=ng8n41nqoiyoorqwe ---> 12104-1110103--=11098113=ei1101=110103811041110113oi121oo114113119e
```

### На 6 баллов
## Схема
```
+-------+                     +-------+
| проц. |    fd1[1]           | проц. |
|   1   |-------------------> |   2   |
|       |<------------------- |       |
|       |       fd1[0]        |       |
+-------+                     +-------+

```
## Код программы 
```c
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
```

## Результаты выполнения программы
Файл 1
```
peragoltamgoert ---> 112e114a103o108116a109103oe114116
```
Файл 2
```
dsfeoprviiiiaadte ---> 100115102eo112114118iiiiaa100116e
```
Файл 3
```
nvdsjerpoiujpo ---> 110118100115106e114112oiu106112o
```
Файл 4
```
asdkjfpioerjmvfd ---> a115100107106102112ioe114106109118102100
```
Файл 5
```
12h-1ng--=nbq=ein1=ng8n41nqoiyoorqwe ---> 12104-1110103--=11098113=ei1101=110103811041110113oi121oo114113119e
```