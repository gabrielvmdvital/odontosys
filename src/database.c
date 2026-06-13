#include "database.h" // Inclui o cabeçalho com definições e estruturas do banco de dados
#include "logs.h"     // Inclui funções para registro de logs do sistema
#include <stdio.h>    // Inclui a biblioteca padrão de entrada e saída para manipulação de arquivos
#include <stdlib.h>   // Inclui a biblioteca padrão para alocação de memória (malloc, etc) e conversões
#include <string.h>   // Inclui a biblioteca para manipulação de strings (strcpy, strcmp, etc)
#include <windows.h>  // Inclui funções da API do Windows, como CreateDirectoryA

// Função auxiliar interna para dividir uma linha de CSV delimitada por ponto e vírgula.
static int split_csv_line(char *line, char **fields, int max_fields) {
    int count = 0; // Inicializa o contador de campos encontrados
    char *curr = line; // Ponteiro que percorre a string da linha atual
    while (*curr != '\0' && count < max_fields) { // Loop até o fim da string ou até atingir o limite de campos
        fields[count++] = curr; // Salva o ponteiro de início do campo atual no array fields e incrementa o contador
        char *delim = strchr(curr, ';'); // Procura pelo próximo ponto e vírgula (;)
        if (delim != NULL) { // Se encontrou o delimitador
            *delim = '\0'; // Substitui o delimitador por terminador nulo para isolar a string do campo
            curr = delim + 1; // Avança o ponteiro curr para o início do próximo campo
        } else { // Se não encontrou delimitador (último campo)
            // Remove quebras de linha (\r ou \n) no último campo
            char *nl = strchr(curr, '\n'); // Procura por nova linha (\n)
            if (nl != NULL) *nl = '\0'; // Se achar, substitui por terminador nulo
            nl = strchr(curr, '\r'); // Procura por retorno de carro (\r)
            if (nl != NULL) *nl = '\0'; // Se achar, substitui por terminador nulo
            break; // Sai do loop, pois não há mais campos
        }
    }
    return count; // Retorna a quantidade total de campos encontrados
}

void database_init(void) {
    // Cria a pasta "database" na raiz do projeto se ela não existir.
    CreateDirectoryA("database", NULL); // Chamada da API do Windows para criar diretório

    FILE *fp = fopen(PATIENT_FILE, "r"); // Tenta abrir o arquivo de pacientes em modo leitura
    if (!fp) { // Se o arquivo não existir (fopen retornou NULL)
        fp = fopen(PATIENT_FILE, "w"); // Cria o arquivo de pacientes em modo de escrita
        if (fp) { // Se a criação foi bem-sucedida
            fprintf(fp, "id;nome;email;cpf;data_nascimento;altura;peso\n"); // Escreve o cabeçalho das colunas do CSV
            fclose(fp); // Fecha o arquivo recém-criado
        }
    } else { // Se o arquivo já existe
        fclose(fp); // Apenas fecha o arquivo
    }

    FILE *fc = fopen(CLINICAL_FILE, "r"); // Tenta abrir o arquivo de prontuários em modo leitura
    if (!fc) { // Se o arquivo não existir
        fc = fopen(CLINICAL_FILE, "w"); // Cria o arquivo em modo de escrita
        if (fc) { // Se a criação foi bem-sucedida
            // Escreve o cabeçalho longo do CSV de prontuários
            fprintf(fc, "id;patient_id;diag_date;altura;peso;idade;anb;coa;maxila_tipo;maxila_desvio;cogn;afai;sngogn;na1_dist;na1_ang;na2_dist;na2_ang;perf_tegument;pre_diagnostico\n");
            fclose(fc); // Fecha o arquivo de prontuários
        }
    } else { // Se já existe
        fclose(fc); // Apenas o fecha
    }

    log_message(LOG_INFO, "Banco de dados inicializado."); // Registra no log que o banco iniciou corretamente
}

int get_cached_patient_count(void) {
    FILE *f = fopen(PATIENT_COUNT_FILE, "r"); // Abre o arquivo de contagem de pacientes para leitura
    if (!f) return 0; // Se não conseguir abrir (arquivo inexistente), retorna contagem 0
    int count = 0; // Variável para armazenar a contagem lida
    fscanf(f, "%d", &count); // Lê um número inteiro do arquivo e armazena em count
    fclose(f); // Fecha o arquivo
    return count; // Retorna a quantidade lida
}

