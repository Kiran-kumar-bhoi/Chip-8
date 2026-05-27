#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>

#define WIDTH 64
#define HEIGHT 32
#define SCALE 10

int main() {

    SDL_Init(SDL_INIT_VIDEO);

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

    SDL_Window* window = SDL_CreateWindow(
        "Moving Pixel",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WIDTH * SCALE,
        HEIGHT * SCALE,
        SDL_WINDOW_SHOWN
    );

    SDL_Renderer* renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED
    );

    SDL_RenderSetLogicalSize(renderer, WIDTH, HEIGHT);

    SDL_Texture* texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        WIDTH,
        HEIGHT
    );

    uint32_t pixels[WIDTH * HEIGHT];

    // Player position
    int x = WIDTH / 2;
    int y = HEIGHT / 2;

    bool quit = false;
    SDL_Event e;

    while (!quit) {

        // Handle events
        while (SDL_PollEvent(&e)) {

            if (e.type == SDL_QUIT) {
                quit = true;
            }

            if (e.type == SDL_KEYDOWN) {

                switch (e.key.keysym.sym) {

                    case SDLK_w:
                        if (y > 0)
                            y--;
                        break;

                    case SDLK_s:
                        if (y < HEIGHT - 1)
                            y++;
                        break;

                    case SDLK_a:
                        if (x > 0)
                            x--;
                        break;

                    case SDLK_d:
                        if (x < WIDTH - 1)
                            x++;
                        break;
                }
            }
        }

        // Clear screen
        for (int i = 0; i < WIDTH * HEIGHT; i++) {
            pixels[i] = 0xFF000000;
        }

        // Draw white pixel
        pixels[x + y * WIDTH] = 0xFFFFFFFF;

        // Update texture
        SDL_UpdateTexture(
            texture,
            NULL,
            pixels,
            WIDTH * sizeof(uint32_t)
        );

        // Render
        SDL_RenderClear(renderer);

        SDL_RenderCopy(renderer, texture, NULL, NULL);

        SDL_RenderPresent(renderer);

        SDL_Delay(16);
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}
