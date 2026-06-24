#include "database.h"
#include "logs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <stdint.h>
#include <time.h>
#include <locale.h>

/**
 * @brief Divide uma linha em formato CSV em multiplos campos
 * @param line A string contendo a linha CSV completa
 * @param fields Array de ponteiros que guardará as strings separadas
 * @param max_fields Número máximo de colunas que esperamos receber
 * @return Retorna o numero de campos encontrados
 */
int split_csv_line(char *line, char **fields, int max_fields) {
    int count = 0;
    char *curr = line; // Ponteiro móvel que percorre a string
    
    // Percorre a string ate encontrar o final (caractere nulo) ou atingir o limite maximo de colunas
    while (*curr != '\0' && count < max_fields) {
        // Guarda a posição inicial da string atual no vetor de colunas
        fields[count++] = curr;
        
        // Procura a próxima ocorrência do delimitador ';' a partir do ponteiro atual
        char *delim = strchr(curr, ';'); // strchr encontra a primeira ocorrência do caractere na string
        
        if (delim != NULL) {
            // Se encontrar o delimitador, ele é substituído por '\0'
            // Isso efetivamente "quebra" a string original, isolando a coluna no ponteiro salvo
            *delim = '\0';
            
            // Avança o ponteiro móvel para a próxima posição após o delimitador que acabou de ser cortado
            curr = delim + 1;
        } else {
            // Se não encontrou delimitador, significa que estamos no último campo da linha
            
            // Procura e remove quebras de linha '\n' (Linux/Mac) no ultimo campo
            char *nl = strchr(curr, '\n');
            if (nl != NULL) *nl = '\0';
            
            // Procura e remove carriage returns '\r' (Windows) no ultimo campo
            nl = strchr(curr, '\r');
            if (nl != NULL) *nl = '\0';
            
            // Interrompe o loop pois era o último campo
            break;
        }
    }
    
    // Retorna a quantidade de colunas que conseguiu separar com sucesso
    return count;
}

/*
 * Inicializa a estrutura de diretorios e arquivos do banco de dados
 */
void database_init(void) {
    // Cria o diretorio base para os arquivos de dados caso nao exista
    CreateDirectoryA("database", NULL);

    // Inicializa o arquivo de pacientes com cabecalho caso esteja ausente
    FILE *fp = fopen(PATIENT_FILE, "r"); // fopen abre um arquivo
    if (!fp) {
        fp = fopen(PATIENT_FILE, "w");
        if (fp) {
            fprintf(fp, "patient_id;dentist_id;nome;email;cpf;data_nascimento;telefone\n");
            fclose(fp); // fclose fecha o manipulador do arquivo
        }
    } else {
        fclose(fp);
    }

    // Inicializa o arquivo clinico com cabecalho caso esteja ausente
    FILE *fc = fopen(CLINICAL_FILE, "r");
    if (!fc) {
        fc = fopen(CLINICAL_FILE, "w");
        if (fc) {
            fprintf(fc, "clinical_id;patient_id;dentist_id;diag_date;idade;anb;coa;maxila_tipo;maxila_desvio;cogn;afai;sngogn;na1_dist;na1_ang;nb1_dist;nb1_ang;perf_tegument;pre_diagnostico\n");
            fclose(fc);
        }
    } else {
        fclose(fc);
    }

    // Inicializa o arquivo de dentistas com cabecalho e admin padrao caso ausente
    FILE *fu = fopen(DENTIST_FILE, "r");
    if (!fu) {
        fu = fopen(DENTIST_FILE, "w");
        if (fu) {
            fprintf(fu, "dentist_id;name;username;cpf;password;role\n");
            fprintf(fu, "0;admin;admin;00000000000;admin;1\n");
            fclose(fu);
        }
    } else {
        fclose(fu);
    }

    log_message(LOG_INFO, "[DATABASE] Banco de dados inicializado.");
}

/*
 * Adiciona uma nova linha ao final do arquivo especificado
 * Retorna 1 em caso de sucesso, 0 caso contrario
 */
int db_append_line(const char *filepath, const char *line) {
    database_init();
    FILE *file = fopen(filepath, "a");
    if (file == NULL) {
        log_message(LOG_ERROR, "[DATABASE] Nao foi possivel abrir o arquivo %s para append.", filepath);
        return 0;
    }
    fprintf(file, "%s", line);
    fclose(file);
    return 1;
}

/**
 * @brief Remove linhas de um arquivo baseando-se no valor de uma coluna especifica
 * Retorna a quantidade de linhas removidas
 */
