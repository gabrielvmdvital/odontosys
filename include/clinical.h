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
 * @brief Operadores de comparação suportados na tomada de decisão.
 */
typedef enum {
    OP_LESS_THAN,       /**< Operador menor que (<) */
    OP_GREATER_THAN,    /**< Operador maior que (>) */
    OP_EQUAL            /**< Operador de igualdade (==) */
} CompareOperator;

/**
 * @brief Estrutura do nó da árvore binária de decisão para pré-diagnóstico clínico.
 */
typedef struct DecisionNode {
    bool is_leaf;                       /**< Indica se o nó é folha (diagnóstico final) ou nó de decisão (pergunta) */
    char title[100];                    /**< Título do diagnóstico (se folha) ou descrição do teste (se pergunta) */
    char recommendation[256];           /**< Recomendação de tratamento sugerida (apenas se for nó folha) */
    
    CompareOperator op;                 /**< Operador de comparação condicional */
    double reference_value;             /**< Valor de referência clínico para o teste */
    
    struct DecisionNode *true_branch;   /**< Ponteiro para o próximo nó se a condição for verdadeira */
    struct DecisionNode *false_branch;  /**< Ponteiro para o próximo nó se a condição for falsa */
} DecisionNode;

/**
 * @brief Representa o prontuário final com o histórico clínico de um paciente.
 */
typedef struct {
    int id;                             /**< ID único do registro do prontuário */
    int patient_id;                     /**< ID do paciente relacionado (chave estrangeira) */
    char diag_date[11];                 /**< Data do pré-diagnóstico (DD/MM/AAAA) */
    PatientMetrics collected_metrics;   /**< Métricas coletadas no dia do atendimento */
    char diagnosis[100];                /**< Diagnóstico obtido através da árvore de decisão */
} ClinicalRecord;

/**
 * @brief Constrói e inicializa a árvore binária de decisão clínica na memória.
 * 
 * @return Retorna um ponteiro para o nó raiz da árvore alocada.
 */
DecisionNode* clinical_init_decision_tree(void);

/**
 * @brief Desaloca recursivamente todos os nós da árvore de decisão de forma limpa da memória heap.
 * 
 * @param root Ponteiro para o nó raiz da árvore a ser desalocada.
 */
void clinical_free_tree(DecisionNode *root);

/**
 * @brief Avalia recursivamente as métricas do paciente em relação aos valores de referência da árvore de decisão.
 * 
 * @param root Ponteiro para o nó atual (ou raiz) da árvore de decisão.
 * @param metrics As métricas físicas do paciente que serão avaliadas.
 * @return Retorna um ponteiro para o nó folha resultante, contendo o pré-diagnóstico e recomendações finais.
 */
DecisionNode* clinical_evaluate(DecisionNode *root, PatientMetrics metrics);

#endif // CLINICAL_H
