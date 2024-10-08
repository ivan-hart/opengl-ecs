#include <game.h>

int main(int _argc, char * _argv[])
{
    Game & game = Game::Instance();
    game.Run();
    game.Close();
    return 0;
}