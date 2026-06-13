# Diagrama de Blocos do Sistema: OdontoSys

Este documento apresenta a arquitetura e o diagrama de blocos básico do **OdontoSys**, descrevendo como os módulos se organizam em camadas e como as informações trafegam entre a interface de usuário, as regras de negócio e os arquivos físicos de armazenamento.

---

## Diagrama de Blocos Arquitetural

O sistema é dividido em **4 camadas principais**:

![Diagrama de Blocos Arquitetural](diagrama_odontosys.svg)

---

## Descrição das Camadas e Componentes

### 1. Camada de Apresentação (GUI)
* **Componente:** `gui.c` / `gui.h`
* **Função:** Gerencia a janela gráfica Win32 do Windows. Ela desenha os botões, caixas de texto e tabelas. Ela captura os cliques do mouse do dentista/paciente e lê ou altera as informações contidas no estado global (`AppState`).

### 2. Camada de Lógica & Estado (Core)
* **`main.c`:** Inicializa todas as peças do sistema em ordem (Métricas, Logs, Estado e Janela Gráfica) e gerencia o loop de mensagens da aplicação.
* **`app.c` / `app.h` (`AppState`):** Centraliza o estado atual do programa em uma única estrutura na memória (quem é o usuário logado, qual a aba atual, etc.).
* **`clinical.c` / `clinical.h`:** Contém as lógicas de negócio e as fórmulas para o pré-diagnóstico odontológico e executa a classificação baseada nos parâmetros clínicos inseridos.

### 3. Camada de Dados (Database)
* **Componente:** `database.c` / `database.h`
* **Função:** É o motor de persistência. Traduz as estruturas de memória do C (`Dentist`, `Patient` e `ClinicalRecord`) para linhas de arquivos de texto formatados em **CSV** delimitados por ponto-e-vírgula (`;`). As chaves primárias e estrangeiras são baseadas em inteiros sem sinal de 64 bits (`uint64_t`).
* **`database/dentists.csv`:** Armazena o cadastro e as credenciais de login dos profissionais dentistas (`dentist_id`).
* **`database/pacientes.csv`:** Armazena exclusivamente o cadastro básico de pacientes (`patient_id`).
* **`database/prontuarios.csv`:** Armazena o prontuário de atendimentos e os diagnósticos gerados vinculados aos IDs do paciente e dentista (`clinical_id`).

### 4. Camada Auxiliar (Logs)
* **Componente:** `logs.c` / `logs.h`
* **Função:** Grava de forma segura e padronizada (com carimbos de data/hora) todas as ações relevantes do sistema.
* **`logs/app.log`:** Arquivo físico que documenta desde inicializações com sucesso até alertas de segurança ou falhas críticas de acesso ao disco.
