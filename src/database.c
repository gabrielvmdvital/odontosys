#include "database.h"
#include "logs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <stdint.h>
#include <time.h>

/*
 * Divide uma linha em formato CSV em multiplos campos
 * Retorna o numero de campos encontrados
 */
int split_csv_line(char *line, char **fields, int max_fields) {
    int count = 0;
    char *curr = line;
    // Percorre a string ate encontrar o final ou atingir o limite de campos
    while (*curr != '\0' && count < max_fields) {
        fields[count++] = curr;
        char *delim = strchr(curr, ';');
        if (delim != NULL) {
            // Substitui o delimitador por caractere nulo para isolar o campo
            *delim = '\0';
            curr = delim + 1;
        } else {
            // Remove quebras de linha no ultimo campo caso existam
            char *nl = strchr(curr, '\n');
            if (nl != NULL) *nl = '\0';
            nl = strchr(curr, '\r');
            if (nl != NULL) *nl = '\0';
            break;
        }
    }
    return count;
}

/*
 * Inicializa a estrutura de diretorios e arquivos do banco de dados
 */
void database_init(void) {
    // Cria o diretorio base para os arquivos de dados caso nao exista
    CreateDirectoryA("database", NULL);

    // Inicializa o arquivo de pacientes com cabecalho caso esteja ausente
    FILE *fp = fopen(PATIENT_FILE, "r");
    if (!fp) {
        fp = fopen(PATIENT_FILE, "w");
        if (fp) {
            fprintf(fp, "patient_id;dentist_id;nome;email;cpf;data_nascimento;telefone\n");
            fclose(fp);
        }
    } else {
        fclose(fp);
    }

    // Inicializa o arquivo clinico com cabecalho caso esteja ausente
    FILE *fc = fopen(CLINICAL_FILE, "r");
    if (!fc) {
        fc = fopen(CLINICAL_FILE, "w");
        if (fc) {
            fprintf(fc, "clinical_id;patient_id;dentist_id;diag_date;idade;anb;coa;maxila_tipo;maxila_desvio;cogn;afai;sngogn;na1_dist;na1_ang;nb1_dist;nb1_ang;perf_tegument;pre_diagnostico\n");
            fclose(fc);
        }
    } else {
        fclose(fc);
    }

    // Inicializa o arquivo de dentistas com cabecalho e admin padrao caso ausente
    FILE *fu = fopen(DENTIST_FILE, "r");
    if (!fu) {
        fu = fopen(DENTIST_FILE, "w");
        if (fu) {
            fprintf(fu, "dentist_id;name;cpf;password;role\n");
            fprintf(fu, "0;admin;000.000.000-00;admin;1\n");
            fclose(fu);
        }
    } else {
        fclose(fu);
    }

    log_message(LOG_INFO, "Banco de dados inicializado.");
}

/*
 * Adiciona uma nova linha ao final do arquivo especificado
 * Retorna 1 em caso de sucesso, 0 caso contrario
 */
int db_append_line(const char *filepath, const char *line) {
    database_init();
    FILE *file = fopen(filepath, "a");
    if (file == NULL) {
        log_message(LOG_ERROR, "Nao foi possivel abrir o arquivo %s para append.", filepath);
        return 0;
    }
    fprintf(file, "%s", line);
    fclose(file);
    return 1;
}

/*
 * Remove linhas de um arquivo baseando-se no valor de uma coluna especifica
 * Retorna a quantidade de linhas removidas
 */
int db_delete_lines(const char *filepath, const char *temp_filepath, int filter_col_idx, const char *filter_val, int max_cols) {
    // Abre o arquivo fonte para leitura
    FILE *src = fopen(filepath, "r");
    if (src == NULL) return 0;

    // Abre um arquivo temporario para escrita
    FILE *dest = fopen(temp_filepath, "w");
    if (dest == NULL) {
        fclose(src);
        return 0;
    }

    char line[1024];
    char line_copy[1024];
    char **fields = malloc(max_cols * sizeof(char*));
    int deleted = 0;

    // Itera por todas as linhas do arquivo de origem
    while (fgets(line, sizeof(line), src)) {
        if (line[0] == '\n' || line[0] == '\r') {
            fprintf(dest, "%s", line);
            continue;
        }

        strcpy(line_copy, line);
        int cols = split_csv_line(line_copy, fields, max_cols);

        // Omite a escrita no arquivo destino se o valor bater com o filtro
        if (cols > filter_col_idx && strcmp(fields[filter_col_idx], filter_val) == 0) {
            deleted++; // Pula a escrita
        } else {
            fprintf(dest, "%s", line);
        }
    }

    free(fields);
    fclose(src);
    fclose(dest);

    // Substitui o arquivo original pelo temporario com as alteracoes
    remove(filepath);
    rename(temp_filepath, filepath);

    return deleted;
}

/*
 * Atualiza uma linha especifica em um arquivo, buscando pelo valor em uma coluna
 * Retorna 1 se a linha foi atualizada, 0 caso contrario
 */
int db_update_line(const char *filepath, const char *temp_filepath, int filter_col_idx, const char *filter_val, const char *new_line, int max_cols) {
    // Abre o arquivo fonte para leitura
    FILE *src = fopen(filepath, "r");
    if (src == NULL) return 0;

    // Abre um arquivo temporario para escrita
    FILE *dest = fopen(temp_filepath, "w");
    if (dest == NULL) {
        fclose(src);
        return 0;
    }

    char line[1024];
    char line_copy[1024];
    char **fields = malloc(max_cols * sizeof(char*));
    int updated = 0;

    // Itera por todas as linhas do arquivo de origem
    while (fgets(line, sizeof(line), src)) {
        if (line[0] == '\n' || line[0] == '\r') {
            fprintf(dest, "%s", line);
            continue;
        }

        strcpy(line_copy, line);
        int cols = split_csv_line(line_copy, fields, max_cols);

        // Substitui o conteudo se a linha coincidir com o valor filtrado
        if (cols > filter_col_idx && strcmp(fields[filter_col_idx], filter_val) == 0) {
            fprintf(dest, "%s", new_line);
            updated = 1;
        } else {
            fprintf(dest, "%s", line);
        }
    }

    free(fields);
    fclose(src);
    fclose(dest);

    // Substitui o arquivo original pelo temporario atualizado
    remove(filepath);
    rename(temp_filepath, filepath);

    return updated;
}

/*
 * Gera um ID unico de 64 bits combinando timestamp e contador interno
 */
uint64_t generate_unique_id(void) {
    static uint64_t contador = 0; // contador interno para evitar colisões no mesmo segundo
    time_t agora = time(NULL);

    if (agora == ((time_t)-1)) {
        fprintf(stderr, "Erro ao obter o tempo do sistema.\n");
        exit(EXIT_FAILURE);
    }

    // Incrementa o contador a cada chamada
    contador++;

    // Combina o timestamp (32 bits) com o contador (32 bits)
    uint64_t id = ((uint64_t)agora << 32) | (contador & 0xFFFFFFFF);
    return id;
}