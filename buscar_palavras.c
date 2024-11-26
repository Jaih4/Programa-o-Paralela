#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define MAX_LINE_LENGTH 1024

char* inverterPalavra(const char* palavra) {
    int len = strlen(palavra);
    char* palavraInvertida = (char*)malloc((len + 1) * sizeof(char));
    for (int i = 0; i < len; i++) {
        palavraInvertida[i] = palavra[len - i - 1];
    }
    palavraInvertida[len] = '\0';
    return palavraInvertida;
}

int buscarPalavraNaMatriz(char** grid, int rows, int cols, const char* palavra) {
    cols--;
    int len = strlen(palavra);
    int found = 0;
    char* palavraInvertida = inverterPalavra(palavra);

    #pragma omp parallel sections
    {
        #pragma omp section
        {
            //horizontal
            for (int i = 0; i < rows; i++) {
                for (int j = 0; j < cols ; j++) {
                    int k;
                    for (k = 0; k < len; k++) {
                        if (j + k >= cols) {
                            if (grid[i][(j + k) % cols] != palavra[k] && grid[i][(j + k) % cols] != palavraInvertida[k]) {
                                break;
                            }
                        } else {
                            if (grid[i][j + k] != palavra[k] && grid[i][j + k] != palavraInvertida[k]) {
                                break;
                            }
                        }
                    }
                    if (k == len) {
                        #pragma omp critical
                        {
                            printf("Palavra '%s' encontrada na linha %d, comecando na coluna %d (horizontal)\n", palavra, i + 1, j + 1);
                        }
                        found = 1;
                    }
                }
            }
        }

        #pragma omp section
        {
            // Vertical
            for (int i = 0; i < rows; i++) {
                for (int j = 0; j < cols; j++) {
                    int k;
                    for (k = 0; k < len; k++) {
                        if (i+k>=rows){
                            if (grid[(i + k) % rows][j] != palavra[k] && grid[(i + k) % rows][j] != palavraInvertida[k]) {
                                break;
                            }
                        }else{
                            if (grid[i + k][j] != palavra[k] && grid[i + k][j] != palavraInvertida[k]) {
                                break;
                            }
                        }
                    }
                    if (k == len) {
                        #pragma omp critical
                        {
                            printf("Palavra '%s' encontrada na coluna %d, comecando na linha %d (vertical)\n", palavra, j+1, i+1);
                        }
                        found = 1;
                    }
                }
            }
        }

        #pragma omp section
        {
            // diagonal
            for (int i = 0; i < rows-len; i++) {
                for (int j = 0; j < cols-len; j++) {
                    int k;
                    for (k = 0; k < len; k++) {
                        
                        if (grid[i + k][j + k] != palavra[k] && grid[i + k][j + k] != palavraInvertida[k]) {
                            break;
                        }
                    }
                    if (k == len) {
                        #pragma omp critical
                        {
                            printf("Palavra '%s' encontrada na diagonal, comecando na linha %d, coluna %d\n", palavra, i+1, j+1);
                        }
                        found = 1;
                    }
                }
            }
        }
    }

    free(palavraInvertida);
    return found;
}

void removerEspacos(char *texto) {
    int i = 0, j = 0;
    while (texto[i]) {
        if (texto[i] != ' ') {
            texto[j++] = texto[i];
        }
        i++;
    }
    texto[j] = '\0';
}

char** extrairSubstrings(const char* last_line, int* count) {
    int len = strlen(last_line);
    int start = 0;
    *count = 0;

    char** substrings = (char**) malloc(len * sizeof(char*));
    if (substrings == NULL) {
        printf("Erro de alocação de memória para substrings.\n");
        return NULL;
    }

    for (int i = 0; i <= len; i++) {
        if (last_line[i] == ',' || last_line[i] == '\0') {
            int substring_length = i - start;
            if (substring_length > 0) {
                substrings[*count] = (char*) malloc((substring_length + 1) * sizeof(char));
                if (substrings[*count] == NULL) {
                    printf("Erro de alocação de memória para substring.\n");
                    for (int j = 0; j < *count; j++) {
                        free(substrings[j]);
                    }
                    free(substrings);
                    return NULL;
                }
                strncpy(substrings[*count], &last_line[start], substring_length);
                substrings[*count][substring_length] = '\0';
                (*count)++;
            }
            start = i + 1;
        }
    }

    char** ajustado = (char**) realloc(substrings, (*count) * sizeof(char*));
    if (ajustado == NULL && *count > 0) {
        printf("Erro ao reajustar memória para substrings.\n");
        for (int j = 0; j < *count; j++) {
            free(substrings[j]);
        }
        free(substrings);
        return NULL;
    }
    return ajustado ? ajustado : substrings;
}

void liberarSubstrings(char** substrings, int count) {
    for (int i = 0; i < count; i++) {
        free(substrings[i]);
    }
    free(substrings);
}

int main(int argc, char *argv[]) {

    if (argc < 1) {
        fprintf(stderr, "Uso: %s <arquivo> <numero_de_threads>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int num_threads = atoi(argv[2]);
    if (num_threads <= 0) {
        fprintf(stderr, "Erro: número de threads inválido.\n");
        return EXIT_FAILURE;
    }

    omp_set_num_threads(num_threads); 

    FILE *file = fopen(argv[1], "r");
    if (!file) {
        fprintf(stderr, "Erro ao abrir o arquivo: %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    char buffer[MAX_LINE_LENGTH];
    char last_line[MAX_LINE_LENGTH];
    int i = 0, j, rows = 0, cols = 0, end = 0;

    while (fgets(buffer, sizeof(buffer), file)) {
        if (buffer[0] == '\n' && rows > 0) {
            break;
        }

        if (buffer[0] != '\n') {
            removerEspacos(buffer);
            int len = strlen(buffer);
            if (len > cols) {
                cols = len;
            }
            rows++;
        }
    }
    while (fgets(buffer, sizeof(buffer), file)) {
        strcpy(last_line, buffer);
        removerEspacos(last_line);
    }   
    fclose(file); 
    FILE *file1 = fopen("matriz.txt", "r");
    if (file1 == NULL) {
        perror("Erro ao reabrir o arquivo");
        return -1;
    }

    char** grid = (char**)malloc(rows * sizeof(char*));
    for (int i = 0; i < rows; i++) {
        grid[i] = (char*)malloc(cols + 1);
    }
    i = 0;
    while (fgets(buffer, sizeof(buffer), file1)) {
        if (buffer[0] != '\n') {
            removerEspacos(buffer);
            strncpy(grid[i], buffer, cols);
            grid[i][cols] = '\0';
            i++;
        }else{
            break;
        }
    }
    fclose(file1);
    
    int count = 0;
    char** substrings = extrairSubstrings(last_line, &count);
    for (int i = 0; i < count; i++) {
        printf("Procurando pela palavra: %s\n", substrings[i]);
        if (!buscarPalavraNaMatriz(grid, rows, cols, substrings[i])) {
            printf("Palavra '%s' nao encontrada.\n", substrings[i]);
        }
    }
    liberarSubstrings(substrings, count);
    free(grid);
    return 0;
}
