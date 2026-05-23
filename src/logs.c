#include "logs.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include <windows.h>

// Ponteiro global para o arquivo de log
static FILE *log_file = NULL;

// Converte o nível de log para string
const char* log_level_to_string(LogLevel level) {
    switch (level) {
        case LOG_INFO:    return "INFO";
        case LOG_WARNING: return "WARNING";
        case LOG_ERROR:   return "ERROR";
        case LOG_DEBUG:   return "DEBUG";
        default:          return "UNKNOWN";
    }
}

// Obtém o carimbo de data/hora atual como string
void get_timestamp(char *buffer, size_t size) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    if (t != NULL) {
        strftime(buffer, size, "%Y-%m-%d %H:%M:%S", t);
    } else {
        strncpy(buffer, "HORA DESCONHECIDA", size);
    }
}

// Inicializa o sistema de logs
int init_logger(const char *filename) {
    if (filename != NULL) {
        if (strstr(filename, "logs/") == filename || strstr(filename, "logs\\") == filename) {
            CreateDirectoryA("logs", NULL);
        }

        log_file = fopen(filename, "a");
        if (!log_file) {
            perror("Falha ao abrir arquivo de log");
            return 0;
        }
    }
    return 1;
}

// Encerra o sistema de logs
void close_logger(void) {
    if (log_file) {
        fclose(log_file);
        log_file = NULL;
    }
}

// Grava uma mensagem de log com argumentos variáveis
void log_message(LogLevel level, const char *format, ...) {
    char timestamp[20];
    get_timestamp(timestamp, sizeof(timestamp));

    char prefix[50];
    snprintf(prefix, sizeof(prefix), "[%s] [%s]: ", timestamp, log_level_to_string(level));

    printf("%s", prefix);
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");

    if (log_file) {
        fprintf(log_file, "%s", prefix);
        va_start(args, format);
        vfprintf(log_file, format, args);
        va_end(args);
        fprintf(log_file, "\n");
        fflush(log_file);
    }
}
