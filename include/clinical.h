#ifndef CLINICAL_H
#define CLINICAL_H

#include <stdbool.h>

/**
 * @brief Representa as métricas físicas básicas de um paciente para análise clínica.
 */
typedef struct {
    double height; /**< Altura do paciente em metros */
    double weight; /**< Peso do paciente em quilogramas */
    int age;
} PatientMetrics;


/**
 * @brief Representa o prontuário final com o histórico clínico de um paciente.
 */
typedef struct {
    int id;                             /**< ID único do registro do prontuário */
    int patient_id;                     /**< ID do paciente relacionado (chave estrangeira) */
    char diag_date[11];                 /**< Data do pré-diagnóstico (DD/MM/AAAA) */
    PatientMetrics collected_metrics;   /**< Métricas coletadas no dia do atendimento */
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

#endif // CLINICAL_H
