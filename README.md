# 🦷 OdontoSys

> Sistema de apoio à triagem odontológica e geração de pré-diagnósticos ortodônticos desenvolvido em **C** utilizando **GTK4**.

---

# 📑 Sumário

* 📌 Arquitetura do Sistema
* 🛠 Configuração do Ambiente
* 🚀 Compilação e Execução
* 🔄 Fluxo de Utilização
* 📂 Estrutura do Projeto
* 👨‍💻 Equipe de Desenvolvimento

---

# 🏗 Arquitetura do Sistema

O projeto segue uma arquitetura em **duas camadas**, separando a interface gráfica da lógica de negócio.

## 🎨 Front-End

**Arquivo Principal**

```text
src/gui.c
src/gui/*
```

Responsável por toda a interface gráfica do sistema utilizando a biblioteca **GTK4**.

### Funcionalidades

* ✅ Construção das telas da aplicação
* ✅ Navegação utilizando `GtkStack`
* ✅ Organização visual através de `GtkGrid`
* ✅ Captura das entradas do usuário
* ✅ Envio das informações para o Back-End através de estruturas (`gpointer`)

### Telas Desenvolvidas

* 🔐 Login
* 📊 Dashboard
* 📋 Prontuários
* 👥 Cadastro de Pacientes
* 🧠 Pré-Diagnóstico

---

## ⚙️ Back-End

Arquivos responsáveis pela lógica da aplicação:

```text
src/clinical.c
src/database.c
src/logs.c
src/dentist.c
src/patient.c
```

### Funções

* ✔️ Validação dos dados recebidos
* ✔️ Processamento das informações clínicas
* ✔️ Execução da árvore de decisão ortodôntica
* ✔️ Geração do pré-diagnóstico
* ✔️ Registro de eventos em arquivos de log
* ✔️ Persistência das fichas em disco

---

# 🛠 Configuração do Ambiente

O ambiente oficial utilizado para desenvolvimento no Windows é:

* 🖥 **MSYS2**
* ⚙️ **UCRT64**
* 🧩 **GTK4**
* 🔨 **GCC**

---

## 1️⃣ Instalação do MSYS2

Faça o download do instalador oficial:

🔗 https://www.msys2.org/

Após instalar, abra o terminal:

```text
MSYS2 UCRT64
```

---

## 2️⃣ Atualização do Sistema

```bash
pacman -Syu --noconfirm
```

---

## 3️⃣ Instalação do GCC

```bash
pacman -S --needed base-devel mingw-w64-ucrt-x86_64-toolchain --noconfirm
```

---

## 4️⃣ Instalação do GTK4

```bash
pacman -S mingw-w64-ucrt-x86_64-gtk4 --noconfirm
```

---

## 5️⃣ Instalação do pkg-config

```bash
pacman -S mingw-w64-ucrt-x86_64-pkg-config --noconfirm
```

---

# 🚀 Compilação do Projeto

## Acesse o diretório

```bash
cd "/c/Users/kuanv/OneDrive/Desktop/Prototipo 1 interface/odontosys"
```

---

## Compile o sistema

```bash
gcc \
src/gui.c \
src/main.c \
src/app.c \
src/logs.c \
src/database.c \
src/clinical.c \
src/dentist.c \
src/patient.c \
-o odontosys_app.exe \
-Iinclude \
$(pkg-config --cflags --libs gtk4)
```

---

## Execute o programa

```bash
./odontosys_app.exe
```

---

## 📦 Geração da Build para Distribuição

Para gerar uma versão final pronta para distribuição em computadores que não possuem o ambiente MSYS2 configurado:

1. **Compile o executável** garantindo que as dependências estejam vinculadas corretamente (o `Makefile` facilita esse processo, use `mingw32-make`).
2. **Execute o script de empacotamento** via PowerShell. Ele copiará o executável e todas as DLLs e assets do GTK necessários:
   ```powershell
   .\scripts\package_dist.ps1
   ```
   *Isto criará a pasta `dist/OdontoSys` com todos os arquivos isolados.*
