#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#define SIZE 3
#define RED "\e[31m"
#define RESET "\e[0m"
// Estrutura para armazenar o tabuleiro e o resultado
typedef struct
{
    char board[SIZE][SIZE];
    int result; // 0: ninguém venceu, 1: jogador 1 venceu, 2: jogador 2 venceu
    pthread_mutex_t lock;
} GameState;

void *check_rows(void *arg)
{
    GameState *state = (GameState *)arg;

    while (1)
    {
        for (int i = 0; i < SIZE; i++)
        {
            pthread_mutex_lock(&state->lock);
            if (state->board[i][0] != '_' && state->board[i][0] == state->board[i][1] && state->board[i][1] == state->board[i][2])
            {
                if (state->board[i][0] == 'X')
                    state->result = 1;
                else if (state->board[i][0] == 'O')
                    state->result = 2;
            }
            pthread_mutex_unlock(&state->lock);
            break;
        }
        if (state->result != 0)
            break;
    }
    pthread_exit(NULL);
}

void *check_columns(void *arg)
{
    GameState *state = (GameState *)arg;

    while (1)
    {
        for (int i = 0; i < SIZE; i++)
        {
            pthread_mutex_lock(&state->lock);
            if (state->board[0][i] != '_' && state->board[0][i] == state->board[1][i] && state->board[1][i] == state->board[2][i])
            {
                if (state->board[0][i] == 'X')
                    state->result = 1;
                else if (state->board[0][i] == 'O')
                    state->result = 2;
            }
            pthread_mutex_unlock(&state->lock);
        }
        if (state->result != 0)
            break;
    }
    pthread_exit(NULL);
}

void *check_diagonals(void *arg)
{
    GameState *state = (GameState *)arg;
    while (1)
    {
        pthread_mutex_lock(&state->lock);
        if (state->board[0][0] != '_' && state->board[0][0] == state->board[1][1] && state->board[1][1] == state->board[2][2])
        {
            if (state->board[0][0] == 'X')
                state->result = 1;
            else if (state->board[0][0] == 'O')
                state->result = 2;
        }

        if (state->board[0][2] != '_' && state->board[0][2] == state->board[1][1] && state->board[1][1] == state->board[2][0])
        {
            if (state->board[0][2] == 'X')
                state->result = 1;
            else if (state->board[0][2] == 'O')
                state->result = 2;
        }
        pthread_mutex_unlock(&state->lock);

        if (state->result != 0)
            break;
    }

    pthread_exit(NULL);
}

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

void play_game(GameState *state)
{
    int turn = 0; // 0 para jogador 1 (X), 1 para jogador 2 (O)
    int moves = 0;

    while (state->result == 0 && moves < SIZE * SIZE)
    {
        int row, col;
        char mark = (turn == 0) ? 'X' : 'O';

        print_board(state);

        printf("\nJogador %d (%c), insira sua jogada (linha e coluna): ", turn + 1, mark);
        scanf("%d %d", &row, &col);

        if (row >= 0 && row < SIZE && col >= 0 && col < SIZE && state->board[row][col] == '_')
        {
            state->board[row][col] = mark;
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
    GameState state = {
        .board = {
            {'_', '_', '_'},
            {'_', '_', '_'},
            {'_', '_', '_'}},
        .result = 0};

    pthread_mutex_init(&state.lock, NULL);

    pthread_t threads[3];

    // Cria threads para verificar linhas, colunas e diagonais
    pthread_create(&threads[0], NULL, check_rows, &state);
    pthread_create(&threads[1], NULL, check_columns, &state);
    pthread_create(&threads[2], NULL, check_diagonals, &state);
    // Jogar o jogo
    play_game(&state);

    // Espera todas as threads terminarem
    for (int i = 0; i < 3; i++)
        pthread_join(threads[i], NULL);

    // Determina o resultado final
    if (state.result == 1)
        printf("O jogador 1 venceu!\n");
    else if (state.result == 2)
        printf("O jogador 2 venceu!\n");
    else if (state.result == 3)
        printf("Deu velha!\n");

    pthread_mutex_destroy(&state.lock);
    return 0;
}
