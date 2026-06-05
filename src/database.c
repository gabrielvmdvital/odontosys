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

    FILE *fp = fopen(PATIENT_FILE, "r");
    if (!fp) {
        fp = fopen(PATIENT_FILE, "w");
        if (fp) {
            fprintf(fp, "id;nome;email;cpf;data_nascimento;altura;peso\n");
            fclose(fp);
        }
    } else {
        fclose(fp);
    }

    FILE *fc = fopen(CLINICAL_FILE, "r");
    if (!fc) {
        fc = fopen(CLINICAL_FILE, "w");
        if (fc) {
            fprintf(fc, "id;patient_id;diag_date;altura;peso;idade;anb;coa;co_gn;afai;sn_go_gn;na1_dist;na1_ang;na2_dist;na2_ang;perf_tegument;pre_diagnostico\n");
            fclose(fc);
        }
    } else {
        fclose(fc);
    }

    log_message(LOG_INFO, "Banco de dados inicializado.");
}

int get_cached_patient_count(void) {
    FILE *f = fopen(PATIENT_COUNT_FILE, "r");
    if (!f) return 0;
    int count = 0;
    fscanf(f, "%d", &count);
    fclose(f);
    return count;
}

void update_cached_patient_count(int delta) {
    int count = get_cached_patient_count();
    count += delta;
    if (count < 0) count = 0;
    FILE *f = fopen(PATIENT_COUNT_FILE, "w");
    if (f) {
        fprintf(f, "%d", count);
        fclose(f);
    }
}

int save_patient(Patient *patient) {
    database_init();

    Patient existing = find_patient_by_cpf(patient->cpf);
    if (existing.id != -1) {
        log_message(LOG_WARNING, "Falha ao salvar: ja existe um paciente com o CPF %s.", patient->cpf);
        return 0;
    }

    FILE *file = fopen(PATIENT_FILE, "a");
    if (file == NULL) {
        log_message(LOG_ERROR, "Nao foi possivel abrir o arquivo %s para salvar o paciente.", PATIENT_FILE);
        return 0;
    }

    // Formato CSV: id;nome;email;cpf;data_nascimento;altura;peso
    fprintf(file, "%d;%s;%s;%s;%s;%.2f;%.2f\n", 
            patient->id, 
            patient->name, 
            patient->email, 
            patient->cpf, 
            patient->birth_date,
            patient->metrics.height, 
            patient->metrics.weight);
    fclose(file);
    log_message(LOG_INFO, "Paciente cadastrado no CSV com sucesso: %s (ID: %d)", patient->name, patient->id);
    update_cached_patient_count(1);
    return 1;
}

Patient find_patient_by_cpf(const char *cpf) {
    Patient found;
    memset(&found, 0, sizeof(Patient));
    found.id = -1;

    FILE *file = fopen(PATIENT_FILE, "r");
    if (file == NULL) {
        return found;
    }

    char line[512];
    char *fields[7];
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '\n' || line[0] == '\r') continue;
        if (strncmp(line, "id;", 3) == 0) continue;
        if (strncmp(line, "id;", 3) == 0) continue;
        if (strncmp(line, "id;", 3) == 0) continue;

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

