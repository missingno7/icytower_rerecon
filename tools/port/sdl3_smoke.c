#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
int main(int argc, char **argv)
{
    (void)argc; (void)argv;
    if (!SDL_Init(0)) return 1;
    SDL_Log("SDL %d.%d.%d", SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_MICRO_VERSION);
    SDL_Quit();
    return 0;
}
