#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>

SDL_HitTestResult HitTestCallback(SDL_Window* win, const SDL_Point* area, void* data) {
    return SDL_HITTEST_DRAGGABLE;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <image_path>\n", argv[0]);
        return 1;
    }

    if (SDL_Init(SDL_INIT_VIDEO) < 0) return 1;
    IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");

    SDL_Surface* temp_surface = IMG_Load(argv[1]);
    if (!temp_surface) {
        printf("Could not load image: %s\n", IMG_GetError());
        return 1;
    }

    int img_w = temp_surface->w;
    int img_h = temp_surface->h;
    
    SDL_Window* window = SDL_CreateWindow("k-review", 
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
        img_w, img_h, SDL_WINDOW_BORDERLESS | SDL_WINDOW_ALWAYS_ON_TOP);

    SDL_SetWindowHitTest(window, HitTestCallback, NULL);

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, temp_surface);
    SDL_FreeSurface(temp_surface);

    int quit = 0;
    SDL_Event e;
    float scale = 1.0f;

    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) quit = 1;

            if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_ESCAPE: 
                        quit = 1;
                        break;
                }
            }

            if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_RIGHT) {
                quit = 1;
            }

            if (e.type == SDL_MOUSEWHEEL) {
                if (e.wheel.y > 0) scale *= 1.05f;
                else if (e.wheel.y < 0) scale /= 1.05f;
                
                SDL_SetWindowSize(window, (int)(img_w * scale), (int)(img_h * scale));
            }
        }

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}
