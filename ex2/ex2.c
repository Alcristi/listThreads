#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#define SIZE 3
#define RED "\e[31m"
#define RESET "\e[0m"

/*
    -  O jogo é executado na thread principal, após a criação das outras threads
    - Foi utilizado o mutex para atribuir valor  variável result, pois ela é a zona crítica
    onde poderia haver problema de concorrência
*/

// Estrutura para armazenar o tabuleiro do jogo e o resultado
typedef struct
{
    char board[SIZE][SIZE];
    int result; // 0: sem vencedor, 1: jogador 1 vence, 2: jogador 2 vence, 3: Velha
    pthread_mutex_t lock;
} GameState;

// Função para verificar linhas e colunas

void *check_lines(void *arg)
{
    GameState *state = (GameState *)arg;

    while (1)
    {
        pthread_mutex_lock(&state->lock);

        // Verifica as linhas
        for (int i = 0; i < SIZE; i++)
        {
            if (state->result == 0 && state->board[i][0] != '_' && state->board[i][0] == state->board[i][1] && state->board[i][1] == state->board[i][2])
            {
                if (state->board[i][0] == 'X')
                    state->result = 1;
                else if (state->board[i][0] == 'O')
                    state->result = 2;
                pthread_mutex_unlock(&state->lock);
                pthread_exit(NULL);
            }
        }

        // Verifica as colunas
        for (int i = 0; i < SIZE; i++)
        {
            if (state->result == 0 && state->board[0][i] != '_' && state->board[0][i] == state->board[1][i] && state->board[1][i] == state->board[2][i])
            {
                if (state->board[0][i] == 'X')
                    state->result = 1;
                else if (state->board[0][i] == 'O')
                    state->result = 2;
                pthread_mutex_unlock(&state->lock);
                pthread_exit(NULL);
            }
        }

        pthread_mutex_unlock(&state->lock);

        if (state->result != 0)
            break;
    }

    pthread_exit(NULL);
}

// Função para verificar as diagonais
void *check_diagonals(void *arg)
{
    GameState *state = (GameState *)arg;

    while (1)
    {
        pthread_mutex_lock(&state->lock);

        // Verifica a primeira diagonal (canto superior esquerdo para canto inferior direito)
        if (state->result == 0 && state->board[0][0] != '_' && state->board[0][0] == state->board[1][1] && state->board[1][1] == state->board[2][2])
        {
            if (state->board[0][0] == 'X')
                state->result = 1;
            else if (state->board[0][0] == 'O')
                state->result = 2;
            pthread_mutex_unlock(&state->lock);
            pthread_exit(NULL);
        }

        // Verifica a segunda diagonal (canto superior direito para canto inferior esquerdo)
        if (state->board[0][2] != '_' && state->board[0][2] == state->board[1][1] && state->board[1][1] == state->board[2][0])
        {
            if (state->board[0][2] == 'X')
                state->result = 1;
            else if (state->board[0][2] == 'O')
                state->result = 2;
            pthread_mutex_unlock(&state->lock);
            pthread_exit(NULL);
        }

        pthread_mutex_unlock(&state->lock);

        if (state->result != 0)
            break;
    }

    pthread_exit(NULL);
}

// Função para verificar se o jogo terminou em empate
void *check_draw(void *arg)
{
    GameState *state = (GameState *)arg;

    while (1)
    {
        pthread_mutex_lock(&state->lock);

        int is_draw = 1;
        for (int i = 0; i < SIZE; i++)
        {
            for (int j = 0; j < SIZE; j++)
            {
                if (state->board[i][j] == '_')
                {
                    is_draw = 0;
                    break;
                }
            }
            if (!is_draw)
                break;
        }

        if (is_draw && state->result == 0)
        {
            state->result = 3;
            pthread_mutex_unlock(&state->lock);
            pthread_exit(NULL);
        }

        pthread_mutex_unlock(&state->lock);

        if (state->result != 0)
            break;
    }

    pthread_exit(NULL);
}

// Função para imprimir o tabuleiro do jogo
void print_board(GameState *state)
{
    printf("\nTabuleiro:\n ");
    for (int j = 0; j < SIZE; j++)
        printf(" %d", j);
    printf("\n");

    for (int i = 0; i < SIZE; i++)
    {
        printf("%d ", i);
        for (int j = 0; j < SIZE; j++)
            printf("%c ", state->board[i][j]);
        printf("\n");
    }
}

// Função para controlar o jogo
void play_game(GameState *state)
{
    int turn = 0; // 0 para jogador 1 (X), 1 para jogador 2 (O)
    int moves = 0;

    while (state->result == 0 && moves < SIZE * SIZE)
    {
        int row, col;
        char mark = (turn == 0) ? 'X' : 'O';

        print_board(state);

        printf("\nJogador %d (%c), informe sua jogada (linha e coluna): ", turn + 1, mark);
        scanf("%d %d", &row, &col);
        if (row >= 0 && row < SIZE && col >= 0 && col < SIZE && state->board[row][col] == '_')
        {

            pthread_mutex_lock(&state->lock);
            state->board[row][col] = mark;
            pthread_mutex_unlock(&state->lock);
            moves++;
            turn = 1 - turn;

            print_board(state);
        }
        else
            printf(RED "\nJogada inválida. Tente novamente.\n" RESET);
    }
}

int main()
{
    // Inicializa o estado do jogo
    GameState state = {
        .board = {
            {'_', '_', '_'},
            {'_', '_', '_'},
            {'_', '_', '_'}},
        .result = 0};

    pthread_mutex_init(&state.lock, NULL);

    pthread_t threads[3];

    // Cria threads para verificar linhas, diagonais e empate
    pthread_create(&threads[0], NULL, check_lines, &state);
    pthread_create(&threads[1], NULL, check_diagonals, &state);
    pthread_create(&threads[2], NULL, check_draw, &state);

    // Inicia o jogo
    play_game(&state);

    // Aguarda todas as threads terminarem
    for (int i = 0; i < 3; i++)
        pthread_join(threads[i], NULL);

    // Determina o resultado final
    if (state.result == 1)
        printf("Jogador 1 venceu!\n");
    else if (state.result == 2)
        printf("Jogador 2 venceu!\n");
    else if (state.result == 3)
        printf("Velha!\n");

    pthread_mutex_destroy(&state.lock);
    return 0;
}
