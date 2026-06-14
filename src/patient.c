#include "patient.h"
#include "database.h"
#include "clinical.h" // For delete_clinical_records_by_patient
#include "logs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

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
    if (existing.patient_id != (uint64_t)-1) {
        log_message(LOG_WARNING, "Falha ao salvar: ja existe um paciente com o CPF %s.", patient->cpf);
        return 0;
    }

    patient->patient_id = generate_unique_id();

    char line[512];
    snprintf(line, sizeof(line), "%" PRIu64 ";%" PRIu64 ";%s;%s;%s;%s;%s\n", 
             patient->patient_id, patient->dentist_id, patient->name, patient->email, 
             patient->cpf, patient->birth_date, patient->phone);

    if (db_append_line(PATIENT_FILE, line)) {
        log_message(LOG_INFO, "Paciente cadastrado no CSV com sucesso: %s (ID: %" PRIu64 ")", patient->name, patient->patient_id);
        update_cached_patient_count(1);
        return 1;
    }
    return 0;
}

Patient find_patient_by_cpf(const char *cpf) {
    Patient found;
    memset(&found, 0, sizeof(Patient));
    found.patient_id = -1;

    FILE *file = fopen(PATIENT_FILE, "r");
    if (file == NULL) return found;

    char line[512];
    char *fields[6];
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '\n' || line[0] == '\r') continue;
        if (strncmp(line, "patient_id;", 11) == 0 || strncmp(line, "id;", 3) == 0) continue;

        char line_copy[512];
        strcpy(line_copy, line);
        int cols = split_csv_line(line_copy, fields, 7);
        if (cols < 7) continue;

        if (strcmp(fields[4], cpf) == 0) {
            found.patient_id = strtoull(fields[0], NULL, 10);
            found.dentist_id = strtoull(fields[1], NULL, 10);
            strcpy(found.name, fields[2]);
            strcpy(found.email, fields[3]);
            strcpy(found.cpf, fields[4]);
            strcpy(found.birth_date, fields[5]);
            strcpy(found.phone, fields[6]);
            break;
        }
    }
    fclose(file);
    return found;
}

Patient find_patient_by_id(uint64_t patient_id) {
    Patient found;
    memset(&found, 0, sizeof(Patient));
    found.patient_id = -1; // -1 on a uint64_t makes it max value, checking found.patient_id == (uint64_t)-1 is valid

    FILE *file = fopen(PATIENT_FILE, "r");
    if (file == NULL) return found;

    char line[512];
    char *fields[6];
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '\n' || line[0] == '\r') continue;
        if (strncmp(line, "patient_id;", 11) == 0 || strncmp(line, "id;", 3) == 0) continue;

        char line_copy[512];
        strcpy(line_copy, line);
        int cols = split_csv_line(line_copy, fields, 7);
        if (cols < 7) continue;

        if (strtoull(fields[0], NULL, 10) == patient_id) {
            found.patient_id = strtoull(fields[0], NULL, 10);
            found.dentist_id = strtoull(fields[1], NULL, 10);
            strcpy(found.name, fields[2]);
            strcpy(found.email, fields[3]);
            strcpy(found.cpf, fields[4]);
            strcpy(found.birth_date, fields[5]);
            strcpy(found.phone, fields[6]);
            break;
        }
    }
    fclose(file);
    return found;
}

int update_patient(uint64_t patient_id, Patient *patient) {
    Patient existing = find_patient_by_cpf(patient->cpf);
    if (existing.patient_id != (uint64_t)-1 && (uint64_t)existing.patient_id != patient_id) {
        log_message(LOG_WARNING, "Falha ao atualizar: ja existe outro paciente com o CPF %s.", patient->cpf);
        return 0;
    }

    char filter_val[32];
    snprintf(filter_val, sizeof(filter_val), "%" PRIu64, patient_id);

    char new_line[512];
    snprintf(new_line, sizeof(new_line), "%" PRIu64 ";%" PRIu64 ";%s;%s;%s;%s;%s\n", 
             patient->patient_id, patient->dentist_id, patient->name, patient->email, 
             patient->cpf, patient->birth_date, patient->phone);

    int updated = db_update_line(PATIENT_FILE, PATIENT_FILE_TEMP, 0, filter_val, new_line, 7);

    if (updated) {
        log_message(LOG_INFO, "Paciente ID %" PRIu64 " (%s) atualizado com sucesso no CSV.", patient_id, patient->name);
    } else {
        log_message(LOG_WARNING, "Nenhum paciente correspondente ao ID %" PRIu64 " encontrado para atualizacao no CSV.", patient_id);
    }

    return updated;
}

int delete_patient(uint64_t patient_id) {
    char filter_val[32];
    snprintf(filter_val, sizeof(filter_val), "%" PRIu64, patient_id);

    int deleted = db_delete_lines(PATIENT_FILE, PATIENT_FILE_TEMP, 0, filter_val, 7);

    if (deleted > 0) {
        log_message(LOG_INFO, "Paciente ID %" PRIu64 " removido do CSV com sucesso.", patient_id);
        update_cached_patient_count(-1);
        delete_clinical_records_by_patient(patient_id); // Deleta em cascata
        return 1;
    }
    return 0;
}

Patient* get_all_patients(int *total_count) {
    *total_count = 0;
    int count = get_cached_patient_count();
    if (count == 0) return NULL;

    FILE *file = fopen(PATIENT_FILE, "r");
    if (file == NULL) return NULL;

    Patient *patients = malloc(count * sizeof(Patient));
    if (patients == NULL) {
        fclose(file);
        return NULL;
    }

    char line[512];
    char *fields[6];
    int current = 0;

    while (fgets(line, sizeof(line), file) && current < count) {
        if (line[0] == '\n' || line[0] == '\r') continue;
        if (strncmp(line, "patient_id;", 11) == 0 || strncmp(line, "id;", 3) == 0) continue;

        char line_copy[512];
        strcpy(line_copy, line);
        int cols = split_csv_line(line_copy, fields, 7);
        if (cols < 7) continue;

        patients[current].patient_id = strtoull(fields[0], NULL, 10);
        patients[current].dentist_id = strtoull(fields[1], NULL, 10);
        strcpy(patients[current].name, fields[2]);
        strcpy(patients[current].email, fields[3]);
        strcpy(patients[current].cpf, fields[4]);
        strcpy(patients[current].birth_date, fields[5]);
        strcpy(patients[current].phone, fields[6]);
        current++;
    }

    fclose(file);
    *total_count = current;
    return patients;
}
