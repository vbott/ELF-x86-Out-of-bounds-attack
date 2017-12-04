#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <alloca.h>


#define ALLOCA_MAX_SIZE 4096
#define BUF_SIZE (8)

#define N_ELEMS(x) (int) (sizeof x / sizeof x[0])
#define die() do { fprintf(stderr, "died at line %u\n", __LINE__); exit(__LINE__); } while (0)

static long base = 10;

static void prologue(void)
{
        char name[64];

        printf("Enter your name >>\n");
        (void) fgets(name, sizeof name, stdin);
        name[strcspn(name, "\n")] = '\0';
        printf("Hi %s, we're going to display an integer array in binary, "
               "decimal or hexadecimal.\n", name);
}

static unsigned char set_arithmetic_base(void)
{
        unsigned char b;
        char buf[BUF_SIZE] = "";

        printf("Set the base you'll work with (default=%ld) >>\n", base);
        (void) fgets(buf, sizeof buf, stdin);
        if ('\0' != buf[0]) {
                char *endptr = NULL;
                errno = 0;
                b = strtoul(buf, &endptr, 10);
                if (ERANGE == errno || '\0' == *endptr) {
                        perror("strtoul");
                        die();
                }
                if (b < 2 ||  b > 16) {
                        puts("Please, provide a widely used base.");
                        die();
                }
        }

        return b;
}

static void get_input_string(char *buf, size_t buf_len)
{
        int c = 0;
        size_t n = 0;

        while (1) {
                c = getchar();
                if (EOF == c || '\n' == c)
                        break;
                if (n >= buf_len)
                        continue;
                buf[n++] = c;
        }
}

static long get_integer_from_user(void)
{
        unsigned long v;
        char buf[BUF_SIZE] = "";
        char *endptr = NULL;

        get_input_string(buf, sizeof buf - 1);
        errno = 0;
        v = strtoul(buf, &endptr, base);
        if (ERANGE == errno || '\0' != *endptr) {
                perror("strtoul");
                die();
        }

        return v;
}

static void display_binary(int value)
{
        /* another bit-trick to display binary values */
        unsigned v = value;
        putchar('0');
        for (; v; v >>= 1)
                putchar('0' + (v & 1));
        puts("b");
}

static void display_hexadecimal(int value)
{
        printf("%#x\n", value);
}

static void display_decimal(int value)
{
        printf("%d\n", value);
}

static void display_array(int *array, int canary)
{
        while (*array != canary) {
                unsigned v = *array++;
                printf("Value: %u, %#x, ", v, v);
                display_binary(v);
        }
}

static void process(void)
{
        int canary = 0xcdcdcdcd;
        int *array = NULL;
        int array_size;

        printf("The size of the dynamic array we'll store values in (max "
               "%d) >>\n", ALLOCA_MAX_SIZE);
        if ((array_size = abs(get_integer_from_user())) > ALLOCA_MAX_SIZE)
                array_size = ALLOCA_MAX_SIZE;

        /* no need to check the return value of alloca(2) */
        array = alloca((array_size + 1) * sizeof *array);
        /* set a canary, just in case... */
        array[array_size] = canary;

        while (1) {
                void (*callbacks[])(int) = {
                        [0] = display_binary,
                        [1] = display_hexadecimal,
                        [2] = display_decimal
                };
                int value = 0, mode = 0, do_quit = 0, index = 0;

                printf("Exit the loop? Yes(0, default), No(1) >>\n");
                if ((do_quit = get_integer_from_user()) == 0)
                        break;

                printf("Enter the value to display >>\n");
                value = get_integer_from_user();

                printf("Array index we'll store the value (%#x) at >>\n", value);
                index = get_integer_from_user();
                index = abs(index) % array_size;
                array[index] = value;

                printf("Display mode? 0=binary, 1=hexadecimal, 2=decimal >>\n");
                mode = get_integer_from_user();
                mode = abs(mode) % N_ELEMS(callbacks);
                callbacks[mode](value);
        }

        printf("Display the whole array? Yes(0, default), No(1) >>\n");
        if (0 == get_integer_from_user())
                display_array(array, canary);
}

int main(void)
{
        sleep(1);
        prologue();
        base = set_arithmetic_base();
        process();
        return 0;
}