void update_cached_patient_count(int delta) {
    int count = get_cached_patient_count(); // Pega a contagem atual usando a função definida anteriormente
    count += delta; // Adiciona (ou subtrai) o delta fornecido
    if (count < 0) count = 0; // Impede que a contagem fique negativa
    FILE *f = fopen(PATIENT_COUNT_FILE, "w"); // Abre o arquivo de contagem no modo escrita, sobrescrevendo seu conteúdo
    if (f) { // Se conseguiu abrir com sucesso
        fprintf(f, "%d", count); // Escreve o novo valor no arquivo
        fclose(f); // Fecha o arquivo
    }
}

int save_patient(Patient *patient) {
    database_init(); // Garante que a pasta e os arquivos básicos existam

    Patient existing = find_patient_by_cpf(patient->cpf); // Busca se já existe um paciente com este CPF
    if (existing.id != -1) { // Se id for diferente de -1, significa que o paciente foi encontrado
        log_message(LOG_WARNING, "Falha ao salvar: ja existe um paciente com o CPF %s.", patient->cpf); // Log de aviso de duplicidade
        return 0; // Retorna falha (0)
    }

    FILE *file = fopen(PATIENT_FILE, "a"); // Abre o arquivo de pacientes em modo "append" (anexar no final)
    if (file == NULL) { // Se deu erro ao abrir o arquivo
        log_message(LOG_ERROR, "Nao foi possivel abrir o arquivo %s para salvar o paciente.", PATIENT_FILE); // Log de erro
        return 0; // Retorna falha (0)
    }

    // Escreve os dados do paciente no final do arquivo, formatado em CSV separado por ';'
    fprintf(file, "%d;%s;%s;%s;%s;%.2f;%.2f\n", 
            patient->id, 
            patient->name, 
            patient->email, 
            patient->cpf, 
            patient->birth_date,
            patient->metrics.height, 
            patient->metrics.weight);
    fclose(file); // Fecha o arquivo para consolidar a escrita no disco
    log_message(LOG_INFO, "Paciente cadastrado no CSV com sucesso: %s (ID: %d)", patient->name, patient->id); // Registra log de sucesso
    update_cached_patient_count(1); // Incrementa o contador de pacientes no cache em +1
    return 1; // Retorna sucesso (1)
}

Patient find_patient_by_cpf(const char *cpf) {
    Patient found; // Variável que armazenará os dados se o paciente for achado
    memset(&found, 0, sizeof(Patient)); // Zera toda a memória ocupada pela struct found
    found.id = -1; // Define ID inicial como -1, indicando que ainda não achou

    FILE *file = fopen(PATIENT_FILE, "r"); // Abre o arquivo CSV para leitura
    if (file == NULL) { // Se não encontrou o arquivo
        return found; // Retorna struct vazia (id -1)
    }

    char line[512]; // Buffer para ler as linhas do arquivo (máx 512 caracteres)
    char *fields[7]; // Array para armazenar ponteiros para as 7 colunas
    while (fgets(line, sizeof(line), file)) { // Lê linha por linha até o fim do arquivo
        if (line[0] == '\n' || line[0] == '\r') continue; // Pula linhas em branco
        if (strncmp(line, "id;", 3) == 0) continue; // Pula o cabeçalho principal
        // Repetições do código original, pulam o cabeçalho de qualquer maneira
        if (strncmp(line, "id;", 3) == 0) continue; 
        if (strncmp(line, "id;", 3) == 0) continue;

        char line_copy[512]; // Buffer cópia pois a função split_csv_line modifica a string
        strcpy(line_copy, line); // Copia o conteúdo lido

        int cols = split_csv_line(line_copy, fields, 7); // Quebra a linha e joga os ponteiros nos fields
        if (cols < 7) continue; // Se não tiver 7 colunas, a linha está inválida, pula para a próxima

        if (strcmp(fields[3], cpf) == 0) { // Compara a coluna 3 (CPF) com o CPF desejado
            found.id = atoi(fields[0]); // Converte ID para int
            strcpy(found.name, fields[1]); // Copia nome
            strcpy(found.email, fields[2]); // Copia email
            strcpy(found.cpf, fields[3]); // Copia CPF
            strcpy(found.birth_date, fields[4]); // Copia data de nascimento
            found.metrics.height = atof(fields[5]); // Converte altura para float
            found.metrics.weight = atof(fields[6]); // Converte peso para float
            break; // Interrompe o loop pois já encontrou quem queria
        }
    }

    fclose(file); // Fecha o arquivo lido
    return found; // Retorna os dados do paciente, ou id = -1 se não encontrou
}

