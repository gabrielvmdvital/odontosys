#ifndef DATABASE_H
#define DATABASE_H

#include <stdint.h>

#define PATIENT_FILE "database/pacientes.csv"
#define PATIENT_FILE_TEMP "database/pacientes_temp.csv"
#define CLINICAL_FILE "database/prontuarios.csv"
#define CLINICAL_FILE_TEMP "database/prontuarios_temp.csv"
#define DENTIST_FILE "database/dentists.csv"
#define DENTIST_FILE_TEMP "database/dentists_temp.csv"
#define PATIENT_COUNT_FILE "database/patient_count.txt"

/**
 * @brief Inicializa o banco de dados, garantindo que a pasta 'database' e os arquivos existam.
 */
void database_init(void);

/**
 * @brief Quebra uma linha de CSV delimitada por ';' em um array de strings.
 * @param line A linha a ser processada (ela será modificada, '\0' inseridos)
 * @param fields Array de ponteiros onde as colunas serão armazenadas
 * @param max_fields Número máximo de colunas esperadas
 * @return Número de colunas encontradas
 */
int split_csv_line(char *line, char **fields, int max_fields);

/**
 * @brief Adiciona uma linha ao final de um arquivo CSV.
 * @param filepath Caminho do arquivo
 * @param line Linha formatada a ser adicionada (não esqueça o \n no final)
 * @return 1 em sucesso, 0 em falha
 */
int db_append_line(const char *filepath, const char *line);

/**
 * @brief Remove linhas de um arquivo CSV onde a coluna especificada tem um valor exato.
 * @param filepath Caminho do arquivo CSV principal
 * @param temp_filepath Caminho do arquivo CSV temporário para manipulação
 * @param filter_col_idx Índice da coluna (0 a N) a ser testada
 * @param filter_val Valor a ser comparado (ex: ID em string)
 * @param max_cols Número máximo de colunas para usar no parse
 * @return Quantidade de linhas deletadas
 */
int db_delete_lines(const char *filepath, const char *temp_filepath, int filter_col_idx, const char *filter_val, int max_cols);

/**
 * @brief Atualiza uma linha em um arquivo CSV onde a coluna especificada tem um valor exato.
 * @param filepath Caminho do arquivo CSV principal
 * @param temp_filepath Caminho do arquivo CSV temporário
 * @param filter_col_idx Índice da coluna a ser testada
 * @param filter_val Valor a ser comparado (ex: ID em string)
 * @param new_line Nova linha formatada que substituirá a antiga
 * @param max_cols Número máximo de colunas para usar no parse
 * @return 1 em sucesso (atualizou), 0 em falha
 */
int db_update_line(const char *filepath, const char *temp_filepath, int filter_col_idx, const char *filter_val, const char *new_line, int max_cols);

/**
 * @brief Obtém um ID único de 64 bits baseado no timestamp e num contador local.
 * @return Próximo ID
 */
uint64_t generate_unique_id(void);

#endif // DATABASE_H
