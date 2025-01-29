#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#define SIZE 3
#define RED "\e[31m"
#define RESET "\e[0m"

// Structure to store the game board and result
typedef struct
{
    char board[SIZE][SIZE];
    int result; // 0: no winner, 1: player 1 wins, 2: player 2 wins, 3: draw
    pthread_mutex_t lock;
} GameState;

// Function to check rows and columns
void *check_lines(void *arg)
{
    GameState *state = (GameState *)arg;

    while (1)
    {
        pthread_mutex_lock(&state->lock);

        // Check rows
        for (int i = 0; i < SIZE; i++)
        {
            if (state->board[i][0] != '_' && state->board[i][0] == state->board[i][1] && state->board[i][1] == state->board[i][2])
            {
                if (state->board[i][0] == 'X')
                    state->result = 1;
                else if (state->board[i][0] == 'O')
                    state->result = 2;
                pthread_mutex_unlock(&state->lock);
                pthread_exit(NULL);
            }
        }

        // Check columns
        for (int i = 0; i < SIZE; i++)
        {
            if (state->board[0][i] != '_' && state->board[0][i] == state->board[1][i] && state->board[1][i] == state->board[2][i])
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

// Function to check diagonals
void *check_diagonals(void *arg)
{
    GameState *state = (GameState *)arg;

    while (1)
    {
        pthread_mutex_lock(&state->lock);

        // Check first diagonal (top-left to bottom-right)
        if (state->board[0][0] != '_' && state->board[0][0] == state->board[1][1] && state->board[1][1] == state->board[2][2])
        {
            if (state->board[0][0] == 'X')
                state->result = 1;
            else if (state->board[0][0] == 'O')
                state->result = 2;
            pthread_mutex_unlock(&state->lock);
            pthread_exit(NULL);
        }

        // Check second diagonal (top-right to bottom-left)
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

// Function to check if the game is a draw
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
            state->result = 3; // Draw
            pthread_mutex_unlock(&state->lock);
            pthread_exit(NULL);
        }

        pthread_mutex_unlock(&state->lock);

        if (state->result != 0)
            break;
    }

    pthread_exit(NULL);
}

// Function to print the game board
void print_board(GameState *state)
{
    printf("\nBoard:\n ");
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

// Function to play the game
void play_game(GameState *state)
{
    int turn = 0; // 0 for player 1 (X), 1 for player 2 (O)
    int moves = 0;

    while (state->result == 0 && moves < SIZE * SIZE)
    {
        int row, col;
        char mark = (turn == 0) ? 'X' : 'O';

        print_board(state);

        printf("\nPlayer %d (%c), enter your move (row and column): ", turn + 1, mark);
        scanf("%d %d", &row, &col);

        if (row >= 0 && row < SIZE && col >= 0 && col < SIZE && state->board[row][col] == '_')
        {
            state->board[row][col] = mark;
            moves++;
            turn = 1 - turn;

            print_board(state);
        }
        else
            printf(RED "\nInvalid move. Try again.\n" RESET);
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

    // Create threads to check lines, diagonals, and draw
    pthread_create(&threads[0], NULL, check_lines, &state);
    pthread_create(&threads[1], NULL, check_diagonals, &state);
    pthread_create(&threads[2], NULL, check_draw, &state);

    // Play the game
    play_game(&state);

    // Wait for all threads to finish
    for (int i = 0; i < 3; i++)
        pthread_join(threads[i], NULL);

    // Determine the final result
    if (state.result == 1)
        printf("Player 1 wins!\n");
    else if (state.result == 2)
        printf("Player 2 wins!\n");
    else if (state.result == 3)
        printf("It's a draw!\n");

    pthread_mutex_destroy(&state.lock);
    return 0;
}