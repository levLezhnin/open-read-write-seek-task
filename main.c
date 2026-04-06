#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>

#define DEFAULT_BLOCK_SIZE 4096

size_t blockSize = DEFAULT_BLOCK_SIZE;
char* srcFilePath = NULL;
char* destFilePath = NULL;

void exitWithPerror(const char* message) {
    perror(message);
    exit(1);
}

void exitWithError(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    exit(1);
}

// По умолчанию считаем, что первый прочитанный строковый аргумент - путь откуда читаем; второй прочитанный строковый аргумент - путь куда пишем.
// По параметру с флагом -b считываем размер блока.
void handleInputParameters(int argc, char** argv) {
    int opt;

    while ((opt = getopt(argc, argv, "b:")) != -1) {
        switch (opt) {
            case 'b':
                blockSize = (size_t) atol(optarg);
                if (blockSize == 0) {
                    exitWithError("Ошибка: некорректный размер блока.\n");
                }
                break;
            default:
                exitWithError("Использование: %s [-b BLOCK_SIZE] [INPUT] OUTPUT\n", argv[0]);
        }
    }

    int argsLeft = argc - optind;
    if (argsLeft == 1) {
        destFilePath = argv[optind];
    } else if (argsLeft == 2) {
        srcFilePath = argv[optind];
        destFilePath = argv[optind + 1];
    } else {
        exitWithError("Ошибка: требует 1 или 2 аргумента\n");
    }
}

bool isAllZeros(char* buf, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        if (buf[i] != '\0') {
            return false;
        }
    }
    return true;
}

void sparse(int fdIn, int fdOut) {
    char* buf = malloc(blockSize);
    if (!buf) {
        exitWithPerror("malloc");
    }

    off_t totalSize = 0;
    ssize_t n;

    while ((n = read(fdIn, buf, blockSize)) > 0) {
        if (isAllZeros(buf, n)) {
            if (lseek(fdOut, n, SEEK_CUR) == -1) {
                exitWithPerror("lseek");
            }
        } else {
            ssize_t written = 0;
            while (written < n) {
                ssize_t w = write(fdOut, buf + written, n - written);
                if (w == -1) {
                    exitWithPerror("write");
                }
                written += w;
            }
        }
        totalSize += n;
    }

    if (n == -1) {
        exitWithPerror("read");
    }

    if (ftruncate(fdOut, totalSize) == -1) {
        exitWithPerror("ftruncate");
    }

    free(buf);
}

int main(int argc, char** argv) {
    handleInputParameters(argc, argv);

    int fdIn = STDIN_FILENO;
    if (srcFilePath) {
        fdIn = open(srcFilePath, O_RDONLY);
        if (fdIn == -1) {
            exitWithPerror("open (input)");
        }
    }

    int fdOut = open(destFilePath, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fdOut == -1) {
        exitWithPerror("open (output)");
    }

    sparse(fdIn, fdOut);

    if (fdIn != STDIN_FILENO) {
        close(fdIn);
    }
    close(fdOut);

    return 0;
}