Patient find_patient_by_id(int patient_id) {
    Patient found;
    found.id = -1;

    FILE *file = fopen(PATIENT_FILE, "r");
    if (file == NULL) {
        return found;
    }

    char line[512];
    char *fields[7];
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '\n' || line[0] == '\r') continue;
        if (strncmp(line, "id;", 3) == 0) continue;
        if (strncmp(line, "id;", 3) == 0) continue;
        if (strncmp(line, "id;", 3) == 0) continue;

        char line_copy[512];
        strcpy(line_copy, line);

        int cols = split_csv_line(line_copy, fields, 7);
        if (cols < 7) continue;

        if (atoi(fields[0]) == patient_id) {
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

int update_patient(int patient_id, Patient *patient) {

    Patient existing = find_patient_by_cpf(patient->cpf);
    if (existing.id != -1 && existing.id != patient_id) {
        log_message(LOG_WARNING, "Falha ao atualizar: ja existe outro paciente com o CPF %s.", patient->cpf);
        return 0;
    }

    FILE *src = fopen(PATIENT_FILE, "r");
    if (src == NULL) {
        log_message(LOG_WARNING, "Tentativa de atualizar paciente ID %d, mas %s nao existe.", patient_id, PATIENT_FILE);
        return 0;
    }

    FILE *dest = fopen("database/pacientes.tmp", "w");
    if (dest == NULL) {
        log_message(LOG_ERROR, "Nao foi possivel criar o arquivo temporario database/pacientes.tmp para atualizacao.");
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

        if (cols >= 7 && atoi(fields[0]) == patient_id) {
            fprintf(dest, "%d;%s;%s;%s;%s;%.2f;%.2f\n", 
                    patient->id, 
                    patient->name, 
                    patient->email, 
                    patient->cpf, 
                    patient->birth_date,
                    patient->metrics.height, 
                    patient->metrics.weight);
            updated = 1;
        } else {
            fprintf(dest, "%s", line);
        }
    }

    fclose(src);
    fclose(dest);

    remove(PATIENT_FILE);
    rename("database/pacientes.tmp", PATIENT_FILE);

    if (updated) {
        log_message(LOG_INFO, "Paciente ID %d (%s) atualizado com sucesso no CSV.", patient_id, patient->name);
    } else {
        log_message(LOG_WARNING, "Nenhum paciente correspondente ao ID %d encontrado para atualizacao no CSV.", patient_id);
    }

    return updated;
}

int delete_patient(int patient_id) {
    FILE *src = fopen(PATIENT_FILE, "r");
    if (src == NULL) {
        return 0;
    }

    FILE *dest = fopen(PATIENT_FILE_TEMP, "w");
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

        if (cols >= 7 && atoi(fields[0]) == patient_id) {
            deleted = 1;
        } else {
            fprintf(dest, "%s", line);
        }
    }

    fclose(src);
    fclose(dest);

    remove(PATIENT_FILE);
    rename(PATIENT_FILE_TEMP, PATIENT_FILE);

    if (deleted) {
        log_message(LOG_INFO, "Paciente ID %d removido do CSV com sucesso.", patient_id);
        update_cached_patient_count(-1);

        // Remove todos os prontuários associados a este usuário
        delete_clinical_records_by_patient(patient_id);
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
    char *cl_fields[17];
    int cl_deleted_count = 0;

    while (fgets(cl_line, sizeof(cl_line), cl_src)) {
        if (cl_line[0] == '\n' || cl_line[0] == '\r') {
            fprintf(cl_dest, "%s", cl_line);
            continue;
        }

        strcpy(cl_line_copy, cl_line);
        int cl_cols = split_csv_line(cl_line_copy, cl_fields, 17);

        if (cl_cols >= 17 && atoi(cl_fields[1]) == patient_id) {
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

    Patient patient = find_patient_by_id(record->patient_id);
    if (patient.id == -1) {
        log_message(LOG_ERROR, "Impossivel criar prontuario: Paciente ID %d nao existe.", record->patient_id);
        return 0;
    }

    FILE *file = fopen(CLINICAL_FILE, "a");
    if (file == NULL) {
        log_message(LOG_ERROR, "Nao foi possivel abrir o arquivo %s para gravar prontuario.", CLINICAL_FILE);
        return 0;
    }

    // Formato CSV do prontuário: id;patient_id;data;altura;peso;idade;anb;coa;co_gn;afai;sn_go_gn;na1_dist;na1_ang;na2_dist;na2_ang;perf_tegument;pre_diagnostico
    fprintf(file, "%d;%d;%s;%.2f;%.2f;%d;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f;%s;%s\n", 
        record->id, 
        record->patient_id, 
        record->diag_date, 
        record->collected_metrics.height, 
        record->collected_metrics.weight, 
        record->collected_metrics.age,
        record->anb,
        record->coa,
        record->co_gn,
        record->afai,
        record->sn_go_gn,
        record->na1_dist,
        record->na1_ang,
        record->na2_dist,
        record->na2_ang,
        record->perf_tegument,
        record->pre_diagnosis
    );

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
    char *fields[17];
    int count = 0;

    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '\n' || line[0] == '\r') continue;
        if (strncmp(line, "id;", 3) == 0) continue;
        if (strncmp(line, "id;", 3) == 0) continue;
        if (strncmp(line, "id;", 3) == 0) continue;
        char line_copy[512];
        strcpy(line_copy, line);
        int cols = split_csv_line(line_copy, fields, 17);
        if (cols >= 17 && atoi(fields[1]) == patient_id) {
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
        if (strncmp(line, "id;", 3) == 0) continue;

        char line_copy[512];
        strcpy(line_copy, line);

        int cols = split_csv_line(line_copy, fields, 17);
        if (cols < 17) continue;

        if (atoi(fields[1]) == patient_id) {
            records[current].id = atoi(fields[0]);
            records[current].patient_id = atoi(fields[1]);
            strcpy(records[current].diag_date, fields[2]);
            records[current].collected_metrics.height = atof(fields[3]);
            records[current].collected_metrics.weight = atof(fields[4]);
            records[current].collected_metrics.age = atoi(fields[5]);
            records[current].anb = atof(fields[6]);
            records[current].coa = atof(fields[7]);
            records[current].co_gn = atof(fields[8]);
            records[current].afai = atof(fields[9]);
            records[current].sn_go_gn = atof(fields[10]);
            records[current].na1_dist = atof(fields[11]);
            records[current].na1_ang = atof(fields[12]);
            records[current].na2_dist = atof(fields[13]);
            records[current].na2_ang = atof(fields[14]);
            strcpy(records[current].perf_tegument, fields[15]);
            strcpy(records[current].pre_diagnosis, fields[16]);
            current++;
        }
    }

    fclose(file);
    *total_count = current;
    return records;
}

Patient* get_all_patients(int *total_count) {
    *total_count = 0;

    int count = get_cached_patient_count();
    if (count == 0) {
        return NULL;
    }

    FILE *file = fopen(PATIENT_FILE, "r");
    if (file == NULL) {
        return NULL; 
    }

    Patient *patients = malloc(count * sizeof(Patient));
    if (patients == NULL) {
        fclose(file);
        return NULL;
    }

    char line[512];
    char *fields[7];
    int current = 0;

    while (fgets(line, sizeof(line), file) && current < count) {
        if (line[0] == '\n' || line[0] == '\r') continue;
        if (strncmp(line, "id;", 3) == 0) continue;

        char line_copy[512];
        strcpy(line_copy, line);

        int cols = split_csv_line(line_copy, fields, 7);
        if (cols < 7) continue;

        patients[current].id = atoi(fields[0]);
        strcpy(patients[current].name, fields[1]);
        strcpy(patients[current].email, fields[2]);
        strcpy(patients[current].cpf, fields[3]);
        strcpy(patients[current].birth_date, fields[4]);
        patients[current].metrics.height = atof(fields[5]);
        patients[current].metrics.weight = atof(fields[6]);
        current++;
    }

    fclose(file);
    *total_count = current;
    return patients;
}