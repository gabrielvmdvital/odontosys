#include "database.h"
#include "logs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

// Função auxiliar interna para dividir uma linha de CSV delimitada por ponto e vírgula.
static int split_csv_line(char *line, char **fields, int max_fields) {
    int count = 0;
    char *curr = line;
    while (*curr != '\0' && count < max_fields) {
        fields[count++] = curr;
        char *delim = strchr(curr, ';');
        if (delim != NULL) {
            *delim = '\0';
            curr = delim + 1;
        } else {
            // Remove quebras de linha (\r ou \n) no último campo
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
    // Cria a pasta "database" na raiz do projeto se ela não existir.
    CreateDirectoryA("database", NULL);
    log_message(LOG_INFO, "Banco de dados inicializado (diretorio 'database' verificado/criado).");
}

int get_cached_user_count(void) {
    FILE *f = fopen(USER_COUNT_FILE, "r");
    if (!f) return 0;
    int count = 0;
    fscanf(f, "%d", &count);
    fclose(f);
    return count;
}

void update_cached_user_count(int delta) {
    int count = get_cached_user_count();
    count += delta;
    if (count < 0) count = 0;
    FILE *f = fopen(USER_COUNT_FILE, "w");
    if (f) {
        fprintf(f, "%d", count);
        fclose(f);
    }
}

int save_user(User *user) {
    database_init();

    User existing = find_user_by_cpf(user->cpf);
    if (existing.id != -1) {
        log_message(LOG_WARNING, "Falha ao salvar: ja existe um usuario com o CPF %s.", user->cpf);
        return 0;
    }

    FILE *file = fopen(USER_FILE, "a");
    if (file == NULL) {
        log_message(LOG_ERROR, "Nao foi possivel abrir o arquivo %s para salvar o usuario.", USER_FILE);
        return 0;
    }

    // Formato CSV: id;nome;email;cpf;data_nascimento;altura;peso
    fprintf(file, "%d;%s;%s;%s;%s;%.2f;%.2f\n", 
            user->id, 
            user->name, 
            user->email, 
            user->cpf, 
            user->birth_date,
            user->metrics.height, 
            user->metrics.weight);
    fclose(file);
    log_message(LOG_INFO, "Usuario cadastrado no CSV com sucesso: %s (ID: %d)", user->name, user->id);
    update_cached_user_count(1);
    return 1;
}

User find_user_by_cpf(const char *cpf) {
            user->birth_date,
            user->metrics.height, 
            user->metrics.weight);      // ANALISAR ERRO !!!!!!
    fclose(file);
    log_message(LOG_INFO, "Usuario cadastrado no CSV com sucesso: %s (ID: %d)", user->name, user->id);
    update_cached_user_count(1);
    return 1;
}

User find_user_by_cpf(const char *cpf) {
    User found;
    found.id = -1;

    FILE *file = fopen(USER_FILE, "r");
    if (file == NULL) {
        return found;
    }

    char line[512];
    char *fields[7];
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '\n' || line[0] == '\r') continue;

        char line_copy[512];
        strcpy(line_copy, line);

        int cols = split_csv_line(line_copy, fields, 7);
        if (cols < 7) continue;

        if (strcmp(fields[3], cpf) == 0) {
            found.id = atoi(fields[0]);
            strcpy(found.name, fields[1]);
            strcpy(found.email, fields[2]);
            strcpy(found.cpf, fields[3]);
            strcpy(found.birth_date, fields[4]);
            found.metrics.height = atof(fields[5]);
            found.metrics.weight = atof(fields[6]);
            break;
        }
    }

    fclose(file);
    return found;
}

User find_user_by_id(int user_id) {
    User found;
    found.id = -1;

    FILE *file = fopen(USER_FILE, "r");
    if (file == NULL) {
        return found;
    }

    char line[512];
    char *fields[7];
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '\n' || line[0] == '\r') continue;

        char line_copy[512];
        strcpy(line_copy, line);

        int cols = split_csv_line(line_copy, fields, 7);
        if (cols < 7) continue;

        if (atoi(fields[0]) == user_id) {
            found.id = atoi(fields[0]);
            strcpy(found.name, fields[1]);
            strcpy(found.email, fields[2]);
            strcpy(found.cpf, fields[3]);
            strcpy(found.birth_date, fields[4]);
            found.metrics.height = atof(fields[5]);
            found.metrics.weight = atof(fields[6]);
            break;
        }
    }

    fclose(file);
    return found;
}

