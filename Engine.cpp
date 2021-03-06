#include "engine.h"
#include "engine_ai.h"

#include <fstream>


Engine::Engine(bool AI_mode):Board()
{

    player1 = true;
    game_status = GAME_NORMAL;

    prev_ai_move.x0 = -1;
    prev_ai_move.y0 = -1;
    prev_ai_move.x1 = -1;
    prev_ai_move.y1 = -1;

    ai_mode = AI_mode;
}

Engine::Engine(bool AI_mode, string fname)
{
    try
    {
        int x0, y0, x1, y1;
        Engine temp(false);
        vector<moves> redo_moves;
        redo_moves.clear();
        ifstream dbg(fname);
        while(true)
        {
            dbg>>x0>>y0>>x1>>y1;
            if(dbg.eof())
                break;
            if(x0>=0 && x0<8 && y0>=0 && y0<8 && x1>=0 && x1<8 && y1>=0 && y1<8)
                redo_moves.push_back(moves(x0, y0,x1, y1));
        }

        int length = redo_moves.size();
        int i=0;
        for(vector<moves>::iterator it = redo_moves.begin();i<length-length%2;++it, ++i)
        {
            temp.ProcessInput(it->x0, it->y0, it->x1, it->y1);
        }
        *this = temp;
        this->ai_mode = AI_mode;

        printf("%u\n", length);
        if(length%2==1)
        {
            moves last_move = redo_moves.back();
            this->ProcessInput(last_move.x0, last_move.y0, last_move.x1, last_move.y1);
        }

        dbg.close();
    }
    catch(...)
    {
        cout<<"Error reading from "<<fname<<endl;
    }
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
            if(ENGINE_DEBUG)
            {
                try
                {
                    ofstream dbg;
                    if(previous_moves.size()==0)
                        dbg.open("debug.dat", ios::trunc);
                    else
                        dbg.open("debug.dat", ios::app);
                    dbg<<x0<<"\t"<<y0<<"\t"<<x1<<"\t"<<y1<<"\n";
                    dbg.close();
                }
                catch(...)
                {
                    cout<<"Error writing to file"<<endl;
                }
            }
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
            if(player1==false && ai_mode && game_status!=GAME_CHECKMATE && game_status!=GAME_STALEMATE)
            {
                game_status = GAME_THINKING;
                MakeAIMove();
            }
        }
}


int Board::MakeMove(int x0, int y0, int x1, int y1)
{
    previous_moves.push_back(moves(x0,y0,x1,y1));
    int temp = board_matrix[x1][y1];
    board_matrix[x1][y1] = board_matrix[x0][y0];
    board_matrix[x0][y0] = 0;

    if(y1 == 7 && board_matrix[x1][y1] == 1)
    {
        //Handle pawn promotion
        board_matrix[x1][y1] = 5;
        temp -= 9;
    }
    else if(y1 == 0 && board_matrix[x1][y1] == -1)
    {
        //Handle pawn promotion
        board_matrix[x1][y1] = -5;
        temp += 9;
    }
    else if(board_matrix[x1][y1]==6 && y1==0 && y0==0 && abs(x1-x0)==2)
    {
        //Handle Castling for player 1
        if(x1==2)
        {
            //Far castle
            board_matrix[0][0] = 0;
            board_matrix[3][0] = 4;
            temp = 7;
        }
        else if(x1==6)
        {
            //Near castle
            board_matrix[7][0] = 0;
            board_matrix[5][0] = 4;
            temp = 7;
        }
    }
    else if(board_matrix[x1][y1]==-6 && y1==7 && y0==7 && abs(x1-x0)==2)
    {
        //Handle Castling for player 2
        if(x1==2)
        {
            //Far castle
            board_matrix[0][7] = 0;
            board_matrix[3][7] = -4;
            temp = -7;
        }
        else if(x1==6)
        {
            //Near castle
            board_matrix[7][7] = 0;
            board_matrix[5][7] = -4;
            temp = -7;
        }
    }
    else if(board_matrix[x1][y1]==1 && temp == 0 && abs(x1 - x0) == 1)
    {
        //En passant
        board_matrix[x1][4] = 0;
        temp = 8;
    }
    else if(board_matrix[x1][y1]==-1 && temp == 0 && abs(x1 - x0) == 1)
    {
        //En passant
        board_matrix[x1][3] = 0;
        temp = -8;
    }

    return temp;
}

