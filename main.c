#include <stdio.h>
#include <time.h>
#include <SDL3/SDL.h>

#define BOARD_SIZE_X 16
#define BOARD_SIZE_Y 14
#define BASE_SPEED_DELAY 200
#define BASE_LENGTH 3

typedef enum SnakeTile
{
    TILE_EMPTY = 0,
    TILE_FRUIT = -1,
    // Positive nonzero values are snake segments.
} SnakeTile;

typedef enum Direction4
{
    UP = 1,
    RIGHT = 2,
    DOWN = 3,
    LEFT = 4,
} Direction4;

typedef struct Vector2
{
    int x;
    int y;
} Vector2;

int length;
Direction4 heading_direction;
Vector2 head_pos;
SnakeTile board[BOARD_SIZE_Y][BOARD_SIZE_X];

SDL_Window *win;
SDL_Renderer *renderer;
SDL_Event e;

void wrap_vector2(Vector2 *pos)
{

    if (pos->x < 0)
    {
        pos->x = BOARD_SIZE_X - 1;
    }
    pos->x = pos->x % BOARD_SIZE_X;
    if (pos->y < 0)
    {
        pos->y = BOARD_SIZE_Y - 1;
    }
    pos->y = pos->y % BOARD_SIZE_Y;
}

void new_fruit()
{
    short fruit_y, fruit_x;
    do
    {
        fruit_x = SDL_rand(BOARD_SIZE_X);
        fruit_y = SDL_rand(BOARD_SIZE_Y);
    } while (board[fruit_y][fruit_x] != 0);
    board[fruit_y][fruit_x] = TILE_FRUIT;
}

void draw()
{
    // Clear the window
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);

    // Draw the board
    for (int y = 0; y < BOARD_SIZE_Y; y++)
    {
        for (int x = 0; x < BOARD_SIZE_X; x++)
        {
            SnakeTile tile = board[y][x];
            switch (tile)
            {
            case TILE_EMPTY:
                SDL_SetRenderDrawColor(renderer, 0, 40, 20, SDL_ALPHA_OPAQUE);
                break;
            case TILE_FRUIT:
                SDL_SetRenderDrawColor(renderer, 200, 0, 20, SDL_ALPHA_OPAQUE);
                break;
            default: // Snake segments
                // Do a nice gradient
                int g = 255 - (length - tile) * 10;
                if (g < 100)
                {
                    g = 100;
                }
                SDL_SetRenderDrawColor(renderer, 0, g, 20, SDL_ALPHA_OPAQUE);
                break;
            }
            SDL_RenderPoint(renderer, x, y);
        }
    }

    // Present the rendering
    SDL_RenderPresent(renderer);
}

int game()
{
    // Clear out the board from the previous game
    for (short y = 0; y < BOARD_SIZE_Y; y++)
    {
        for (short x = 0; x < BOARD_SIZE_X; x++)
        {
            board[y][x] = 0;
        }
    }

    // Initialise stuff
    head_pos.x = BOARD_SIZE_X / 2;
    head_pos.y = BOARD_SIZE_Y / 2;
    length = BASE_LENGTH;
    heading_direction = UP;
    new_fruit();

    // Game loop
    while (true)
    {
        Uint64 start = SDL_GetTicks();

        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_EVENT_QUIT)
            {
                // Quit game (return 0)
                return 0;
            }
        }

        // Process game input
        const bool *keyboard_state = SDL_GetKeyboardState(NULL);
        if (keyboard_state[SDL_SCANCODE_UP] && heading_direction != DOWN)
        {
            heading_direction = UP;
        }
        else if (keyboard_state[SDL_SCANCODE_DOWN] && heading_direction != UP)
        {
            heading_direction = DOWN;
        }
        else if (keyboard_state[SDL_SCANCODE_LEFT] && heading_direction != RIGHT)
        {
            heading_direction = LEFT;
        }
        else if (keyboard_state[SDL_SCANCODE_RIGHT] && heading_direction != LEFT)
        {
            heading_direction = RIGHT;
        }

        // Move
        switch (heading_direction)
        {
        case UP:
            head_pos.y--;
            break;
        case RIGHT:
            head_pos.x++;
            break;
        case DOWN:
            head_pos.y++;
            break;
        case LEFT:
            head_pos.x--;
            break;
        }
        wrap_vector2(&head_pos);

        // Process hitting self
        if (board[head_pos.y][head_pos.x] > 0)
        {
            // Wait a second before beginning a new game (return 1)
            SDL_Delay(1000);
            return 1;
        }

        // Process getting fruit
        if (board[head_pos.y][head_pos.x] == TILE_FRUIT)
        {
            board[head_pos.y][head_pos.x] = length++;
            new_fruit();
        }
        else
        {
            board[head_pos.y][head_pos.x] = length + 1; // +1 so it doesn't immediately get decayed to zero
            // Decay snake length.
            // Skip decay if we got a fruit, increasing our overall size
            for (int y = 0; y < BOARD_SIZE_Y; y++)
            {
                for (int x = 0; x < BOARD_SIZE_X; x++)
                {
                    SnakeTile tile = board[y][x];
                    if (tile > 0)
                    {
                        board[y][x] = --tile;
                    }
                }
            }
        }

        draw();

        // Do game speed
        Uint64 end = SDL_GetTicks();
        short frame_legnth = BASE_SPEED_DELAY - (length * 3 / 2);
        float elapsedMS = (end - start) / (float)SDL_GetPerformanceFrequency() * 1000.0f;
        SDL_Delay(frame_legnth - (16.666f - elapsedMS));
    }
}

int main(int argc, char *argv[])
{
    SDL_srand(time(NULL));

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
    {
        puts("SDL failed to initialise!");
        puts(SDL_GetError());
        return 1;
    };

    win = SDL_CreateWindow("SnakeC", 512, 448, SDL_WINDOW_RESIZABLE);
    if (win == NULL)
    {
        puts("Error creating window:");
        puts(SDL_GetError());
        return 1;
    }
    renderer = SDL_CreateRenderer(win, NULL);
    if (renderer == NULL)
    {
        puts("Error creating renderer:");
        puts(SDL_GetError());
        return 1;
    }
    if (!SDL_SetRenderLogicalPresentation(renderer, BOARD_SIZE_X, BOARD_SIZE_Y, SDL_LOGICAL_PRESENTATION_LETTERBOX))
    {
        puts("Error configuring renderer:");
        puts(SDL_GetError());
        return 1;
    }

    // Big fan of this
    while (game() != 0);

    return 0;
}