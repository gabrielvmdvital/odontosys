#include "dentist.h"
#include "database.h"
#include "logs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <locale.h>

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
    // PRIu64 e uma macro para formatar inteiros uint64_t
    snprintf(line, sizeof(line), "%" PRIu64 ";%s;%s;%s;%s;%d\n", dentist->dentist_id, dentist->name, dentist->username, dentist->cpf, dentist->password, dentist->role); // snprintf formata com limite seguro de tamanho
    if (db_append_line(DENTIST_FILE, line)) {
        log_message(LOG_INFO, "[DATABASE] Dentist %s saved with ID %" PRIu64 ".", dentist->name, dentist->dentist_id);
        return 1;
    }
    return 0;
}

/*
 * Verifica as credenciais de um dentista
 */
int check_dentist(const char *cpf_or_user, const char *password) {
    return validate_login(cpf_or_user, password);
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
    snprintf(new_line, sizeof(new_line), "%" PRIu64 ";%s;%s;%s;%s;%d\n", existing.dentist_id, dentist->name, dentist->username, dentist->cpf, dentist->password, dentist->role);
    // Realiza a atualizacao da linha buscando a coluna de CPF (agora na coluna index 3)
    int updated = db_update_line(DENTIST_FILE, DENTIST_FILE_TEMP, 3, dentist->cpf, new_line, 6);
    if (updated) {
        log_message(LOG_INFO, "[DATABASE] Dentist %s updated.", dentist->name);
    }
    return updated;
}

/*
 * Remove um dentista do banco de dados a partir de seu ID
 * Retorna 1 em caso de sucesso, 0 caso contrario
 */
int delete_dentist(uint64_t dentist_id) {
    char filter_val[32];
    snprintf(filter_val, sizeof(filter_val), "%" PRIu64, dentist_id);
    int deleted = db_delete_lines(DENTIST_FILE, DENTIST_FILE_TEMP, 0, filter_val, 6);
    if (deleted > 0) {
        log_message(LOG_INFO, "[DATABASE] Dentist ID %" PRIu64 " deleted.", dentist_id);
    }
    return deleted > 0 ? 1 : 0;
}

/*
 * Valida o login do dentista verificando Username/CPF e senha
 * Retorna o role (ex: 1) para credenciais validas, -1 caso contrario
 */
int validate_login(const char *cpf_or_user, const char *password) {
    // Tenta abrir banco de dados de dentistas para leitura de credenciais
    FILE *file = fopen(DENTIST_FILE, "r"); // fopen abre um arquivo
    if (!file) return -1;

    char line[1024];
    char *fields[6];
    int valid = -1;

    // Itera por todas as linhas registradas
    while (fgets(line, sizeof(line), file)) { // fgets lê uma linha do arquivo e previne overflow
        // Pula o cabecalho de colunas e linhas quebras/vazias
        if (line[0] == '\n' || line[0] == '\r') continue;
        if (strncmp(line, "dentist_id;", 11) == 0 || strncmp(line, "id;", 3) == 0) continue; // strncmp compara até N caracteres protegendo limites

        char line_copy[1024];
        strcpy(line_copy, line); // strcpy copia a string da origem para o destino
        // Separa a linha em vetores atraves dos delimitadores
        int cols = split_csv_line(line_copy, fields, 6);
        if (cols < 6) continue;

        // Compara credenciais fornecidas com as armazenadas (Username na col 2 ou CPF na col 3) e senha (col 4)
        if ((strcmp(fields[2], cpf_or_user) == 0 || strcmp(fields[3], cpf_or_user) == 0) && strcmp(fields[4], password) == 0) { // strcmp compara strings (0 = iguais)
            valid = atoi(fields[5]); // atoi converte string para inteiro numérico padrão
            break;
        }
    }
    fclose(file); // fclose fecha o manipulador do arquivo
    return valid;
}

/*
 * Busca um dentista a partir do seu CPF
 */
Dentist find_dentist_by_cpf(const char *cpf) {
    // Inicializa a estrutura de retorno indicando falha previa
    Dentist found;
    memset(&found, 0, sizeof(Dentist)); // memset preenche um bloco de memória com um valor
    found.dentist_id = -1;

    // Abre arquivo para busca iterativa
    FILE *file = fopen(DENTIST_FILE, "r");
    if (!file) return found;

    char line[1024];
    char *fields[6];
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '\n' || line[0] == '\r') continue;
        if (strncmp(line, "dentist_id;", 11) == 0 || strncmp(line, "id;", 3) == 0) continue;

        char line_copy[1024];
        strcpy(line_copy, line);
        int cols = split_csv_line(line_copy, fields, 6);
        if (cols < 6) continue;

        // Preenche dados ao encontrar o registro com CPF correspondente (coluna 3) ou Username (coluna 2)
        if (strcmp(fields[2], cpf) == 0 || strcmp(fields[3], cpf) == 0) {
            found.dentist_id = strtoull(fields[0], NULL, 10); // strtoull converte string para unsigned long long
            strcpy(found.name, fields[1]);
            strcpy(found.username, fields[2]);
            strcpy(found.cpf, fields[3]);
            strcpy(found.password, fields[4]);
            found.role = atoi(fields[5]);
            break;
        }
    }
    fclose(file);
    return found;
}
