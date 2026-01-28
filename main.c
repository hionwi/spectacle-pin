#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <unistd.h>

void CopySurfaceToClipboard(SDL_Surface* surface) {
    // 1. 检查是否有 wl-copy
    if (system("which wl-copy > /dev/null 2>&1") != 0) {
        printf("Error: wl-copy not found. Please install 'wl-clipboard'.\n");
        return;
    }

    const char* temp_file = "/tmp/sdl_clipboard_temp.png";
    
    // 2. 保存 Surface 为 PNG
    if (IMG_SavePNG(surface, temp_file) != 0) {
        printf("Failed to save temp image: %s\n", IMG_GetError());
        return;
    }

    // 3. 构造 Wayland 复制命令
    // wl-copy 直接支持从文件读取，且会自动在后台维护剪贴板
    char command[256];
    snprintf(command, sizeof(command), "wl-copy -t image/png < %s", temp_file);
    
    if (system(command) == 0) {
        printf("Image copied to clipboard via Wayland!\n");
    }

    // 4. 删除临时文件
    // wl-copy 会在读取完输入流后立即返回，所以这里删除是安全的
    unlink(temp_file);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <image_path>\n", argv[0]);
        return 1;
    }

    if (SDL_Init(SDL_INIT_VIDEO) < 0) return 1;
    IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);

    // 【关键】设置缩放质量：0 = nearest, 1 = linear, 2 = best (anisitropic)
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");

    SDL_Surface* temp_surface = IMG_Load(argv[1]);
    if (!temp_surface) {
        printf("Could not load image: %s\n", IMG_GetError());
        return 1;
    }
    // --- 启动时复制 ---
    CopySurfaceToClipboard(temp_surface);

    int img_w = temp_surface->w;
    int img_h = temp_surface->h;
    
    // 创建无边框窗口
    SDL_Window* window = SDL_CreateWindow("k-review", 
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
        img_w, img_h, SDL_WINDOW_BORDERLESS);

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, temp_surface);
    SDL_FreeSurface(temp_surface);

    int quit = 0;
    SDL_Event e;
    float scale = 1.0f;
    int is_dragging = 0;
    int mouse_x, mouse_y;

    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) quit = 1;

            // 右键退出
            if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_RIGHT) {
                quit = 1;
            }

            // 鼠标滚轮缩放
            if (e.type == SDL_MOUSEWHEEL) {
                if (e.wheel.y > 0) scale *= 1.05f; // 缩小步长让缩放更丝滑
                else if (e.wheel.y < 0) scale /= 1.05f;
                
                // 动态调整窗口大小
                SDL_SetWindowSize(window, (int)(img_w * scale), (int)(img_h * scale));
            }

            // 窗口拖拽逻辑
            if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                is_dragging = 1;
                SDL_GetGlobalMouseState(&mouse_x, &mouse_y);
            }
            if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT) {
                is_dragging = 0;
            }
            if (e.type == SDL_MOUSEMOTION && is_dragging) {
                int curr_x, curr_y;
                SDL_GetGlobalMouseState(&curr_x, &curr_y);
                int win_x, win_y;
                SDL_GetWindowPosition(window, &win_x, &win_y);
                SDL_SetWindowPosition(window, win_x + (curr_x - mouse_x), win_y + (curr_y - mouse_y));
                mouse_x = curr_x;
                mouse_y = curr_y;
            }
        }

        SDL_RenderClear(renderer);
        // 将纹理完整渲染到当前窗口大小，SDL 会根据 Hint 自动进行平滑缩放
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