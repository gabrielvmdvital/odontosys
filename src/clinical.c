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
// 3) Padrão crescimento facial: vertical | equilibrado | horizontal
// Onde: AFAI (mm) -> aumentado | normal | reduzido; e SNGoGn (graus) -> aumentado | normal | reduzido
/*      MAPA DE CRUZA-DADOS (Padrão de Crescimento Facial) gerado com auxílio de IA:
        ==============================================================================
        *   AFAI (McNamara)  |   SNGoGn (Ângulo)   |  PADRÃO DE CRESCIMENTO (Veredito)
        * -------------------|---------------------|----------------------------------
        *   aumentada        |   aumentado         |  "vertical"    (Concordância +)
        *   normal           |   aumentado         |  "vertical"    (Tendência Óssea)
        *   aumentada        |   normal            |  "vertical"    (Tendência Linear)
        * -------------------|---------------------|----------------------------------
        *   diminuida        |   diminuido         |  "horizontal"  (Concordância -)
        *   normal           |   diminuido         |  "horizontal"  (Tendência Óssea)
        *   diminuida        |   normal            |  "horizontal"  (Tendência Linear)
        * -------------------|---------------------|----------------------------------
        *   normal           |   normal            |  "equilibrado" (Harmonia Total)
        ==============================================================================
*/
// 4) 1-NA (Posição incisivo sup): 3-5 = normal | >5 = protruído | <3 = retruído
// 5) 1.NA (Inclinação incisivo sup): 24-25 = normal | >25 = inclinado | <23 = verticalizado
// 6) 1-NB (Posição incisivo inf): 3-5 = normal | >5 = protruído | <3 = retruído
// 7) 1.NB (Inclinação incisivo inf): 24-26 = normal | >26 = inclinado | <24 = verticalizado
// 8) Perf_tegument (Perfil facial): reto | convexo | concavo | normal

/* Exemplo de Pré-Diagnóstico:
        Paciente Classe I esquelética, com mandíbula reduzida, padrão de crescimento facial vertical,
        incisivos superiores retruídos e verticalizados, incisivos inferiores protruídos e inclinados,
        e perfil facial côncavo.
*/