int update_user(int user_id, User *user) {

    User existing = find_user_by_cpf(user->cpf);
    if (existing.id != -1 && existing.id != user_id) {
        log_message(LOG_WARNING, "Falha ao atualizar: ja existe outro usuario com o CPF %s.", user->cpf);
        return 0;
    }

    FILE *src = fopen(USER_FILE, "r");
    if (src == NULL) {
        log_message(LOG_WARNING, "Tentativa de atualizar usuario ID %d, mas %s nao existe.", user_id, USER_FILE);
        return 0;
    }

    FILE *dest = fopen("database/usuarios.tmp", "w");
    if (dest == NULL) {
        log_message(LOG_ERROR, "Nao foi possivel criar o arquivo temporario database/usuarios.tmp para atualizacao.");
        fclose(src);
        return 0;
    }

    char line[512];
    char line_copy[512];
    char *fields[7];
    int updated = 0;

    while (fgets(line, sizeof(line), src)) {
        if (line[0] == '\n' || line[0] == '\r') {
            fprintf(dest, "%s", line);
            continue;
        }

        strcpy(line_copy, line);
        int cols = split_csv_line(line_copy, fields, 7);

        if (cols >= 7 && atoi(fields[0]) == user_id) {
            fprintf(dest, "%d;%s;%s;%s;%.2f;%.2f\n", 
                    user->id, 
                    user->name, 
                    user->email, 
                    user->cpf, 
                    user->birth_date,
                    user->metrics.height, 
                    user->metrics.weight);
            updated = 1;
        } else {
            fprintf(dest, "%s", line);
        }
    }

    fclose(src);
    fclose(dest);

    remove(USER_FILE);
    rename("database/usuarios.tmp", USER_FILE);

    if (updated) {
        log_message(LOG_INFO, "Usuario ID %d (%s) atualizado com sucesso no CSV.", user_id, user->name);
    } else {
        log_message(LOG_WARNING, "Nenhum usuario correspondente ao ID %d encontrado para atualizacao no CSV.", user_id);
    }

    return updated;
}

int delete_user(int user_id) {
    FILE *src = fopen(USER_FILE, "r");
    if (src == NULL) {
        return 0;
    }

    FILE *dest = fopen(USER_FILE_TEMP, "w");
    if (dest == NULL) {
        fclose(src);
        return 0;
    }

    char line[512];
    char line_copy[512];
    char *fields[7];
    int deleted = 0;

    while (fgets(line, sizeof(line), src)) {
        if (line[0] == '\n' || line[0] == '\r') {
            fprintf(dest, "%s", line);
            continue;
        }

        strcpy(line_copy, line);
        int cols = split_csv_line(line_copy, fields, 7);

        if (cols >= 7 && atoi(fields[0]) == user_id) {
            deleted = 1;
        } else {
            fprintf(dest, "%s", line);
        }
    }

    fclose(src);
    fclose(dest);

    remove(USER_FILE);
    rename(USER_FILE_TEMP, USER_FILE);

    if (deleted) {
        log_message(LOG_INFO, "Usuario ID %d removido do CSV com sucesso.", user_id);
        update_cached_user_count(-1);

        // Remove todos os prontuários associados a este usuário
        delete_clinical_records_by_patient(user_id);
    }

    return deleted;
}

int delete_clinical_records_by_patient(int patient_id) {
    FILE *cl_src = fopen(CLINICAL_FILE, "r");
    if (cl_src == NULL) {
        return 0;
    }

    FILE *cl_dest = fopen(CLINICAL_FILE_TEMP, "w");
    if (cl_dest == NULL) {
        fclose(cl_src);
        return 0;
    }

    char cl_line[512];
    char cl_line_copy[512];
    char *cl_fields[7];
    int cl_deleted_count = 0;

    while (fgets(cl_line, sizeof(cl_line), cl_src)) {
        if (cl_line[0] == '\n' || cl_line[0] == '\r') {
            fprintf(cl_dest, "%s", cl_line);
            continue;
        }

        strcpy(cl_line_copy, cl_line);
        int cl_cols = split_csv_line(cl_line_copy, cl_fields, 7);

        if (cl_cols >= 7 && atoi(cl_fields[1]) == patient_id) {
            cl_deleted_count++;
        } else {
            fprintf(cl_dest, "%s", cl_line);
        }
    }
    
    fclose(cl_src);
    fclose(cl_dest);
    
    remove(CLINICAL_FILE);
    rename(CLINICAL_FILE_TEMP, CLINICAL_FILE);
    
    if (cl_deleted_count > 0) {
        log_message(LOG_INFO, "%d prontuarios vinculados ao paciente ID %d foram removidos de %s.", cl_deleted_count, patient_id, CLINICAL_FILE);
    }
    
    return cl_deleted_count;
}

