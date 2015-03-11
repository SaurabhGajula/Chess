#include "engine.h"

int initial_board[8][8] ={{ 4,  1,  0,  0,  0,  0,  -1, -4 },
                          { 3,  1,  0,  0,  0,  0,  -1, -3 },
                          { 2,  1,  0,  0,  0,  0,  -1, -2 },
                          { 5,  1,  0,  0,  0,  0,  -1, -5 },
                          { 6,  1,  0,  0,  0,  0,  -1, -6 },
                          { 2,  1,  0,  0,  0,  0,  -1, -2 },
                          { 3,  1,  0,  0,  0,  0,  -1, -3 },
                          { 4,  1,  0,  0,  0,  0,  -1, -4 } };

Engine::Engine(bool AI_mode)
{
    for(int i=0;i<8;i++)
        for(int j=0;j<8;j++)
            board_matrix[i][j] = initial_board[i][j];
    player1 = true;
    game_status = GAME_NORMAL;

    prev_ai_move.x0 = -1;
    prev_ai_move.y0 = -1;
    prev_ai_move.x1 = -1;
    prev_ai_move.y1 = -1;

    ai_mode = AI_mode;
}

int Engine::GetGameStatus()
{
    return game_status;
}

int Engine::GetCurrentPlayer()
{
    if(player1)
        return 1;
    else return 2;
}
int Engine::GetPiece(int x, int y)
{
    return board_matrix[x][y];
}

void Engine::ProcessInput(int x0, int y0, int x1, int y1)
{
    if((player1 && board_matrix[x0][y0]>0) || (!player1 && board_matrix[x0][y0]<0))
        if(IsValidMove(x0, y0, x1, y1))
        {
            MakeMove(x0, y0, x1, y1);
            player1 = !player1;

            if((player1 && IsCheck1()) || (!player1 && IsCheck2()))
                game_status = GAME_CHECK;
            else
                game_status = GAME_NORMAL;

            if((player1 && IsCheckmate1()) || (!player1 && IsCheckmate2()))
                game_status = GAME_CHECKMATE;
            else if((player1 && IsStalemate1()) || (!player1 && IsStalemate2()))
                game_status = GAME_STALEMATE;

            //Just for debugging
            if(player1==false && ai_mode)
            {
                game_status = GAME_THINKING;
                MakeAIMove();
            }
        }
}

int Engine::MakeMove(int x0, int y0, int x1, int y1)
{
    int temp = board_matrix[x1][y1];
    board_matrix[x1][y1] = board_matrix[x0][y0];
    board_matrix[x0][y0] = 0;

    return temp;
}

moves Engine::GetAIMove()
{
    return prev_ai_move;
}


