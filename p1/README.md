# Gustavo Piroupo Neumann

## Estrutura do Projeto

O projeto está organizado em três diretórios principais:

- **compiler**: Responsável por converter código de alto nível (.lpn) para assembly (.asm)
- **assembler**: Converte código assembly (.asm) para código de máquina (.mem)
- **executor**: Executa o código de máquina (.mem) na arquitetura Neander

Cada componente possui sua própria estrutura de diretórios:
```
p1-compiladores/
├── compiler/
│   ├── bin/
│   ├── obj/
│   ├── output/
│   ├── src/
│   └── Makefile
├── assembler/
│   ├── bin/
│   ├── obj/
│   ├── output/
│   ├── src/
│   └── Makefile
├── executor/
│   ├── bin/
│   ├── obj/
│   ├── output/
│   ├── src/
│   └── Makefile
├── Makefile
└── program.lpn
```

## Como Compilar o Projeto

Para compilar todos os componentes do projeto, execute o seguinte comando na raiz do diretório:

```bash
make
```

Este comando irá compilar o compilador, o assembler e o executor.

## Como Executar o Projeto

1. Crie um arquivo `program.lpn` na raiz do projeto com seu código fonte. Exemplo:

```
PROGRAMA "calc":
INICIO
x = 5
y = 10
z = x * y
k = z - 2 + 3
RES k
FIM
```

2. Execute o pipeline completo com o comando:

```bash
make run
```

Este comando irá:
1. Compilar o código fonte utilizando o compilador, gerando um arquivo assembly em `compiler/output/output.asm`
2. Traduzir o código assembly utilizando o assembler, gerando um arquivo de código de máquina em `assembler/output/output.mem`
3. Executar o código de máquina no executor Neander, exibindo o resultado no terminal

## Limpeza

Para limpar os arquivos compilados e saídas geradas:

```bash
make clean
```

## Limitações

Atualmente, o compilador não implementa operações de divisão. A linguagem suporta apenas as operações de adição, subtração e multiplicação.
