#ifndef LOGS_H
#define LOGS_H

#include <stddef.h>

/**
 * @brief Representa os níveis de severidade disponíveis no sistema de logs.
 */
typedef enum {
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR,
    LOG_DEBUG
} LogLevel;

/**
 * @brief Converte o nível de log para sua representação textual (string).
 * 
 * @param level O nível de log (LOG_INFO, LOG_WARNING, etc.).
 * @return Retorna a string representativa ("INFO", "WARNING", "ERROR", "DEBUG" ou "UNKNOWN").
 */
const char* log_level_to_string(LogLevel level);

/**
 * @brief Obtém o carimbo de data/hora atual formatado como string.
 * 
 * @param buffer Ponteiro para o buffer de caracteres que receberá a string formatada.
 * @param size Tamanho máximo suportado pelo buffer para evitar estouro de memória.
 */
void get_timestamp(char *buffer, size_t size);

/**
 * @brief Inicializa o sistema de logs, criando a pasta 'logs' se ela não existir e abrindo o arquivo de saída.
 * 
 * @return Retorna 1 se o logger foi inicializado com sucesso ou 0 caso falhe ao abrir o arquivo.
 */
int init_logger(void);

/**
 * @brief Encerra o sistema de logs fechando o descritor de arquivo aberto.
 */
void close_logger(void);

/**
 * @brief Imprime uma mensagem de log formatada (estilo printf) simultaneamente no console e no arquivo de log configurado.
 * 
 * @param level O nível de severidade do log (LOG_INFO, LOG_WARNING, etc.).
 * @param format A string de formato estilo printf.
 * @param ... Os argumentos adicionais de formatação.
 */
void log_message(LogLevel level, const char *format, ...);

#endif // LOGS_H
