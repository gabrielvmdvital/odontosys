#ifndef PATIENT_H
#define PATIENT_H

#include "clinical.h"

/**
 * @brief Estrutura que representa o cadastro básico de um Paciente no sistema.
 */
typedef struct {
    int id;                 /**< Identificador único do paciente (chave primária) */
    char name[100];         /**< Nome completo do paciente */
    char email[100];        /**< Endereço de e-mail do paciente */
    char cpf[11];           /**< Registro de CPF (Cadastro de Pessoas Físicas) */
    char birth_date[11];    /**< Data de nascimento (DD/MM/AAAA) */
    PatientMetrics metrics; /**< Métricas físicas básicas (altura/peso) registradas atualmente */
} Patient;

#endif // PATIENT_H