3. **Gere o instalador** utilizando o [Inno Setup](https://jrsoftware.org/isinfo.php). Abra o arquivo `odontosys_setup.iss` no aplicativo Inno Setup e clique no botão **Compile**.
   *O arquivo do instalador final (ex: `Instalador_OdontoSys_v1.0.exe`) será gerado na raiz do projeto!*

---

# 🔄 Fluxo de Utilização
---
## 🔑 0. Pré-requisito: Cadastro do Dentista (feito pelo Admin)

Antes de um dentista conseguir fazer login, ele precisa ter sido cadastrado no sistema. Esse cadastro só pode ser feito por um usuário **Administrador**.

### Login do Administrador

O sistema já vem com um usuário admin criado automaticamente na primeira execução (`database_init()` cria esse usuário no `dentists.csv`).

* **Usuário:** `admin`
* **Senha:** `admin`

### Cadastrando um novo Dentista

Ao logar como admin, o menu de **Cadastrar Dentista** fica disponível (visível apenas para esse perfil). Preencha:

- Nome
- Usuário (login)
- CPF
- Senha

➡ Clique em **Salvar**.


## 🔐 1. Login

O profissional informa:

* Usuário
* Senha

➡ Clique em **Entrar**.

---

## 📊 2. Dashboard

Após a autenticação é exibido o painel principal com as opções:

* 📁 Consultar Prontuários
* 👥 Cadastrar Novo Paciente

Selecione:

**👥 Cadastrar Paciente**

---

## 👤 3. Cadastro do Paciente

Preencha as informações básicas:

* Nome
* CPF
* Telefone
* Data de Nascimento
* E-mail (opcional)

Ao clicar em:

**💾 Salvar Ficha**

o sistema:

* valida os dados;
* registra o evento em log;
* encaminha automaticamente para a próxima etapa.

---

## 🧠 4. Pré-Diagnóstico

Nesta etapa são preenchidos os parâmetros clínicos coletados nos exames que serão utilizados pela árvore de decisão ortodôntica.
Lembrar de utilizar pontos ao invés de vírgulas nos números decimais.

Após o preenchimento, clique em:

**🧠 Gerar Pré-Diagnóstico**

---

## 📄 5. Resultado Clínico

O sistema executa toda a lógica clínica e apresenta o resultado em um componente:

```text
GtkFrame
```
---

## ♻️ 6. Novo Atendimento

Após finalizar o diagnóstico, o botão passa a ser:

```text
✨ Concluir e Novo Cadastro
```

Ao clicar:

* 🧹 Todos os campos são limpos
* 🗑 A memória temporária é reinicializada
* 👤 Um novo paciente pode ser cadastrado imediatamente

---

## ⬅️ Retorno ao Painel

Em qualquer momento é possível interromper o cadastro clicando em:

```text
⬅ Voltar
```
---

# 📂 Estrutura do Projeto

```text
odontosys/
│
├── include/
│
├── src/
│   ├── app.c
│   ├── clinical.c
│   ├── database.c
│   ├── dentist.c
│   ├── gui.c
│   ├── logs.c
│   ├── main.c
│   └── patient.c
│
├── README.md
│
└── odontosys_app.exe
```

---

# 🧰 Tecnologias Utilizadas

| Tecnologia | Finalidade                  |
| ---------- | --------------------------- |
| C          | Linguagem principal         |
| GTK4       | Interface gráfica           |
| GCC        | Compilador                  |
| MSYS2      | Ambiente de desenvolvimento |
| UCRT64     | Toolchain para Windows      |
| Make       | Gerenciador de compilação   |

---

# 👨‍💻 Equipe de Desenvolvimento

<table>
<tr>
<td align="center">

### 👨‍💻  Gabriel Victor Marques de Oliveira Vital

Desenvolvimento

</td>

<td align="center">

### 👨‍💻 Kauan Victor de Moura Dias

Desenvolvimento

</td>

<td align="center">

### 👨‍💻 Milena Rafaela Duarte Farias de Lima

Desenvolvimento

</td>

<td align="center">

### 👨‍💻 Sofia Ruas Machado Marques 

Desenvolvimento

</td>

</tr>

<td align="center">

### 👨‍💻 Vicente Luiz Barros de Souza

Desenvolvimento

</td>

</tr>
</table>