Patient find_patient_by_id(int patient_id) {
    Patient found; // Declara a struct de retorno
    found.id = -1; // Inicializa ID como não encontrado

    FILE *file = fopen(PATIENT_FILE, "r"); // Abre leitura
    if (file == NULL) { // Se der erro na leitura
        return found; // Retorna id -1
    }

    char line[512]; // Buffer
    char *fields[7]; // Ponteiros para as colunas
    while (fgets(line, sizeof(line), file)) { // Lê o arquivo inteiro linha a linha
        if (line[0] == '\n' || line[0] == '\r') continue; // Ignora quebras de linha
        if (strncmp(line, "id;", 3) == 0) continue; // Pula cabeçalho
        if (strncmp(line, "id;", 3) == 0) continue; // Pula cabeçalho
        if (strncmp(line, "id;", 3) == 0) continue; // Pula cabeçalho

        char line_copy[512]; // Cópia de linha
        strcpy(line_copy, line); // Faz a cópia

        int cols = split_csv_line(line_copy, fields, 7); // Split
        if (cols < 7) continue; // Pula linhas mal formatadas

        if (atoi(fields[0]) == patient_id) { // Verifica se a conversão do campo 0 (ID) é igual ao buscado
            found.id = atoi(fields[0]); // Pega ID
            strcpy(found.name, fields[1]); // Pega nome
            strcpy(found.email, fields[2]); // Pega email
            strcpy(found.cpf, fields[3]); // Pega cpf
            strcpy(found.birth_date, fields[4]); // Pega nascimento
            found.metrics.height = atof(fields[5]); // Pega altura
            found.metrics.weight = atof(fields[6]); // Pega peso
            break; // Já achou, sai do loop
        }
    }

    fclose(file); // Fecha leitura
    return found; // Retorna os dados
}

int update_patient(int patient_id, Patient *patient) {

    Patient existing = find_patient_by_cpf(patient->cpf); // Procura se o CPF que quer atualizar já está em uso
    if (existing.id != -1 && existing.id != patient_id) { // Se estiver sendo usado por OUTRO paciente (ID diferente)
        log_message(LOG_WARNING, "Falha ao atualizar: ja existe outro paciente com o CPF %s.", patient->cpf); // Erro de duplicidade
        return 0; // Retorna falha
    }

    FILE *src = fopen(PATIENT_FILE, "r"); // Abre arquivo atual em modo leitura
    if (src == NULL) { // Se não conseguiu abrir
        log_message(LOG_WARNING, "Tentativa de atualizar paciente ID %d, mas %s nao existe.", patient_id, PATIENT_FILE); // Log de erro
        return 0; // Falha
    }

    FILE *dest = fopen("database/pacientes.tmp", "w"); // Cria um arquivo temporário na pasta database
    if (dest == NULL) { // Se erro na criação
        log_message(LOG_ERROR, "Nao foi possivel criar o arquivo temporario database/pacientes.tmp para atualizacao."); // Log
        fclose(src); // Fecha o primeiro
        return 0; // Retorna falha
    }

    char line[512]; // Buffer de leitura
    char line_copy[512]; // Cópia de leitura
    char *fields[7]; // Ponteiros para split
    int updated = 0; // Flag para saber se alterou alguém

    while (fgets(line, sizeof(line), src)) { // Percorre todo o CSV antigo
        if (line[0] == '\n' || line[0] == '\r') { // Se for linha vazia
            fprintf(dest, "%s", line); // Só copia igual
            continue; // E pula pro próximo
        }

        strcpy(line_copy, line); // Copia para o split não estragar a original
        int cols = split_csv_line(line_copy, fields, 7); // Faz o split da linha

        if (cols >= 7 && atoi(fields[0]) == patient_id) { // Se as colunas estiverem certas e for o ID procurado
            // Grava no arquivo NOVO a nova versão dos dados
            fprintf(dest, "%d;%s;%s;%s;%s;%.2f;%.2f\n", 
                    patient->id, 
                    patient->name, 
                    patient->email, 
                    patient->cpf, 
                    patient->birth_date,
                    patient->metrics.height, 
                    patient->metrics.weight);
            updated = 1; // Marca que realizou a atualização
        } else {
            fprintf(dest, "%s", line); // Se não for o ID procurado, apenas copia a linha original para o novo arquivo
        }
    }

    fclose(src); // Fecha o arquivo fonte
    fclose(dest); // Fecha o arquivo temporário com tudo atualizado

    remove(PATIENT_FILE); // Deleta o arquivo antigo
    rename("database/pacientes.tmp", PATIENT_FILE); // Renomeia o temporário para ser o novo arquivo oficial

    if (updated) { // Se gravou os novos dados
        log_message(LOG_INFO, "Paciente ID %d (%s) atualizado com sucesso no CSV.", patient_id, patient->name); // Sucesso
    } else { // Se não encontrou aquele ID no arquivo
        log_message(LOG_WARNING, "Nenhum paciente correspondente ao ID %d encontrado para atualizacao no CSV.", patient_id); // Aviso
    }

    return updated; // Retorna o status de sucesso ou falha
}

