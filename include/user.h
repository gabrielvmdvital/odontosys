#ifndef USER_H
#define USER_H

#include "clinical.h"

/**
 * @brief Estrutura que representa o cadastro básico de um Usuário/Paciente no sistema.
 */
typedef struct {
    int id;                 /**< Identificador único do usuário (chave primária) */
    char name[100];         /**< Nome completo do usuário */
    char email[100];        /**< Endereço de e-mail do usuário */
    char cpf[11];           /**< Registro de CPF (Cadastro de Pessoas Físicas) */
    PatientMetrics metrics; /**< Métricas físicas básicas (altura/peso) registradas atualmente */
} User;

#endif // USER_H