# Construção de Compiladores — ACH2087

Repositório destinado ao desenvolvimento do projeto da disciplina **ACH2087 — Construção de Compiladores**, com foco na implementação de um compilador para a linguagem COBOL.

O projeto está sendo desenvolvido em etapas, começando pela análise léxica, responsável por ler o código-fonte e identificar os tokens presentes na linguagem.

## Sobre o projeto

O objetivo do trabalho é colocar em prática os conceitos apresentados durante a disciplina, desenvolvendo os componentes necessários para a construção de um compilador.

Nesta primeira etapa, foi desenvolvido um analisador léxico em linguagem C, capaz de ler arquivos COBOL, reconhecer diferentes tipos de tokens e identificar possíveis erros léxicos.

## Analisador Léxico

O analisador léxico realiza a leitura do código-fonte COBOL e classifica os elementos encontrados.

Entre os elementos reconhecidos estão:

- Palavras reservadas da linguagem COBOL;
- Identificadores e nomes de variáveis;
- Constantes numéricas e alfanuméricas;
- Operadores aritméticos e relacionais;
- Símbolos e caracteres especiais;
- Comentários e indicadores do formato fixo COBOL.

O programa também apresenta mensagens de erro e avisos quando encontra elementos que não seguem as regras léxicas estabelecidas.

## Estrutura do repositório

Os principais arquivos do projeto são:

| Arquivo | Descrição |
|---|---|
| `cobol_lexer.c` | Código-fonte do analisador léxico |
| `exemplo.cob` | Arquivo de exemplo para testes |
| `prog001` a `prog007` | Programas COBOL utilizados para testar o analisador |
| `01` a `10` | Casos de teste com erros e avisos léxicos |
| `Relatório Analisador Léxico.pdf` | Relatório da implementação |

## Compilação e execução

Para compilar o analisador, é necessário ter o GCC instalado.

No terminal, dentro da pasta do projeto, execute:

```bash
gcc -o cobol_lexer cobol_lexer.c
```

Para analisar um arquivo COBOL:

**Linux/macOS:**
```bash
./cobol_lexer exemplo.cob
```

**Windows:**
```bash
.\cobol_lexer.exe exemplo.cob
```

Também é possível executar os demais arquivos `.cob` presentes no repositório.

## Testes

Foram criados arquivos de teste para verificar o comportamento do analisador em diferentes situações, incluindo códigos válidos e casos com erros léxicos.

Os testes incluem caracteres inválidos, identificadores fora do padrão, literais não terminados e problemas relacionados ao formato do código COBOL.

Esses arquivos permitem verificar se o analisador reconhece corretamente os tokens e apresenta as mensagens de erro esperadas.

## Autores

| Nome | NUSP |
|---|---|
| Jonatan | 15510806 |

**Disciplina:** ACH2087 — Construção de Compiladores  
**Instituição:** Universidade de São Paulo (USP)  
**Ano:** 2026

## Documentação

Mais detalhes sobre a implementação e os testes realizados podem ser encontrados no arquivo [Relatório Analisador Léxico.pdf](Relat%C3%B3rio%20Analisador%20L%C3%A9xico.pdf), disponível neste repositório.
