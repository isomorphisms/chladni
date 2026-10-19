#define _POSIX_C_SOURCE 200809L
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

static void check(int condition, const char *message)
{
    if (!condition) { fprintf(stderr, "FAIL: %s\n", message); exit(1); }
}

static void run(char *const arguments[], int expected_status, int image_expected)
{
    char output_name[] = "/tmp/chladni-cli-XXXXXX";
    char error_name[] = "/tmp/chladni-error-XXXXXX";
    const int output = mkstemp(output_name), error = mkstemp(error_name);
    check(output >= 0 && error >= 0, "temporary CLI captures");
    check(unlink(output_name) == 0 && unlink(error_name) == 0, "anonymous captures");
    const pid_t child = fork();
    check(child >= 0, "fork CLI");
    if (child == 0) {
        if (dup2(output, STDOUT_FILENO) < 0 || dup2(error, STDERR_FILENO) < 0) _exit(126);
        close(output); close(error);
        execv(arguments[0], arguments);
        _exit(127);
    }
    int status;
    check(waitpid(child, &status, 0) == child, "wait for CLI");
    check(WIFEXITED(status) && WEXITSTATUS(status) == expected_status, "exact CLI status");
    struct stat output_stat, error_stat;
    check(fstat(output, &output_stat) == 0 && fstat(error, &error_stat) == 0,
          "capture metadata");
    check(error_stat.st_size > 0, "CLI diagnostic captured");
    if (image_expected) {
        const char header[] = "P5\n100 96\n255\n";
        char actual[sizeof(header) - 1];
        check(lseek(output, 0, SEEK_SET) == 0, "rewind PGM");
        check(read(output, actual, sizeof(actual)) == (ssize_t)sizeof(actual), "PGM header size");
        check(memcmp(actual, header, sizeof(actual)) == 0, "exact PGM header");
        check(output_stat.st_size == (off_t)(sizeof(actual) + 100 * 96), "exact PGM byte count");
    } else check(output_stat.st_size == 0, "invalid request emits no image");
    close(output); close(error);
}

int main(int argc, char **argv)
{
    check(argc == 2, "absolute CLI executable required");
    char *valid[] = {argv[1], "1", "1", "1", "2", "100", "96", "antisymmetric", NULL};
    char *invalid[] = {argv[1], "2", "1", "1", "2", "100", "96", "antisymmetric", NULL};
    char *nonfinite[] = {argv[1], "nan", "1", "1", "2", "100", "96", NULL};
    run(valid, 0, 1);
    run(invalid, 2, 0);
    run(nonfinite, 2, 0);
    puts("ICK CLI: PGM and negative controls PASS");
    return 0;
}
