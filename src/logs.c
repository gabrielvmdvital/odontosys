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
    time_t now = time(NULL); // time obtém o timestamp atual do sistema
    struct tm *t = localtime(&now); // localtime converte timestamp para estrutura de tempo local
    if (t != NULL) {
        strftime(buffer, size, "%Y-%m-%d %H:%M:%S", t);
    } else {
        strncpy(buffer, "HORA DESCONHECIDA", size); // strncpy copia N caracteres de uma string, prevenindo overflow
    }
}

// Inicializa o sistema de logs
int init_logger(const char *filename) {
    if (filename != NULL) {
        // Verifica se o caminho base usa o diretorio padrao "logs"
        if (strstr(filename, "logs/") == filename || strstr(filename, "logs\\") == filename) {
            // Garante que o diretorio exista antes de iniciar o log
            CreateDirectoryA("logs", NULL);
        }

        // Abre o arquivo de log no modo de adicao para nao sobrescrever registros anteriores
        log_file = fopen(filename, "a"); // fopen abre um arquivo
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
        fclose(log_file); // fclose fecha o manipulador do arquivo
        log_file = NULL;
    }
}

// Grava uma mensagem de log com argumentos variáveis
void log_message(LogLevel level, const char *format, ...) {
    char timestamp[20];
    // Extrai o horario formatado do sistema
    get_timestamp(timestamp, sizeof(timestamp));

    char prefix[50];
    // Estrutura o prefixo padrao da mensagem de log (horario e nivel)
    snprintf(prefix, sizeof(prefix), "[%s] [%s]: ", timestamp, log_level_to_string(level)); // snprintf formata com limite seguro de tamanho

    // Exibe a mensagem estruturada na saida padrao do terminal (console)
    printf("%s", prefix);
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");

    if (log_file) {
        // Grava fisicamente a mesma mensagem estruturada no arquivo de log
        fprintf(log_file, "%s", prefix);
        va_start(args, format);
        vfprintf(log_file, format, args);
        va_end(args);
        fprintf(log_file, "\n");
        // Forca a gravacao imediata no disco para evitar perda por encerramento subido
        fflush(log_file);
    }
}
