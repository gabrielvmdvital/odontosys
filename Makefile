# ==============================================================================
# Makefile para Compilação Modular
# UPE - Programação Imperativa
# ==============================================================================

# Compilador e Flags
CC = gcc

# Configurações do GTK4
GTK_CFLAGS = $(shell pkg-config --cflags gtk4)
GTK_LIBS = $(shell pkg-config --libs gtk4)

CFLAGS = -Wall -Wextra -std=c99 -Iinclude $(GTK_CFLAGS)
LDFLAGS = -lgdi32 -luser32 -lkernel32 $(GTK_LIBS)

# Para ocultar o prompt de comando (console) ao rodar a janela principal,
# descomente a linha abaixo (adicione -mwindows):
# LDFLAGS += -mwindows

# Diretórios
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin
TESTS_DIR = test

# Arquivos fonte e objetos correspondentes do app
SRCS = $(filter-out $(SRC_DIR)/main_test.c, $(wildcard $(SRC_DIR)/*.c))
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))
TARGET = $(BIN_DIR)/programa.exe

# Arquivos fonte e objetos correspondentes de testes
TEST_SRCS = $(wildcard $(TESTS_DIR)/*.c)
TEST_OBJS = $(patsubst $(TESTS_DIR)/%.c, $(OBJ_DIR)/%.o, $(TEST_SRCS))
TEST_TARGET = $(BIN_DIR)/testes.exe

# Objetos da aplicação sem o main.o (para evitar duplicidade de função main nos testes)
APP_OBJS = $(filter-out $(OBJ_DIR)/main.o, $(OBJS))

# Regras .PHONY
.PHONY: all clean run test

# Regra principal (compila o programa)
all: $(TARGET)

# Linkagem do executável final
$(TARGET): $(OBJS) | $(BIN_DIR)
	@echo [LINKING] Gerando executavel final $@...
	$(CC) $(OBJS) -o $@ $(LDFLAGS)
	@echo Compilado com sucesso! Executavel gerado em: $(TARGET)

# Compilação dos arquivos fonte do app para objetos
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	@echo [COMPILING] Compilando $<...
	$(CC) $(CFLAGS) -c $< -o $@

# Compilação dos arquivos de testes para objetos
$(OBJ_DIR)/%.o: $(TESTS_DIR)/%.c | $(OBJ_DIR)
	@echo [COMPILING TEST] Compilando $<...
	$(CC) $(CFLAGS) -I$(TESTS_DIR) -c $< -o $@

# Linkagem e execução da suite de testes
test: all $(TEST_TARGET)
	@echo [RUNNING TESTS] Executando testes unitarios...
	@$(TEST_TARGET)

$(TEST_TARGET): $(TEST_OBJS) $(APP_OBJS) | $(BIN_DIR)
	@echo [LINKING TESTS] Gerando executavel de testes $@...
	$(CC) $(TEST_OBJS) $(APP_OBJS) -o $@ $(LDFLAGS)

# Criação das pastas de Build caso não existam
$(BIN_DIR) $(OBJ_DIR):
	@mkdir -p "$@"

# Limpeza dos arquivos compilados
clean:
	@echo [CLEANING] Removendo arquivos temporarios de build...
	@rm -rf "$(OBJ_DIR)"
	@rm -rf "$(BIN_DIR)"
	@echo Limpeza concluida!

# Compilar e executar o projeto automaticamente
run: all
	@echo [RUNNING] Executando o aplicativo...
	@$(TARGET)

