Compile o projeto com:
   ```sh
   make
   ```

Execute o programa:
   ```sh
   make run
   ```

Para limpar:
   ```sh
   make clean
   ```

Ou compile o projeto com todos os arquivos na mesma pasta utilizando:
   ```sh
   gcc -o assembler main.c lexer.c parser.c -Wall
   
   ./assembler test_code.txt output.mem
   ```
   
Uso:
   ```sh
   ./assembler {arquivo de entrada} {arquivo de saida}
   ```
