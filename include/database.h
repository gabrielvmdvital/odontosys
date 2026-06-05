#ifndef DATABASE_H
#define DATABASE_H

#include "patient.h"
#include "clinical.h"

#define PATIENT_FILE "database/pacientes.csv"
#define PATIENT_FILE_TEMP "database/pacientes_temp.csv"
#define CLINICAL_FILE "database/prontuarios.csv"
#define CLINICAL_FILE_TEMP "database/prontuarios_temp.csv"
#define PATIENT_COUNT_FILE "database/patient_count.txt"

/**
 * @brief Inicializa o banco de dados, garantindo que a pasta 'database' exista.
 */
void database_init(void);

/**
 * @brief Pega o total de pacientes cadastrados no arquivo cache.
 */
int get_cached_patient_count(void);

/**
 * @brief Atualiza a quantidade de pacientes salvos no cache.
 */
void update_cached_patient_count(int delta);

/**
 * @brief Salva um novo paciente no arquivo pacientes.csv.
 */
int save_patient(Patient *patient);


/**
 * @brief Busca um paciente pelo CPF em pacientes.csv.
 * Retorna um struct Patient com id = -1 se não encontrado.
 */
Patient find_patient_by_cpf(const char *cpf);

/**
 * @brief Busca um paciente pelo ID em pacientes.csv.
 * Retorna um struct Patient com id = -1 se não encontrado.
 */
Patient find_patient_by_id(int patient_id);

/**
 * @brief Atualiza os dados de um paciente no arquivo pacientes.csv.
 */
int update_patient(int patient_id, Patient *patient);

/**
 * @brief Deleta um paciente do arquivo pacientes.csv e todos os seus prontuários de prontuarios.csv.
 */
int delete_patient(int patient_id);

/**
 * @brief Deleta todos os registros clínicos de um paciente do arquivo prontuarios.csv.
 * Retorna a quantidade de registros deletados.
 */
int delete_clinical_records_by_patient(int patient_id);

/**
 * @brief Salva um registro clínico no arquivo prontuarios.csv.
 */
int save_clinical_record(ClinicalRecord *record);

/**
 * @brief Carrega os registros clínicos de um paciente do arquivo prontuarios.csv.
 * Retorna um array alocado dinamicamente e salva a quantidade em total_count.
 * deve ser liberado com free() após utilizar.
 */
ClinicalRecord* load_clinical_records(int patient_id, int *total_count);

/**
 * @brief Carrega todos os pacientes do arquivo pacientes.csv.
 * Retorna um array alocado dinamicamente e salva a quantidade em total_count.
 * deve ser liberado com free() após utilizar.
 */
Patient* get_all_patients(int *total_count);

#endif // DATABASE_H
