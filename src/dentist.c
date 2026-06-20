#include "dentist.h"
#include "database.h"
#include "logs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

/*
 * Salva um novo dentista no banco de dados
 * Retorna 1 em caso de sucesso, 0 caso contrario
 */
int save_dentist(Dentist *dentist) {
    database_init();
    char line[1024];
    // Gera identificador unico global para o novo dentista
    dentist->dentist_id = generate_unique_id();
    // Formata a linha de dados segundo o padrao CSV para armazenamento
    // OBS: PRIu64 e uma macro para formatar inteiros uint64_t
    snprintf(line, sizeof(line), "%" PRIu64 ";%s;%s;%s;%d\n", dentist->dentist_id, dentist->name, dentist->cpf, dentist->password, dentist->role);
    if (db_append_line(DENTIST_FILE, line)) {
        // OBS: PRIu64 e uma macro para formatar inteiros uint64_t
        log_message(LOG_INFO, "Dentist %s saved with ID %" PRIu64 ".", dentist->name, dentist->dentist_id);
        return 1;
    }
    return 0;
}

/*
 * Verifica as credenciais de um dentista
 */
int check_dentist(const char *cpf, const char *password) {
    return validate_login(cpf, password);
}

/*
 * Atualiza os dados de um dentista existente
 * Retorna 1 se atualizado com sucesso, 0 caso contrario
 */
int update_dentist(Dentist *dentist) {
    // Busca registro previo pelo CPF para evitar atualizacao invalida
    Dentist existing = find_dentist_by_cpf(dentist->cpf);
    if (existing.dentist_id == (uint64_t)-1) return 0;
    
    char new_line[1024];
    // Prepara a string CSV formatada
    // OBS: PRIu64 e uma macro para formatar inteiros uint64_t
    snprintf(new_line, sizeof(new_line), "%" PRIu64 ";%s;%s;%s;%d\n", existing.dentist_id, dentist->name, dentist->cpf, dentist->password, dentist->role);
    // Realiza a atualizacao da linha buscando a coluna de CPF
    int updated = db_update_line(DENTIST_FILE, DENTIST_FILE_TEMP, 2, dentist->cpf, new_line, 5);
    if (updated) {
        log_message(LOG_INFO, "Dentist %s updated.", dentist->name);
    }
    return updated;
}

/*
 * Remove um dentista do banco de dados a partir de seu ID
 * Retorna 1 em caso de sucesso, 0 caso contrario
 */
int delete_dentist(uint64_t dentist_id) {
    char filter_val[32];
    // OBS: PRIu64 e uma macro para formatar inteiros uint64_t
    snprintf(filter_val, sizeof(filter_val), "%" PRIu64, dentist_id);
    int deleted = db_delete_lines(DENTIST_FILE, DENTIST_FILE_TEMP, 0, filter_val, 5);
    if (deleted > 0) {
        // OBS: PRIu64 e uma macro para formatar inteiros uint64_t
        log_message(LOG_INFO, "Dentist ID %" PRIu64 " deleted.", dentist_id);
    }
    return deleted > 0 ? 1 : 0;
}

/*
 * Valida o login do dentista verificando CPF e senha
 * Retorna 1 para credenciais validas, 0 caso contrario
 */
int validate_login(const char *cpf, const char *password) {
    // Tenta abrir banco de dados de dentistas para leitura de credenciais
    FILE *file = fopen(DENTIST_FILE, "r");
    if (!file) return 0;

    char line[1024];
    char *fields[5];
    int valid = 0;

    // Itera por todas as linhas registradas
    while (fgets(line, sizeof(line), file)) {
        // Pula o cabecalho de colunas e linhas quebras/vazias
        if (line[0] == '\n' || line[0] == '\r') continue;
        if (strncmp(line, "dentist_id;", 11) == 0 || strncmp(line, "id;", 3) == 0) continue;

        char line_copy[1024];
        strcpy(line_copy, line);
        // Separa a linha em vetores atraves dos delimitadores
        int cols = split_csv_line(line_copy, fields, 5);
        if (cols < 5) continue;

        // Compara credenciais fornecidas com as armazenadas (CPF e senha)
        if (strcmp(fields[2], cpf) == 0 && strcmp(fields[3], password) == 0) {
            valid = 1;
            break;
        }
    }
    fclose(file);
    return valid;
}

/*
 * Busca um dentista a partir do seu CPF
 */
Dentist find_dentist_by_cpf(const char *cpf) {
    // Inicializa a estrutura de retorno indicando falha previa
    Dentist found;
    memset(&found, 0, sizeof(Dentist));
    found.dentist_id = -1;

    // Abre arquivo para busca iterativa
    FILE *file = fopen(DENTIST_FILE, "r");
    if (!file) return found;

    char line[1024];
    char *fields[5];
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '\n' || line[0] == '\r') continue;
        if (strncmp(line, "dentist_id;", 11) == 0 || strncmp(line, "id;", 3) == 0) continue;

        char line_copy[1024];
        strcpy(line_copy, line);
        int cols = split_csv_line(line_copy, fields, 5);
        if (cols < 5) continue;

        // Preenche dados ao encontrar o registro com CPF correspondente e encerra busca
        if (strcmp(fields[2], cpf) == 0) {
            found.dentist_id = strtoull(fields[0], NULL, 10);
            strcpy(found.name, fields[1]);
            strcpy(found.cpf, fields[2]);
            strcpy(found.password, fields[3]);
            found.role = atoi(fields[4]);
            break;
        }
    }
    fclose(file);
    return found;
}
