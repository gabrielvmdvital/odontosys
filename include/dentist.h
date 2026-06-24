#ifndef DENTIST_H
#define DENTIST_H

#include <stdint.h>

/**
 * @brief Estrutura que representa o cadastro básico de um Dentista do sistema.
 */
typedef struct {
    uint64_t dentist_id;     /**< ID do dentista (chave primária) */
    char name[100];     /**< Nome do dentista */
    char username[50];  /**< Username de acesso */
    char cpf[15];       /**< CPF do dentista (chave primária) */
    char password[100]; /**< Senha do dentista */
    int role;           /**< Nível de acesso (e.g., 1=Admin, 2=Dentista) */
} Dentist;


// USER CRUD

/**
 * @brief Salva um novo dentista no arquivo dentists.csv.
 */
int save_dentist(Dentist *dentist);

/**
 * @brief Verifica se o dentista e senha estão corretos.
 */
int check_dentist(const char *cpf_or_user, const char *password); 

/**
 * @brief Atualiza os dados de um dentista.
 */
int update_dentist(Dentist *dentist);

/**
 * @brief Deleta um dentista do arquivo dentists.csv.
 */
int delete_dentist(uint64_t dentist_id);

/**
 * @brief Valida as credenciais de login de um dentista.
 * Retorna o ID do dentista se as credenciais forem validas, ou -1 caso contrario.
 */
int validate_login(const char *cpf_or_user, const char *password);

/**
 * @brief Busca dentista pelo CPF no arquivo dentists.csv.
 * Retorna um struct Dentist com dentist_id = -1 se não encontrado.
 */
Dentist find_dentist_by_cpf(const char *cpf);

#endif // DENTIST_H