#include "database.h"
#include "logs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <stdint.h>
#include <time.h>

int split_csv_line(char *line, char **fields, int max_fields) {
    int count = 0;
    char *curr = line;
    while (*curr != '\0' && count < max_fields) {
        fields[count++] = curr;
        char *delim = strchr(curr, ';');
        if (delim != NULL) {
            *delim = '\0';
            curr = delim + 1;
        } else {
            char *nl = strchr(curr, '\n');
            if (nl != NULL) *nl = '\0';
            nl = strchr(curr, '\r');
            if (nl != NULL) *nl = '\0';
            break;
        }
    }
    return count;
}

void database_init(void) {
    CreateDirectoryA("database", NULL);

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

int db_delete_lines(const char *filepath, const char *temp_filepath, int filter_col_idx, const char *filter_val, int max_cols) {
    FILE *src = fopen(filepath, "r");
    if (src == NULL) return 0;

    FILE *dest = fopen(temp_filepath, "w");
    if (dest == NULL) {
        fclose(src);
        return 0;
    }

    char line[1024];
    char line_copy[1024];
    char **fields = malloc(max_cols * sizeof(char*));
    int deleted = 0;

    while (fgets(line, sizeof(line), src)) {
        if (line[0] == '\n' || line[0] == '\r') {
            fprintf(dest, "%s", line);
            continue;
        }

        strcpy(line_copy, line);
        int cols = split_csv_line(line_copy, fields, max_cols);

        if (cols > filter_col_idx && strcmp(fields[filter_col_idx], filter_val) == 0) {
            deleted++; // Pula a escrita
        } else {
            fprintf(dest, "%s", line);
        }
    }

    free(fields);
    fclose(src);
    fclose(dest);

    remove(filepath);
    rename(temp_filepath, filepath);

    return deleted;
}

int db_update_line(const char *filepath, const char *temp_filepath, int filter_col_idx, const char *filter_val, const char *new_line, int max_cols) {
    FILE *src = fopen(filepath, "r");
    if (src == NULL) return 0;

    FILE *dest = fopen(temp_filepath, "w");
    if (dest == NULL) {
        fclose(src);
        return 0;
    }

    char line[1024];
    char line_copy[1024];
    char **fields = malloc(max_cols * sizeof(char*));
    int updated = 0;

    while (fgets(line, sizeof(line), src)) {
        if (line[0] == '\n' || line[0] == '\r') {
            fprintf(dest, "%s", line);
            continue;
        }

        strcpy(line_copy, line);
        int cols = split_csv_line(line_copy, fields, max_cols);

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

    remove(filepath);
    rename(temp_filepath, filepath);

    return updated;
}

// Função para gerar um ID único de 64 bits
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