void clinical_formular_diag(ClinicalRecord *record) {

// 1) Classe esquelética (variável utilizada: anb)
        char str_classe[15];
        if (1 <= record->anb && record->anb <= 4)   // OBS.: Usar 'record->anb' é o mesmo que '(*record).anb'
                strcpy(str_classe, "Classe I");

        else if (record->anb > 4)
                strcpy(str_classe, "Classe II");

        else
                strcpy(str_classe, "Classe III");

// 2) Classificação Maxila (maxila_tipo)
        char str_maxila[50];

        if (record->maxila_tipo == 0)
                strcpy(str_maxila, "bem posicionada");

        else if (record->maxila_tipo == 1)
                strcpy(str_maxila, "protruida");

        else if (record->maxila_tipo == -1)
                strcpy(str_maxila, "retruida");

// 3) Tamanho da Mandíbula (coa, maxila_tipo, maxila_desvio + cogn + Tabela de McNamara)
        char str_tam_mand[50];
        // Receber Coa e corrigir proporcinalmente ao desvio da mandibula
        int coa_corrigido;

        // Maxila bem posicionada/normal (Desvio 0)
        coa_corrigido = (int)record->coa;

        // Maxila Protuida (Diminuir os mm extra):
        if (record->maxila_tipo == 1)
                coa_corrigido = coa_corrigido - record->maxila_desvio;

        // Maxila Retruida (Acrescentar os mm extra):
        else if (record->maxila_tipo == -1)
                coa_corrigido = coa_corrigido + record->maxila_desvio;

        // Tabela de McNamara
        int mcNam[29][5] = {
//               0          1    2          3   4
                {80,        97,  100,       57, 58},    // Coluna de id 0: Comprimento da maxila(mm) -> baseado no 'coa_corrigido'
                {81,        99,  102,       57, 58},    // Coluna de id 1: Comprimento mandibular(mm) [limite inferior]
                {82,        101, 104,       58, 59},    // Coluna de id 2: Comprimento mandibular(mm) [limite superior]
                {83,        103, 106,       58, 59},    // Coluna de id 3: Alt Facial inferior(mm) [limite inferior]
                {84,        104, 107,       59, 60},    // Coluna de id 4: Alt Facial inferior(mm) [limite superior]
                {85,        105, 108,       60, 62},
                {86,        107, 110,       60, 62},
                {87,        109, 112,       61, 63},
                {88,        111, 114,       61, 63},
                {89,        112, 115,       62, 64},
                {90,        113, 116,       63, 64},
                {91,        115, 118,       63, 64},
                {92,        117, 120,       64, 65},
                {93,        119, 122,       65, 66},
                {94,        121, 124,       66, 67},
                {95,        122, 125,       67, 69},
                {96,        124, 127,       67, 69},
                {97,        126, 129,       68, 70},
                {98,        128, 131,       68, 70},
                {99,        129, 132,       69, 71},
                {100,       130, 133,       70, 74},
                {101,       132, 135,       71, 75},
                {102,       134, 137,       72, 76},
                {103,       136, 139,       73, 77},
                {104,       137, 140,       74, 78},
                {105,       138, 141,       75, 79},
                {106,       139, 142,       76, 80},
                {107,       140, 143,       77, 81},
                {108,       141, 144,       78, 82}
        };

        // Iterar por cada linha para comparar os valores
        for (int i = 0; i<29; i++) {
                if (coa_corrigido == mcNam[i][0]) {
                        // Mandíbula Normal
                        if (mcNam[i][1] <= record->cogn && record->cogn <= mcNam[i][2]) // cogn do paciente entre os valores ideais
                                strcpy(str_tam_mand, "normal");

                        // Mandíbula Aumentada
                        else if (mcNam[i][2] < record->cogn) // cogn do paciente acima dos valores ideais
                                strcpy(str_tam_mand, "aumentada");
                        
                        // Mandíbula Reduzida
                        else if (record->cogn < mcNam[i][1]) // cogn do paciente abaixo dos valores ideais
                                strcpy(str_tam_mand, "reduzida");
                }
        }

// 4) Padrão de Crescimento Facial (afai, coa_corrigido + Tabela de McNamamra)
        char str_cresc_fac[50];

        // Será dividido em duas partes:
        char str_afai[50], str_sngogn[50];

        // AFAI, pela tabela de McNamara (assim como foi usada para o tamanho da mandíbula)
        for (int i = 0; i<29; i++) {
                if (coa_corrigido == mcNam[i][0]) {
                        // AFAI Normal
                        if (mcNam[i][3] <= record->afai && record->afai <= mcNam[i][4]) // afai do paciente entre os valores ideais
                                strcpy(str_afai, "normal");

                        // AFAI Aumentado
                        else if (mcNam[i][4] < record->afai) // afai do paciente acima dos valores ideais
                                strcpy(str_afai, "aumentado");
                        
                        // AFAI Reduzido
                        else if (record->afai < mcNam[i][3]) // afai do paciente abaixo dos valores ideais
                                strcpy(str_afai, "diminuido");
                }
        }

        // SNGoGn, baseado no Padrão USP/PROFIS
        if (record->sngogn > 36.1)
                strcpy(str_sngogn, "aumentado");

        else if (record->sngogn < 26.9)
                strcpy(str_sngogn, "diminuido");

        else
                strcpy(str_sngogn, "normal");

        // Análise Final do Padrão de crescimento (salvando em str_cresc_fac)
        // Se uma das variáveis é aumentada
        if (strcmp(str_afai, "aumentado") == 0 || strcmp(str_sngogn, "aumentado") == 0) // Se um OU (||) o outro acontecer
                strcpy(str_cresc_fac, "vertical");
        
        // Se uma das variáveis diminuída
        else if (strcmp(str_afai, "diminuido") == 0 || strcmp(str_sngogn, "diminuido") == 0)
                strcpy(str_cresc_fac, "horizontal");

        // O que sobra: ambas normais
        else
                strcpy(str_cresc_fac, "equilibrado");


// 5) Posição incisivo sup (variável: na1_dist)
        char str_pos_incsup[50];

        if (record->na1_dist >= 3 && record->na1_dist <= 5)
                strcpy(str_pos_incsup, "normal");

        else if (record->na1_dist > 5)
                strcpy(str_pos_incsup, "protruido");

        else 
                strcpy(str_pos_incsup, "retruido");

// 6) Inclinação incisivo sup (medida angular: na1_ang)
        char str_inc_incsup[30];

        //1.NA normal 23-25 graus
        if (record->na1_ang >= 23.0 && record->na1_ang <= 25.0)
                strcpy(str_inc_incsup, "inclinacao normal");

        else if (record->na1_ang > 25.0)
                strcpy(str_inc_incsup, "inclinado para vestibular");
                
        else
                strcpy(str_inc_incsup, "verticalizado");


// 7) Posição incisivo inf (nb1_dist)
        char str_pos_incinf[50];

        if (record->nb1_dist >= 3.0 && record->nb1_dist <= 5.0)
                strcpy(str_pos_incinf, "bem posicionado");

        else if (record->nb1_dist < 3.0)
                strcpy(str_pos_incinf, "retruido");

        else
                strcpy(str_pos_incinf, "protruido");

// 8) Inclinação incisivo inf (nb1_ang)
        char str_inc_incinf[50];

        // 1.NB normal = 34-26 graus
        if (record->nb1_ang >= 24.0 && record->nb1_ang <= 26.0)
                strcpy(str_inc_incinf, "com boa inclinação");

        else if (record->nb1_ang < 24.0)
                strcpy(str_inc_incinf, "verticalizados");

        else
                strcpy(str_inc_incinf, "inclinado para vestibular");

// 9) Perfil facial (perf_tegument)
        char str_perf_fac[50];
        strcpy(str_perf_fac, record->perf_tegument);


// No fim, juntar tudo em uma string para copiar em pre_diagnosis.
sprintf(record->pre_diagnosis, "Paciente %s, com maxila %s, com mandibula %s, padrao de crescimento facial %s, incisivos superiores %s e %s, incisivos inferiores %s e %s, e perfil facial %s.", str_classe, str_maxila, str_tam_mand, str_cresc_fac, str_pos_incsup, str_inc_incsup, str_pos_incinf, str_inc_incinf, str_perf_fac);
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
        record->nb1_dist,
        record->nb1_ang,
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
            records[current].nb1_dist = atof(fields[14]);
            records[current].nb1_ang = atof(fields[15]);
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


