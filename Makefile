# ==============================================================================
# Makefile para Compilação Modular
# UPE - Programação Imperativa
# ==============================================================================

# Compilador e Flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -Iinclude
LDFLAGS = -lgdi32 -luser32 -lkernel32

# Para ocultar o prompt de comando (console) ao rodar a janela principal,
# descomente a linha abaixo (adicione -mwindows):
# LDFLAGS += -mwindows

# Diretórios
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# Arquivos fonte e objetos correspondentes
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))
TARGET = $(BIN_DIR)/programa.exe

# Regra principal
.PHONY: all clean run

all: $(TARGET)

# Linkagem do executável final
$(TARGET): $(OBJS) | $(BIN_DIR)
	@echo [LINKING] Gerando executavel final $@...
	$(CC) $(OBJS) -o $@ $(LDFLAGS)
	@echo Compilado com sucesso! Executável gerado em: $(TARGET)

# Compilação dos arquivos fonte para objetos
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	@echo [COMPILING] Compilando $<...
	$(CC) $(CFLAGS) -c $< -o $@

# Criação das pastas de Build caso não existam
$(BIN_DIR) $(OBJ_DIR):
	@if not exist "$@" mkdir "$@"

# Limpeza dos arquivos compilados
clean:
	@echo [CLEANING] Removendo arquivos temporários de build...
	@if exist "$(OBJ_DIR)" rmdir /s /q "$(OBJ_DIR)"
	@if exist "$(BIN_DIR)" rmdir /s /q "$(BIN_DIR)"
	@echo Limpeza concluída!

# Compilar e executar o projeto automaticamente
run: all
	@echo [RUNNING] Executando o aplicativo...
	@$(TARGET)