int delete_patient(int patient_id) {
    FILE *src = fopen(PATIENT_FILE, "r"); // Abre o arquivo atual de pacientes para leitura
    if (src == NULL) { // Se não abrir, falha
        return 0;
    }

    FILE *dest = fopen(PATIENT_FILE_TEMP, "w"); // Abre o arquivo temporário de pacientes
    if (dest == NULL) { // Se não criar o temporário
        fclose(src); // Fecha a leitura
        return 0; // Retorna erro
    }

    char line[512]; // Buffer original
    char line_copy[512]; // Cópia pro split
    char *fields[7]; // Arrays do split
    int deleted = 0; // Flag marcando exclusão

    while (fgets(line, sizeof(line), src)) { // Percorre linha a linha o original
        if (line[0] == '\n' || line[0] == '\r') { // Ignora pulos de linha gravando eles cegamente
            fprintf(dest, "%s", line);
            continue;
        }

        strcpy(line_copy, line); // Copia para quebrar
        int cols = split_csv_line(line_copy, fields, 7); // Quebra em array

        if (cols >= 7 && atoi(fields[0]) == patient_id) { // Se achar o ID a ser apagado
            deleted = 1; // Só marca a flag como verdadeiro, ou seja: pula a escrita no `dest`, excluindo o cara
        } else {
            fprintf(dest, "%s", line); // Para os demais pacientes, copia normal para o novo arquivo
        }
    }

    fclose(src); // Fecha leitura
    fclose(dest); // Fecha gravação

    remove(PATIENT_FILE); // Apaga os dados velhos
    rename(PATIENT_FILE_TEMP, PATIENT_FILE); // Sobrescreve com os dados novos (sem o cara)

    if (deleted) { // Se apagou com sucesso
        log_message(LOG_INFO, "Paciente ID %d removido do CSV com sucesso.", patient_id); // Registra ação
        update_cached_patient_count(-1); // Reduz em -1 a contagem total no cache

        // Remove todos os prontuários associados a este usuário
        delete_clinical_records_by_patient(patient_id); // Chama a função encadeada para manter a limpeza de banco
    }

    return deleted; // Retorna status final
}

int delete_clinical_records_by_patient(int patient_id) {
    FILE *cl_src = fopen(CLINICAL_FILE, "r"); // Abre o CSV de prontuários em leitura
    if (cl_src == NULL) { // Erro ao ler
        return 0;
    }

    FILE *cl_dest = fopen(CLINICAL_FILE_TEMP, "w"); // Abre CSV de prontuários temporário para gravação
    if (cl_dest == NULL) { // Erro ao criar temporário
        fclose(cl_src);
        return 0;
    }

    char cl_line[512]; // Buffer de leitura
    char cl_line_copy[512]; // Cópia
    char *cl_fields[19]; // Array com 19 campos pro CSV de prontuários
    int cl_deleted_count = 0; // Contador de quantos foram removidos

    while (fgets(cl_line, sizeof(cl_line), cl_src)) { // Percorre CSV inteiro de prontuários
        if (cl_line[0] == '\n' || cl_line[0] == '\r') { // Ignora pulos
            fprintf(cl_dest, "%s", cl_line);
            continue;
        }

        strcpy(cl_line_copy, cl_line); // Copia pro split
        int cl_cols = split_csv_line(cl_line_copy, cl_fields, 19); // Pega os 19 dados

        if (cl_cols >= 19 && atoi(cl_fields[1]) == patient_id) { // Verifica se é do paciente a ser apagado (índice 1)
            cl_deleted_count++; // Incrementa contador de deletados, PULA A ESCRITA, então o registro evapora do novo arquivo
        } else {
            fprintf(cl_dest, "%s", cl_line); // Se não for dele, copia
        }
    }
    
    fclose(cl_src); // Fecha o velho
    fclose(cl_dest); // Fecha o novo
    
    remove(CLINICAL_FILE); // Apaga o velho
    rename(CLINICAL_FILE_TEMP, CLINICAL_FILE); // Põe o novo no lugar
    
    if (cl_deleted_count > 0) { // Se encontrou e apagou pelo menos 1 prontuário
        log_message(LOG_INFO, "%d prontuarios vinculados ao paciente ID %d foram removidos de %s.", cl_deleted_count, patient_id, CLINICAL_FILE);
    }
    
    return cl_deleted_count; // Retorna quantos foram deletados
}