int save_clinical_record(ClinicalRecord *record) {
    database_init();

    User patient = find_user_by_id(record->patient_id);
    if (patient.id == -1) {
        log_message(LOG_ERROR, "Impossivel criar prontuario: Paciente ID %d nao existe.", record->patient_id);
        return 0;
    }

    FILE *file = fopen(CLINICAL_FILE, "a");
    if (file == NULL) {
        log_message(LOG_ERROR, "Nao foi possivel abrir o arquivo %s para gravar prontuario.", CLINICAL_FILE);
        return 0;
    }

    // Formato CSV do prontuário: id;patient_id;data;altura;peso;diagnostico;recomendacao
    fprintf(file, "%d;%d;%s;%.2f;%.2f;%s;%s\n", 
            record->id, 
            record->patient_id, 
            record->diag_date, 
            record->collected_metrics.height, 
            record->collected_metrics.weight, 
            record->diagnosis); 
            //record->recommendation); // Não vai precisar de recomendação 

    fclose(file);
    log_message(LOG_INFO, "Prontuario ID %d adicionado no CSV para Paciente ID %d.", record->id, record->patient_id);
    return 1;
}

ClinicalRecord* load_clinical_records(int patient_id, int *total_count) {
    *total_count = 0;
    FILE *file = fopen(CLINICAL_FILE, "r");
    if (file == NULL) {
        return NULL;
    }

    char line[512];
    char *fields[7];
    int count = 0;

    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '\n' || line[0] == '\r') continue;
        char line_copy[512];
        strcpy(line_copy, line);
        int cols = split_csv_line(line_copy, fields, 7);
        if (cols >= 7 && atoi(fields[1]) == patient_id) {
            count++;
        }
    }

    if (count == 0) {
        fclose(file);
        return NULL;
    }

    ClinicalRecord *records = malloc(count * sizeof(ClinicalRecord));
    if (records == NULL) {
        fclose(file);
        return NULL;
    }

    fseek(file, 0, SEEK_SET);
    int current = 0;

    while (fgets(line, sizeof(line), file) && current < count) {
        if (line[0] == '\n' || line[0] == '\r') continue;

        char line_copy[512];
        strcpy(line_copy, line);

        int cols = split_csv_line(line_copy, fields, 7);
        if (cols < 7) continue;

        if (atoi(fields[1]) == patient_id) {
            records[current].id = atoi(fields[0]);
            records[current].patient_id = atoi(fields[1]);
            strcpy(records[current].diag_date, fields[2]);
            records[current].collected_metrics.height = atof(fields[3]);
            records[current].collected_metrics.weight = atof(fields[4]);
            strcpy(records[current].diagnosis, fields[5]);
            // strcpy(records[current].recommendation, fields[6]); Não vamos precisar de recomendação
            current++;
        }
    }

    fclose(file);
    *total_count = current;
    return records;
}

User* get_all_users(int *total_count) {
    *total_count = 0;

    int count = get_cached_user_count();
    if (count == 0) {
        return NULL;
    }

    FILE *file = fopen(USER_FILE, "r");
    if (file == NULL) {
        return NULL; 
    }

    User *users = malloc(count * sizeof(User));
    if (users == NULL) {
        fclose(file);
        return NULL;
    }

    char line[512];
    char *fields[7];
    int current = 0;

    while (fgets(line, sizeof(line), file) && current < count) {
        if (line[0] == '\n' || line[0] == '\r') continue;

        char line_copy[512];
        strcpy(line_copy, line);

        int cols = split_csv_line(line_copy, fields, 7);
        if (cols < 7) continue;

        users[current].id = atoi(fields[0]);
        strcpy(users[current].name, fields[1]);
        strcpy(users[current].email, fields[2]);
        strcpy(users[current].cpf, fields[3]);
        strcpy(users[current].birth_date, fields[4]);
        users[current].metrics.height = atof(fields[5]);
        users[current].metrics.weight = atof(fields[6]);
        current++;
    }

    fclose(file);
    *total_count = current;
    return users;
}