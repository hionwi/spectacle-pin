#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>

// --- 新增：命中测试回调函数 ---
SDL_HitTestResult HitTestCallback(SDL_Window* win, const SDL_Point* area, void* data) {
    // 返回 DRAGGABLE 告诉系统，点击窗口任何位置都可以拖动
    // 这在无边框窗口中完美替代了标题栏的作用
    return SDL_HITTEST_DRAGGABLE;
}

void CopySurfaceToClipboard(SDL_Surface* surface) {
    if (system("which wl-copy > /dev/null 2>&1") != 0) {
        printf("Error: wl-copy not found. Please install 'wl-clipboard'.\n");
        return;
    }

    const char* temp_file = "/tmp/sdl_clipboard_temp.png";
    if (IMG_SavePNG(surface, temp_file) != 0) {
        printf("Failed to save temp image: %s\n", IMG_GetError());
        return;
    }

    char command[256];
    snprintf(command, sizeof(command), "wl-copy -t image/png < %s", temp_file);
    if (system(command) == 0) {
        printf("Image copied to clipboard via Wayland!\n");
    }
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

    CopySurfaceToClipboard(temp_surface);

    int img_w = temp_surface->w;
    int img_h = temp_surface->h;
    
    SDL_Window* window = SDL_CreateWindow("k-review", 
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
        img_w, img_h, SDL_WINDOW_BORDERLESS | SDL_WINDOW_ALWAYS_ON_TOP);

    // 【关键】为 Wayland 设置命中测试，实现丝滑拖拽
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

            // --- 新增：按键监听 ---
            if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_ESCAPE: // 按下 Esc 退出
                        quit = 1;
                        break;
                    // 你可以在这里继续添加其他快捷键，比如按 'r' 重置缩放
                }
            }

            if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_RIGHT) {
                quit = 1;
            }

            if (e.type == SDL_MOUSEWHEEL) {
                if (e.wheel.y > 0) scale *= 1.05f;
                else if (e.wheel.y < 0) scale /= 1.05f;
                
                // 缩放时调整窗口大小
                SDL_SetWindowSize(window, (int)(img_w * scale), (int)(img_h * scale));
            }
            
            // 注意：这里不再需要手动写 MOUSEMOTION 的拖拽逻辑了
            // 系统会自动帮你处理
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