int save_clinical_record(ClinicalRecord *record) {
    database_init(); // Garante estrutura de arquivos

    Patient patient = find_patient_by_id(record->patient_id); // Verifica se o dono do prontuário ainda existe
    if (patient.id == -1) { // Se não encontrou o paciente
        log_message(LOG_ERROR, "Impossivel criar prontuario: Paciente ID %d nao existe.", record->patient_id); // Rejeita a gravação
        return 0; // Retorna erro
    }

    FILE *file = fopen(CLINICAL_FILE, "a"); // Abre o CSV em append para inserir no fim
    if (file == NULL) { // Se deu erro de permissão
        log_message(LOG_ERROR, "Nao foi possivel abrir o arquivo %s para gravar prontuario.", CLINICAL_FILE); // Log de erro
        return 0; // Erro
    }

    // Formato CSV do prontuário: id;patient_id;data;altura;peso;idade;anb;coa;maxila_tipo;maxila_desvio;cogn;afai;sngogn;na1_dist;na1_ang;na2_dist;na2_ang;perf_tegument;pre_diagnostico
    // Grava tudo no final da linha, respeitando a ordem
    fprintf(file, "%d;%d;%s;%.2f;%.2f;%d;%.2f;%.2f;%d;%d;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f;%s;%s\n", 
        record->id, 
        record->patient_id, 
        record->diag_date, 
        record->collected_metrics.height, 
        record->collected_metrics.weight, 
        record->collected_metrics.age,
        record->anb,
        record->coa,
        record->maxila_tipo,
        record->maxila_desvio,
        record->cogn,
        record->afai,
        record->sngogn,
        record->na1_dist,
        record->na1_ang,
        record->na2_dist,
        record->na2_ang,
        record->perf_tegument,
        record->pre_diagnosis
    );

    fclose(file); // Salva no disco
    log_message(LOG_INFO, "Prontuario ID %d adicionado no CSV para Paciente ID %d.", record->id, record->patient_id); // Log de sucesso
    return 1; // Retorna sucesso
}

