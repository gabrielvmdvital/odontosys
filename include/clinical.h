#ifndef CLINICAL_H
#define CLINICAL_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Representa o prontuário final com o histórico clínico de um paciente.
 */
typedef struct {
    uint64_t clinical_id;               /**< ID único do registro do prontuário */
    uint64_t patient_id;                /**< ID do paciente relacionado (chave estrangeira) */
    uint64_t dentist_id;                /**< ID do dentista responsável (chave estrangeira) */
    char diag_date[11];                 /**< Data do pré-diagnóstico (DD/MM/AAAA) */
    int age;                            /**< Idade do paciente */
// Parâmetros necessários para diagnóstico
    float anb;                          /**< Relação maxila/mandíbula (ângulo) */
    float coa;                          /**< Comprimento da maxila (distância) */
    int maxila_tipo;                    /**< normal = 0 | protruida = 1 | retruida = -1 */
    int maxila_desvio;                  /**< se protruido ou retruida, desvio de quanto (valor positivo em mm)? */
    float cogn;                         /**< Comprimento mandibular (distância) */
    float afai;                         /**< Altura facial inferior (distância) */
    float sngogn;                       /**< Padrão vertical facial (ângulo) */
    float na1_dist;                     /**< Posição do incisivo superior (distância) */
    float na1_ang;                      /**< Inclinação do incisivo superior (ângulo) */
    float na2_dist;                     /**< Posição do incisivo inferior (distância) */
    float na2_ang;                      /**< Inclinação do incisivo inferior (ângulo) */
    char perf_tegument[50];             /**< Formato do perfil (texto) */
    char pre_diagnosis[500];            /**< Diagnóstico obtido através da árvore de decisão */
} ClinicalRecord;

void clinical_formular_diag(ClinicalRecord *record);
/**
 * @brief processar o pre diagnostico do paciente com as medidas de Clinicalrecord, tabela de Macnamara e as regras de ngócio.
 * 
 */

// CLINICAL RECORDS CRUD

/**
 * @brief Salva um registro clínico no arquivo prontuarios.csv.
 */
int save_clinical_record(ClinicalRecord *record);

/**
 * @brief Carrega os registros clínicos de um paciente do arquivo prontuarios.csv.
 * Retorna um array alocado dinamicamente e salva a quantidade em total_count.
 * deve ser liberado com free() após utilizar.
 */
ClinicalRecord* load_clinical_records(uint64_t patient_id, int *total_count);

/**
 * @brief Deleta todos os registros clínicos de um paciente do arquivo prontuarios.csv.
 * Retorna a quantidade de registros deletados.
 */
int delete_clinical_records_by_patient(uint64_t patient_id);

#endif // CLINICAL_H
