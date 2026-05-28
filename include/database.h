#ifndef DATABASE_H
#define DATABASE_H

#include "user.h"
#include "clinical.h"

#define USER_FILE "database/usuarios.csv"
#define CLINICAL_FILE "database/prontuarios.csv"

/**
 * @brief Inicializa o banco de dados, garantindo que a pasta 'database' exista.
 */
void database_init(void);

/**
 * @brief Salva um novo usuário no arquivo usuarios.csv.
 */
int save_user(User *user);


/**
 * @brief Busca um usuário pelo CPF em usuarios.csv.
 * Retorna um struct User com id = -1 se não encontrado.
 */
User find_user_by_cpf(const char *cpf);

/**
 * @brief Busca um usuário pelo ID em usuarios.csv.
 * Retorna um struct User com id = -1 se não encontrado.
 */
User find_user_by_id(int user_id);

/**
 * @brief Atualiza os dados de um usuário no arquivo usuarios.csv.
 */
int update_user(int user_id, User *user);

/**
 * @brief Deleta um usuário do arquivo usuarios.csv e todos os seus prontuários de prontuarios.csv.
 */
int delete_user(int user_id);

/**
 * @brief Salva um registro clínico no arquivo prontuarios.csv.
 */
int save_clinical_record(ClinicalRecord *record);

/**
 * @brief Carrega os registros clínicos de um paciente do arquivo prontuarios.csv.
 * Retorna o número de registros encontrados.
 */
int load_clinical_records(int patient_id, ClinicalRecord *records, int max_records);

#endif // DATABASE_H
