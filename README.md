# Construção de Compiladores — ACH2087

![Linguagem C](https://img.shields.io/badge/Linguagem-C-blue?style=for-the-badge&logo=c)
![COBOL](https://img.shields.io/badge/COBOL-An%C3%A1lise%20L%C3%A9xica-orange?style=for-the-badge)
![Status](https://img.shields.io/badge/Status-Em%20Desenvolvimento-yellow?style=for-the-badge)
![GitHub](https://img.shields.io/badge/Plataforma-GitHub-black?style=for-the-badge&logo=github)

## 📌 Sobre o Projeto

Este repositório é destinado ao desenvolvimento de um **compilador para a linguagem COBOL**, como parte das atividades da disciplina **ACH2087 — Construção de Compiladores**.

O projeto tem como objetivo aplicar conceitos fundamentais de construção de compiladores, incluindo análise léxica, análise sintática, análise semântica e demais etapas envolvidas no processamento de linguagens de programação.

Atualmente, o repositório apresenta a implementação de um **analisador léxico para COBOL**, desenvolvido em linguagem C, acompanhado de programas de exemplo, casos de teste e documentação técnica.

## 🎯 Objetivos

- Compreender as etapas de construção de um compilador.
- Desenvolver um analisador léxico para programas COBOL.
- Identificar e classificar os componentes léxicos da linguagem.
- Reconhecer palavras reservadas, identificadores, operadores e literais.
- Detectar e reportar erros léxicos.
- Validar a implementação por meio de programas e casos de teste.
- Evoluir o projeto para as demais etapas do processo de compilação.

## 🛠️ Tecnologias Utilizadas

| Tecnologia | Aplicação |
|---|---|
| C | Implementação do analisador léxico |
| COBOL | Linguagem analisada pelo projeto |
| GCC | Compilação do código-fonte C |
| Git | Controle de versões |
| GitHub | Hospedagem e colaboração |
| Visual Studio Code | Ambiente de desenvolvimento opcional |

## ⚙️ Funcionalidades do Analisador Léxico

O analisador léxico é responsável por percorrer o código-fonte COBOL e identificar unidades léxicas, denominadas **tokens**.

A implementação contempla:

- **Palavras reservadas:** reconhecimento de instruções e elementos da linguagem COBOL.
- **Identificadores:** identificação de nomes de variáveis e outros elementos definidos pelo programador.
- **Literais:** reconhecimento de valores numéricos e alfanuméricos.
- **Operadores:** identificação de operadores aritméticos e relacionais.
- **Pontuação:** reconhecimento de pontos, vírgulas, parênteses e outros símbolos.
- **Formato fixo:** processamento das regiões de código conforme as posições das colunas COBOL.
- **Comentários:** tratamento de linhas de comentário e comentários em linha.
- **Diagnóstico:** emissão de avisos e mensagens de erro com linha e coluna.

### Classificação dos Tokens

| Tipo | Descrição |
|---|---|
| `KEYWORD` | Palavra reservada |
| `IDENT` | Identificador |
| `NUMBER` | Literal numérico |
| `STRING` | Literal alfanumérico |
| `PICTURE` | Especificação de formato de dados |
| `OPERATOR` | Operador |
| `PERIOD` | Ponto |
| `COMMA` | Vírgula |
| `SEMICOLON` | Ponto e vírgula |
| `LPAREN` / `RPAREN` | Parênteses |
| `ERROR` | Token inválido |

## 📁 Estrutura do Repositório

```text
ConstrucaoCompiladores-ACH2087/
│
├── cobol_lexer.c
├── cobol_lexer.exe
├── exemplo.cob
│
├── prog001_divisoes.cob
├── prog002_operacoes.cob
├── prog003_calculadora.cob
├── prog004_folha_pgto.cob
├── prog005_strings.cob
├── prog006_files.cob
├── prog007_loja.cob
│
├── 01_indicador_invalido.cob
├── 02_caractere_invalido_arroba.cob
├── 03_literal_nao_terminado.cob
├── 04_identificador_termina_hifen.cob
├── 05_identificador_longo.cob
├── 06_caractere_invalido_interrogacao.cob
├── 07_caractere_invalido_cifrao.cob
├── 08_caractere_invalido_til.cob
├── 09_literal_numerico_longo_aviso.cob
├── 10_linha_curta_aviso.cob
│
├── Relatório Analisador Léxico.pdf
└── README.md
```

### Principais Arquivos

- **`cobol_lexer.c`** — código-fonte do analisador léxico implementado em C.
- **`exemplo.cob`** — arquivo COBOL para testes iniciais.
- **`prog001` a `prog007`** — exemplos de programas COBOL utilizados na análise.
- **`01` a `10`** — casos de teste relacionados a erros e avisos léxicos.
- **`Relatório Analisador Léxico.pdf`** — documentação técnica da etapa de análise léxica.

## 🚀 Como Executar

### 1. Clonar o repositório

```bash
git clone https://github.com/arcahyter/ConstrucaoCompiladores-ACH2087.git
```

Acesse o diretório:

```bash
cd ConstrucaoCompiladores-ACH2087
```

### 2. Compilar o analisador

É necessário possuir um compilador C, como o GCC.

```bash
gcc -Wall -Wextra -o cobol_lexer cobol_lexer.c
```

### 3. Executar um programa COBOL

No Linux ou macOS:

```bash
./cobol_lexer exemplo.cob
```

No Windows:

```bash
.\cobol_lexer.exe exemplo.cob
```

### 4. Executar com linhas de depuração

O parâmetro `-d` permite incluir linhas de depuração identificadas pelo indicador `D` na coluna 7.

```bash
./cobol_lexer -d exemplo.cob
```

## 🧪 Testes e Validação

O repositório contém arquivos destinados à validação do analisador léxico, abrangendo situações corretas e entradas inválidas.

Entre os cenários de teste estão:

| Arquivo | Cenário |
|---|---|
| `01_indicador_invalido.cob` | Indicador inválido |
| `02_caractere_invalido_arroba.cob` | Caractere `@` inválido |
| `03_literal_nao_terminado.cob` | Literal sem fechamento |
| `04_identificador_termina_hifen.cob` | Identificador terminado em hífen |
| `05_identificador_longo.cob` | Identificador acima do limite |
| `06_caractere_invalido_interrogacao.cob` | Caractere `?` inválido |
| `07_caractere_invalido_cifrao.cob` | Caractere `$` inválido |
| `08_caractere_invalido_til.cob` | Caractere `~` inválido |
| `09_literal_numerico_longo_aviso.cob` | Aviso de literal numérico longo |
| `10_linha_curta_aviso.cob` | Aviso de linha curta |

Para executar um teste:

```bash
./cobol_lexer 01_indicador_invalido.cob
```

O analisador apresenta os tokens identificados e, quando aplicável, mensagens de diagnóstico contendo a localização e a descrição do problema encontrado.

## 📊 Etapas do Desenvolvimento

| Etapa | Situação |
|---|---|
| Definição e estudo da linguagem COBOL | Em desenvolvimento |
| Implementação do analisador léxico | Implementada |
| Criação de casos de teste léxicos | Implementada |
| Documentação da análise léxica | Disponível |
| Análise sintática | Etapa futura |
| Análise semântica | Etapa futura |
| Geração de código | Etapa futura |

O projeto está sujeito a alterações conforme o avanço das atividades da disciplina.

## 📚 Documentação

O documento **[Relatório Analisador Léxico.pdf](https://github.com/arcahyter/ConstrucaoCompiladores-ACH2087/blob/main/Relat%C3%B3rio%20Analisador%20L%C3%A9xico.pdf)** apresenta a documentação específica da etapa de análise léxica.

Os arquivos de exemplo e os casos de teste disponibilizados no repositório complementam o estudo e a validação da implementação.

## 🤝 Contribuições

O desenvolvimento do projeto é realizado de forma colaborativa, utilizando Git e GitHub para gerenciamento de versões.

Para contribuir:

1. Crie uma branch para suas alterações.
2. Implemente ou modifique os arquivos necessários.
3. Realize testes para verificar o funcionamento.
4. Registre suas alterações por meio de commits.
5. Abra um Pull Request para revisão e integração.

As contribuições devem respeitar os objetivos e os requisitos estabelecidos para o projeto acadêmico.

## 👨‍💻 Desenvolvimento

Projeto acadêmico da disciplina **ACH2087 — Construção de Compiladores**.

Repositório mantido por [arcahyter](https://github.com/arcahyter), com participação dos colaboradores registrados no histórico do GitHub.

### Colaboradores

A lista de participantes que contribuíram com alterações integradas ao repositório pode ser consultada em:

[Visualizar contribuidores do projeto](https://github.com/arcahyter/ConstrucaoCompiladores-ACH2087/graphs/contributors)

---

**Construção de Compiladores — ACH2087**

*Projeto acadêmico dedicado ao estudo e à implementação das etapas de construção de um compilador para COBOL.*
