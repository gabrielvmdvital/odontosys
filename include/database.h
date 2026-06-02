#ifndef DATABASE_H
#define DATABASE_H

#include "user.h"
#include "clinical.h"

#define USER_FILE "database/usuarios.csv"
#define USER_FILE_TEMP "database/usuarios_temp.csv"
#define CLINICAL_FILE "database/prontuarios.csv"
#define CLINICAL_FILE_TEMP "database/prontuarios_temp.csv"
#define USER_COUNT_FILE "database/user_count.txt"

/**
 * @brief Inicializa o banco de dados, garantindo que a pasta 'database' exista.
 */
void database_init(void);

/**
 * @brief Pega o total de usuários cadastrados no arquivo cache.
 */
int get_cached_user_count(void);

/**
 * @brief Atualiza a quantidade de usuários salvos no cache.
 */
void update_cached_user_count(int delta);

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
 * @brief Carrega todos os usuários do arquivo usuarios.csv.
 * Retorna um array alocado dinamicamente e salva a quantidade em total_count.
 * deve ser liberado com free() após utilizar.
 */
User* get_all_users(int *total_count);

#endif // DATABASE_H