ClinicalRecord* load_clinical_records(int patient_id, int *total_count) {
    *total_count = 0; // Zera o contador de saída
    FILE *file = fopen(CLINICAL_FILE, "r"); // Abre o arquivo de registros clínicos
    if (file == NULL) { // Se falhar, retorna null
        return NULL;
    }

    char line[512]; // Buffer de leitura
    char *fields[19]; // Array de ponteiros pra colunas
    int count = 0; // Variável para a primeira contagem

    // 1º Passo: Conta quantos registros pertencem ao paciente
    while (fgets(line, sizeof(line), file)) { // Varre todo o arquivo
        if (line[0] == '\n' || line[0] == '\r') continue; // Pula linha vazia
        if (strncmp(line, "id;", 3) == 0) continue; // Pula cabeçalho
        if (strncmp(line, "id;", 3) == 0) continue; // Pula cabeçalho
        if (strncmp(line, "id;", 3) == 0) continue; // Pula cabeçalho
        char line_copy[512]; // Buffer cópia
        strcpy(line_copy, line); // Copia pra não corromper no split
        int cols = split_csv_line(line_copy, fields, 19); // Extrai dados
        if (cols >= 19 && atoi(fields[1]) == patient_id) { // Se for registro validado para ESTE ID (índice 1)
            count++; // Incrementa
        }
    }

    if (count == 0) { // Se não achar nada dele
        fclose(file); // Fecha
        return NULL; // Retorna vetor nulo
    }

    // 2º Passo: Aloca a memória na quantidade correta que foi contada
    ClinicalRecord *records = malloc(count * sizeof(ClinicalRecord)); // Alocação dinâmica de array
    if (records == NULL) { // Se faltar memória RAM
        fclose(file);
        return NULL; // Desiste
    }

    fseek(file, 0, SEEK_SET); // Reseta o ponteiro de leitura do arquivo pra voltar pra primeira linha
    int current = 0; // Index no array

    // 3º Passo: Lê de novo, agora salvando na memória
    while (fgets(line, sizeof(line), file) && current < count) { // Lê novamente varrendo
        if (line[0] == '\n' || line[0] == '\r') continue; // Ignora os vazios
        if (strncmp(line, "id;", 3) == 0) continue; // Ignora o cabeçalho

        char line_copy[512]; // Buffer copy
        strcpy(line_copy, line); // Copy line

        int cols = split_csv_line(line_copy, fields, 19); // Split final
        if (cols < 19) continue; // Se linha for inválida

        if (atoi(fields[1]) == patient_id) { // Verifica se é dele novamente
            // Preenche o array `records` na posição atual
            records[current].id = atoi(fields[0]);
            records[current].patient_id = atoi(fields[1]);
            strcpy(records[current].diag_date, fields[2]);
            records[current].collected_metrics.height = atof(fields[3]);
            records[current].collected_metrics.weight = atof(fields[4]);
            records[current].collected_metrics.age = atoi(fields[5]);
            records[current].anb = atof(fields[6]);
            records[current].coa = atof(fields[7]);
            records[current].maxila_tipo = atoi(fields[8]);
            records[current].maxila_desvio = atoi(fields[9]);
            records[current].cogn = atof(fields[10]);
            records[current].afai = atof(fields[11]);
            records[current].sngogn = atof(fields[12]);
            records[current].na1_dist = atof(fields[13]);
            records[current].na1_ang = atof(fields[14]);
            records[current].na2_dist = atof(fields[15]);
            records[current].na2_ang = atof(fields[16]);
            strcpy(records[current].perf_tegument, fields[17]);
            strcpy(records[current].pre_diagnosis, fields[18]);
            current++; // Vai para a próxima posição livre no array de retorno
        }
    }

    fclose(file); // Fecha o arquivo texto
    *total_count = current; // Retorna quantos preencheu pela variável de ponteiro
    return records; // Retorna a coleção alocada para o sistema inteiro usar
}

Patient* get_all_patients(int *total_count) {
    *total_count = 0; // Previne sujeira na variável de fora

    int count = get_cached_patient_count(); // Busca quantos tem listados no arquivo de contagem rápido
    if (count == 0) { // Se tiver 0 no cache
        return NULL; // Não precisa ler o arquivo CSV pesadão
    }

    FILE *file = fopen(PATIENT_FILE, "r"); // Abre leitura do CSV de pacientes
    if (file == NULL) { // Se não encontrou o arquivo
        return NULL; // Fim
    }

    // Aloca vetor dinâmico pro banco total
    Patient *patients = malloc(count * sizeof(Patient));
    if (patients == NULL) { // Se o sistema falhar em dar memória
        fclose(file);
        return NULL;
    }

    char line[512]; // Linha lida
    char *fields[7]; // Divisões
    int current = 0; // Quantos já iterou

    while (fgets(line, sizeof(line), file) && current < count) {
        if (line[0] == '\n' || line[0] == '\r') continue; // Pula linha em branco
        if (strncmp(line, "id;", 3) == 0) continue; // Pula os headers do CSV

        char line_copy[512];
        strcpy(line_copy, line); // copia os valores da linha para a variavel

        int cols = split_csv_line(line_copy, fields, 7); // Executa corte no delimitador
        if (cols < 7) continue; // Pula se tá incompleto

        // Armazena as fatias dentro do array, convertendo tipos básicos (int/float)
        patients[current].id = atoi(fields[0]);
        strcpy(patients[current].name, fields[1]);
        strcpy(patients[current].email, fields[2]);
        strcpy(patients[current].cpf, fields[3]);
        strcpy(patients[current].birth_date, fields[4]);
        patients[current].metrics.height = atof(fields[5]);
        patients[current].metrics.weight = atof(fields[6]);
        current++; // Vai para o próximo índice do array
    }

    fclose(file); // Fecha leitura
    *total_count = current; // Devolve tamanho preenchido
    return patients; // Envia pro frontend
}