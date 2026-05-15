#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define DEFAULT_LDR_PATH "/sys/kernel/smartlamp/ldr"
#define DEFAULT_BACKLIGHT_PATH "/sys/class/backlight/intel_backlight"
#define DEFAULT_INTERVAL_MS 1000
#define MIN_PERCENT 10

static volatile sig_atomic_t running = 1;

struct config {
    const char *ldr_path;
    const char *backlight_path;
    int interval_ms;
};

static void handle_signal(int signal)
{
    (void)signal;
    running = 0;
}

static int __attribute__((unused)) clamp(int value, int min, int max)
{
    if (value < min)
        return min;
    if (value > max)
        return max;
    return value;
}

static int read_int_file(const char *path, int *value)
{
    /*
     * Ponteiro para o arquivo que será aberto.
     */
    FILE *file;

    /*
     * Abre o arquivo em modo leitura.
     *
     * Exemplo:
     * /sys/kernel/smartlamp/ldr
     */
    file = fopen(path, "r");

    /*
     * Se fopen retornar NULL,
     * significa que ocorreu erro ao abrir o arquivo.
     */
    if (file == NULL)
    {
        return -1;
    }

    /*
     * fscanf lê um inteiro do arquivo.
     *
     * "%d" -> inteiro decimal
     * value -> variável onde o valor será armazenado
     *
     * Exemplo:
     * arquivo contém "75"
     * value receberá 75
     */
    if (fscanf(file, "%d", value) != 1)
    {
        /*
         * Fecha o arquivo antes de retornar erro.
         */
        fclose(file);

        return -1;
    }

    /*
     * Fecha o arquivo após a leitura.
     */
    fclose(file);

    /*
     * Retorna 0 indicando sucesso.
     */
    return 0;
}

static int __attribute__((unused)) write_int_file(const char *path, int value)
{
    /*
     * Ponteiro para o arquivo.
     */
    FILE *file;

    /*
     * Abre o arquivo em modo escrita.
     *
     * Exemplo:
     * /sys/class/backlight/intel_backlight/brightness
     */
    file = fopen(path, "w");

    /*
     * Se não conseguir abrir,
     * retorna erro.
     */
    if (file == NULL)
    {
        return -1;
    }

    /*
     * Escreve o valor inteiro no arquivo.
     *
     * "%d\n"
     * %d  -> inteiro decimal
     * \n  -> quebra de linha
     */
    fprintf(file, "%d\n", value);

    /*
     * Fecha o arquivo após a escrita.
     */
    fclose(file);

    /*
     * Retorna sucesso.
     */
    return 0;
}

static int ldr_to_brightness(int ldr, int max_brightness)
{
    /*
     * percent armazenará o percentual final
     * do brilho após os ajustes.
     */
    int percent;

    /*
     * Limita o valor do LDR entre 0 e 100.
     */

    if (ldr < 0)
    {
        ldr = 0;
    }

    if (ldr > 100)
    {
        ldr = 100;
    }

    /*
     * Aplica brilho mínimo.
     *
     * Exemplo:
     * MIN_PERCENT = 10
     *
     * Mesmo que o LDR seja 0,
     * o brilho nunca ficará totalmente apagado.
     */
    if (ldr < MIN_PERCENT)
    {
        percent = MIN_PERCENT;
    }
    else
    {
        percent = ldr;
    }

    /*
     * Converte o percentual para a escala real
     * do hardware usando max_brightness.
     *
     * Exemplo:
     *
     * percent = 50
     * max_brightness = 12000
     *
     * resultado = 6000
     */
    return (percent * max_brightness) / 100;
}

static void sleep_ms(int milliseconds)
{
    struct timespec request;

    request.tv_sec = milliseconds / 1000;
    request.tv_nsec = (long)(milliseconds % 1000) * 1000000L;

    while (running && nanosleep(&request, &request) == -1 && errno == EINTR) {
    }
}

static int parse_args(int argc, char **argv, struct config *config)
{
    int opt;

    while ((opt = getopt(argc, argv, "l:b:i:h")) != -1) {
        switch (opt) {
        case 'l':
            config->ldr_path = optarg;
            break;
        case 'b':
            config->backlight_path = optarg;
            break;
        case 'i':
            config->interval_ms = atoi(optarg);
            if (config->interval_ms <= 0)
                return -EINVAL;
            break;
        case 'h':
            printf("Usage: %s [-l ldr_path] [-b backlight_path] [-i interval_ms]\n", argv[0]);
            exit(0);
        default:
            return -EINVAL;
        }
    }

    return 0;
}

int main(int argc, char **argv)
{
    char max_path[512];
    char brightness_path[512];
    struct config config = {
        .ldr_path = DEFAULT_LDR_PATH,
        .backlight_path = DEFAULT_BACKLIGHT_PATH,
        .interval_ms = DEFAULT_INTERVAL_MS,
    };
    int max_brightness;

    if (parse_args(argc, argv, &config) < 0)
        return EXIT_FAILURE;

    snprintf(max_path, sizeof(max_path), "%s/max_brightness", config.backlight_path);
    snprintf(brightness_path, sizeof(brightness_path), "%s/brightness", config.backlight_path);

    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    if (read_int_file(max_path, &max_brightness) < 0 || max_brightness <= 0) {
        fprintf(stderr, "failed to read %s\n", max_path);
        return EXIT_FAILURE;
    }

    while (running) {
        int ldr;
        int brightness;

        if (read_int_file(config.ldr_path, &ldr) == 0) {
            brightness = ldr_to_brightness(ldr, max_brightness);

            // TASK 3.3: escreva brightness em brightness_path usando write_int_file().
            printf("ldr=%d brightness=%d max_brightness=%d\n", ldr, brightness, max_brightness);
            fflush(stdout);
        } else {
            fprintf(stderr, "failed to read %s\n", config.ldr_path);
        }

        sleep_ms(config.interval_ms);
    }

    return EXIT_SUCCESS;
}