int db_delete_lines(const char *filepath, const char *temp_filepath, int filter_col_idx, const char *filter_val, int max_cols) {
    // Abre o arquivo fonte para leitura
    FILE *src = fopen(filepath, "r");
    if (src == NULL) return 0; // Aborta e retorna 0 se não conseguiu ler

    // Abre um arquivo temporario para escrita (será o novo arquivo limpo)
    FILE *dest = fopen(temp_filepath, "w");
    if (dest == NULL) {
        fclose(src);
        return 0; // Aborta se não conseguir criar o arquivo temporário
    }

    char line[1024];
    char line_copy[1024];
    
    // Aloca memória dinamicamente para o array de ponteiros que receberá as colunas separadas
    char **fields = malloc(max_cols * sizeof(char*)); // malloc aloca memória dinâmica na heap
    int deleted = 0;

    // Itera linha por linha de todo o arquivo de origem original
    while (fgets(line, sizeof(line), src)) { // fgets lê uma linha do arquivo e previne overflow
        // Se a linha estiver vazia ou for apenas uma quebra, copia ela diretamente e pula pra próxima
        if (line[0] == '\n' || line[0] == '\r') {
            fprintf(dest, "%s", line);
            continue;
        }

        // Cria uma cópia da string original para ser fatiada/destruída pelo split_csv sem corromper a variável principal
        strcpy(line_copy, line); // strcpy copia a string da origem para o destino
        
        // Pica a string em colunas usando a função helper
        int cols = split_csv_line(line_copy, fields, max_cols);

        // Se a linha tem a coluna do índice necessário E o valor dessa coluna for IDÊNTICO ao valor filtrado
        if (cols > filter_col_idx && strcmp(fields[filter_col_idx], filter_val) == 0) { // strcmp compara strings (0 = iguais)
            // Omitimos a escrita no arquivo destino, ou seja, na prática "apagamos" a linha e registramos na variável
            deleted++; 
        } else {
            // Se for diferente do valor alvo do filtro, mantemos a linha escrevendo-a no novo arquivo temp
            fprintf(dest, "%s", line);
        }
    }

    // Limpa a memória e fecha os manipuladores de arquivo para permitir manipulações no SO
    free(fields); // free libera a memória dinâmica alocada
    fclose(src);
    fclose(dest);

    // Estratégia de Atomicidade: Deleta o arquivo antigo inteiro
    remove(filepath); // remove deleta um arquivo do SO
    
    // Renomeia o temporário (que está com o dado alterado/removido) para assumir o nome do antigo
    rename(temp_filepath, filepath); // rename renomeia um arquivo no SO

    // Retorna quantas linhas foram de fato ignoradas (deletadas) na transição
    return deleted;
}

/*
 * Atualiza uma linha especifica em um arquivo, buscando pelo valor em uma coluna
 * Retorna 1 se a linha foi atualizada, 0 caso contrario
 */
int db_update_line(const char *filepath, const char *temp_filepath, int filter_col_idx, const char *filter_val, const char *new_line, int max_cols) {
    // Abre o arquivo fonte para leitura
    FILE *src = fopen(filepath, "r");
    if (src == NULL) return 0;

    // Abre um arquivo temporario para escrita
    FILE *dest = fopen(temp_filepath, "w");
    if (dest == NULL) {
        fclose(src);
        return 0;
    }

    char line[1024];
    char line_copy[1024];
    char **fields = malloc(max_cols * sizeof(char*));
    int updated = 0;

    // Itera por todas as linhas do arquivo de origem
    while (fgets(line, sizeof(line), src)) {
        if (line[0] == '\n' || line[0] == '\r') {
            fprintf(dest, "%s", line);
            continue;
        }

        strcpy(line_copy, line);
        int cols = split_csv_line(line_copy, fields, max_cols);

        // Substitui o conteudo se a linha coincidir com o valor filtrado
        if (cols > filter_col_idx && strcmp(fields[filter_col_idx], filter_val) == 0) {
            fprintf(dest, "%s", new_line);
            updated = 1;
        } else {
            fprintf(dest, "%s", line);
        }
    }

    free(fields);
    fclose(src);
    fclose(dest);

    // Substitui o arquivo original pelo temporario atualizado
    remove(filepath);
    rename(temp_filepath, filepath);

    return updated;
}

/*
 * Gera um ID unico de 64 bits combinando timestamp e contador interno
 */
uint64_t generate_unique_id(void) {
    static uint64_t contador = 0; // contador interno para evitar colisões no mesmo segundo
    time_t agora = time(NULL); // time obtém o timestamp atual do sistema

    if (agora == ((time_t)-1)) {
        log_message(LOG_ERROR, "[DATABASE] Erro ao obter o tempo do sistema.");
        exit(EXIT_FAILURE);
    }

    // Incrementa o contador a cada chamada
    contador++;

    // Combina o timestamp (32 bits) com o contador (32 bits)
    uint64_t id = ((uint64_t)agora << 32) | (contador & 0xFFFFFFFF);
    return id;
}