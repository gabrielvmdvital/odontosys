#ifndef PATIENT_H
#define PATIENT_H

#include <stdint.h>

/**
 * @brief Estrutura que representa o cadastro básico de um Paciente no sistema.
 */
typedef struct {
    uint64_t patient_id;    /**< Identificador único do paciente (chave primária) */
    uint64_t dentist_id;    /**< ID do dentista responsável pelo paciente */
    char name[100];         /**< Nome completo do paciente */
    char email[100];        /**< Endereço de e-mail do paciente */
    char cpf[15];           /**< Registro de CPF (Cadastro de Pessoas Físicas) */
    char birth_date[11];    /**< Data de nascimento (DD/MM/AAAA) */
    char phone[15];         /**< Telefone do paciente */
} Patient;


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
Patient find_patient_by_id(uint64_t patient_id);

/**
 * @brief Atualiza os dados de um paciente no arquivo pacientes.csv.
 */
int update_patient(uint64_t patient_id, Patient *patient);

/**
 * @brief Deleta um paciente do arquivo pacientes.csv e todos os seus prontuários de prontuarios.csv.
 */
int delete_patient(uint64_t patient_id);

/**
 * @brief Retorna a contagem em cache.
 */
int get_cached_patient_count(void);

/**
 * @brief Atualiza a contagem em cache.
 */
void update_cached_patient_count(int delta);

/**
 * @brief Carrega todos os pacientes do arquivo pacientes.csv.
 * Retorna um array alocado dinamicamente e salva a quantidade em total_count.
 * deve ser liberado com free() após utilizar.
 */
Patient* get_all_patients(int *total_count);

#endif // PATIENT_H