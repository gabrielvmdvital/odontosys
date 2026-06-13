#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "clinical.h"
#include "database.h"
#include "logs.h"

// REGRAS DE CLASSIFICAÇÃO ORTODÔNTICA
// 1) ANB (Classe esquelética): 1-4 = Classe I | >4 = Classe II | <1 = Classe III
// 2) CoA + CoGn: Comparação com tabela de McNamara -> Mandíbula reduzida / normal / aumentada
// 3) AFAI (Padrão crescimento facial): aumentada = vertical | normal = equilibrado | diminuída = horizontal
// 4) 1-NA (Posição incisivo sup): 3-5 = normal | >5 = protruído | <3 = retruído
// 5) 1.NA (Inclinação incisivo sup): 24-25 = normal | >25 = inclinado | <23 = verticalizado
// 6) 1-NB (Posição incisivo inf): 3-5 = normal | >5 = protruído | <3 = retruído
// 7) 1.NB (Inclinação incisivo inf): 24-26 = normal | >26 = inclinado | <24 = verticalizado
// 8) SNGoGn (Padrão crescimento): aumentado = vertical | normal = equilibrado | diminuído = horizontal (!! Verificar como utilizar esse !!)
// 9) Perf_tegument (Perfil facial): reto | convexo | concavo | normal

/* Exemplo de Pré-Diagnóstico:
        Paciente Classe I esquelética, com mandíbula reduzida, padrão de crescimento facial vertical,
        incisivos superiores retruídos e verticalizados, incisivos inferiores protruídos e inclinados,
        e perfil facial côncavo.
*/

void clinical_formular_diag(ClinicalRecord *record) {

// 1) Classe esquelética (variável utilizada: anb)
char str_classe[15];
if (1 <= record->anb <= 4)       // OBS.: Usar 'record->anb' é o mesmo que '(*record).anb'
        strcpy(str_classe, "Classe I"); 

else if (record->anb > 4)
        strcpy(str_classe, "Classe II");

else
        strcpy(str_classe, "Classe III");



// 4) Posição incisivo sup (variável: na1_dist)
char str_pos_incsup[20];
if (3 <= record->na1_dist <= 5)
        strcpy(str_pos_incsup, "normal");

else if (record->na1_dist > 5)
        strcpy(str_pos_incsup, "protruido");

else 
        strcpy(str_pos_incsup, "retruido");


// receber medida coa e corrigir se tiver desvio
int coa_corrigido;
//maxila normal:
coa_corrigido = (int)record->coa;

//maxilaprotuida:
if (record->maxila_tipo == 1)
        coa_corrigido = coa_corrigido - record->maxila_desvio;

//maxila retraida:
else if (record->maxila_tipo == -1)
        coa_corrigido = coa_corrigido + record->maxila_desvio;


// No fim, juntar tudo em uma string para copiar em pre_diagnosis.
sprintf(record->pre_diagnosis, "Paciente %s, ...", str_classe);
}

int save_clinical_record(ClinicalRecord *record) {
    database_init();

    // Importante verificar paciente, se a regra de negocio exige...
    record->clinical_id = generate_unique_id();

    char line[1024];
    snprintf(line, sizeof(line), "%" PRIu64 ";%" PRIu64 ";%" PRIu64 ";%s;%d;%.2f;%.2f;%d;%d;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f;%s;%s\n", 
        record->clinical_id, 
        record->patient_id, 
        record->dentist_id,
        record->diag_date, 
        record->age,
        record->anb,
        record->coa,
        record->maxila_tipo,
        record->maxila_desvio,
        record->cogn,
        record->afai,
        record->sngogn,
        record->na1_dist,
        record->na1_ang,
        record->na2_dist,
        record->na2_ang,
        record->perf_tegument,
        record->pre_diagnosis
    );

    if (db_append_line(CLINICAL_FILE, line)) {
        log_message(LOG_INFO, "Prontuario ID %" PRIu64 " adicionado no CSV para Paciente ID %" PRIu64 ".", 
                    record->clinical_id, record->patient_id);
        return 1;
    }
    return 0;
}

ClinicalRecord* load_clinical_records(uint64_t patient_id, int *count) {
    *count = 0;
    FILE *file = fopen(CLINICAL_FILE, "r");
    if (file == NULL) return NULL;

    char line[1024];
    char *fields[18];
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '\n' || line[0] == '\r') continue;
        if (strncmp(line, "clinical_id;", 12) == 0 || strncmp(line, "id;", 3) == 0) continue;
        
        char line_copy[1024];
        strcpy(line_copy, line);
        int cols = split_csv_line(line_copy, fields, 18);
        if (cols >= 18 && strtoull(fields[1], NULL, 10) == patient_id) {
            (*count)++;
        }
    }
    fclose(file);

    if (*count == 0) return NULL;

    ClinicalRecord *records = malloc((*count) * sizeof(ClinicalRecord));
    if (records == NULL) return NULL;

    file = fopen(CLINICAL_FILE, "r");
    if (file == NULL) {
        free(records);
        return NULL;
    }

    int current = 0;
    while (fgets(line, sizeof(line), file) && current < *count) {
        if (line[0] == '\n' || line[0] == '\r') continue;
        if (strncmp(line, "clinical_id;", 12) == 0 || strncmp(line, "id;", 3) == 0) continue;

        char line_copy[1024];
        strcpy(line_copy, line);

        int cols = split_csv_line(line_copy, fields, 18);
        if (cols < 18) continue;

        if (strtoull(fields[1], NULL, 10) == patient_id) {
            records[current].clinical_id = strtoull(fields[0], NULL, 10);
            records[current].patient_id = strtoull(fields[1], NULL, 10);
            records[current].dentist_id = strtoull(fields[2], NULL, 10);
            strcpy(records[current].diag_date, fields[3]);
            records[current].age = atoi(fields[4]);
            records[current].anb = atof(fields[5]);
            records[current].coa = atof(fields[6]);
            records[current].maxila_tipo = atoi(fields[7]);
            records[current].maxila_desvio = atoi(fields[8]);
            records[current].cogn = atof(fields[9]);
            records[current].afai = atof(fields[10]);
            records[current].sngogn = atof(fields[11]);
            records[current].na1_dist = atof(fields[12]);
            records[current].na1_ang = atof(fields[13]);
            records[current].na2_dist = atof(fields[14]);
            records[current].na2_ang = atof(fields[15]);
            strcpy(records[current].perf_tegument, fields[16]);
            strcpy(records[current].pre_diagnosis, fields[17]);
            current++;
        }
    }

    fclose(file);
    *count = current;
    return records;
}

int delete_clinical_records_by_patient(uint64_t patient_id) {
    char filter_val[32];
    snprintf(filter_val, sizeof(filter_val), "%" PRIu64, patient_id);
    
    int cl_deleted_count = db_delete_lines(CLINICAL_FILE, CLINICAL_FILE_TEMP, 1, filter_val, 18);
    
    if (cl_deleted_count > 0) {
        log_message(LOG_INFO, "%d prontuarios vinculados ao paciente ID %" PRIu64 " foram removidos de %s.", cl_deleted_count, patient_id, CLINICAL_FILE);
    }
    
    return cl_deleted_count;
}