void Board::UndoMove(int piece0, int piece1, int x0, int y0, int x1, int y1)
{
    previous_moves.pop_back(); //temporary
    if(piece1 >=9 || piece1 <= -9)
    {
        //Pawn promotion undo
        if(piece1>0)
            piece1-=9;
        else
            piece1+=9;
    }
    else if(piece1 == 7)
    {
        piece1 = 0;
        if(x1 == 2)
        {
            //Far castle
            board_matrix[0][0] = 4;
            board_matrix[3][0] = 0;
        }
        else if(x1 == 6)
        {
            //Far castle
            board_matrix[7][0] = 4;
            board_matrix[5][0] = 0;
        }
    }
    else if(piece1 == -7)
    {
        piece1 = 0;
        if(x1 == 2)
        {
            //Far castle
            board_matrix[0][7] = -4;
            board_matrix[3][7] = 0;
        }
        else if(x1 == 6)
        {
            //Far castle
            board_matrix[7][7] = -4;
            board_matrix[5][7] = 0;
        }
    }
    else if(piece1 == 8)
    {
        piece1 = 0;
        board_matrix[x1][4] = -1;
    }
    else if(piece1 == -8)
    {
        piece1 = 0;
        board_matrix[x1][3] = 1;
    }
    board_matrix[x0][y0] = piece0;
    board_matrix[x1][y1] = piece1;

}

moves Engine::GetAIMove()
{
    return prev_ai_move;
}

long unsigned int __stdcall Engine::AIThread(void *input)
{
    Engine *main_engine = (Engine*)input;

    EngineAI *test_engine = new EngineAI((*(Board*)main_engine));

    printf("Thinking. \n");

    moves ai;

    float init_time = (float)clock()/CLOCKS_PER_SEC;

    ai = test_engine->GetBestMove(AI_MEDIUM);
    test_engine->PrintInfo();



    main_engine->prev_ai_move = moves(ai.x0, ai.y0, ai.x1, ai.y1);
    main_engine->ProcessInput(ai.x0, ai.y0, ai.x1, ai.y1);

    return 0;
}

void Engine::MakeAIMove()
{
    //AIThread(this);

    CreateThread(NULL, 0, AIThread, (void*)this, 0, 0);
}

bool Engine::UndoGame()
{
    bool modified = false;
    int prev_length = previous_moves.size();
    if(prev_length>=1 && game_status!=GAME_CHECKMATE && game_status!=GAME_STALEMATE)
    {
        Engine temp(false);
        if(ai_mode==false)
        {
            previous_moves.pop_back();
            for(vector<moves>::iterator it = previous_moves.begin();it!=previous_moves.end();++it)
            {
                temp.MakeMove(it->x0, it->y0, it->x1, it->y1);
            }

            for(int i=0;i<8;i++)
                for(int j=0;j<8;j++)
                    board_matrix[i][j] = temp.board_matrix[i][j];
            modified = true;
        }
        else if(ai_mode==true && game_status!=GAME_THINKING)
        {
            previous_moves.pop_back();
            previous_moves.pop_back();
            for(vector<moves>::iterator it = previous_moves.begin();it!=previous_moves.end();++it)
            {
                temp.MakeMove(it->x0, it->y0, it->x1, it->y1);
            }

            for(int i=0;i<8;i++)
                for(int j=0;j<8;j++)
                    board_matrix[i][j] = temp.board_matrix[i][j];

            prev_ai_move = moves(-1,-1,-1,-1);
            modified = true;
        }

    }

    return modified;
}
