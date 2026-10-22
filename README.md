# ConstrucaoCompiladores-ACH2087
Repositório para a publicação e desenvolvimento do Analisador Léxico Cobol

# Analisador Léxico para COBOL

Projeto desenvolvido em C para a disciplina de Compiladores. O programa realiza a análise léxica de arquivos COBOL em formato fixo, identificando tokens e reportando erros encontrados durante a análise.

## Funcionalidades
- Identificação de palavras reservadas, identificadores, números, literais e operadores.
- Tratamento de comentários e linhas de continuação.
- Identificação de caracteres inválidos e emissão de avisos e erros.
- Exibição dos tokens com suas respectivas posições no código.

## Compilação

```bash
gcc cobol_lexer.c -o cobol_lexer
```

## Uso

```bash
cobol_lexer -d ./codigo_cobol
```

## Ferramentas

* Linguagem C
* Auxilio de IA (ChatGPT e Claude) para criação de arquivos de teste, auxilio na correção de erros e compreensão nas regras do COBOL


## Autores

Gabriel Chen Hirata 13747946
Jonatan Zhan 15510806
Thomas Delfs 13837175
