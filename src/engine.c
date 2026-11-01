#include "engine.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

SDL_Color MSRAG_ParseColor(const char *hex) {
    SDL_Color c = {0, 0, 0, 255};
    if (!hex || hex[0] != '#') return c;
    unsigned int r, g, b;
    if (sscanf(hex + 1, "%02x%02x%02x", &r, &g, &b) == 3) {
        c.r = (Uint8)r; c.g = (Uint8)g; c.b = (Uint8)b;
    }
    return c;
}

/* 自动探测中文字体：优先随包分发的 assets，再回退系统/Termux 字体 */
static const char *find_font(void) {
    static const char *candidates[] = {
        "./assets/NotoSansSC-Regular.otf",
        "./assets/NotoSansCJK-Regular.ttc",
        "./assets/font.ttf",
        "/system/fonts/NotoSansCJK-Regular.ttc",
        "/system/fonts/NotoSansSC-Regular.otf",
        "/system/fonts/NotoSansCJKsc-Regular.otf",
        "/data/data/com.termux/files/usr/share/fonts/TTF/NotoSansCJK-Regular.ttc",
        "/data/data/com.termux/files/usr/share/fonts/noto/NotoSansCJK-Regular.ttc",
        "/data/data/com.termux/files/usr/share/fonts/truetype/wqy/wqy-microhei.ttc",
        NULL
    };
    for (int i = 0; candidates[i]; i++) {
        FILE *f = fopen(candidates[i], "rb");
        if (f) { fclose(f); return candidates[i]; }
    }
    return NULL;
}

int MSRAG_Init(MSRAG_Engine *eng, const char *title, int w, int h) {
    memset(eng, 0, sizeof(*eng));
    eng->width = w;
    eng->height = h;
    eng->running = 1;
    eng->current_scene = 0;
    eng->selected_choice = 0;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return -1;
    }
    if (TTF_Init() < 0) {
        SDL_Log("TTF_Init failed: %s", TTF_GetError());
        return -1;
    }

    eng->window = SDL_CreateWindow(title,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        w, h, SDL_WINDOW_SHOWN);
    if (!eng->window) {
        SDL_Log("CreateWindow failed: %s", SDL_GetError());
        return -1;
    }

    eng->renderer = SDL_CreateRenderer(eng->window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!eng->renderer) {
        eng->renderer = SDL_CreateRenderer(eng->window, -1, SDL_RENDERER_SOFTWARE);
    }
    if (!eng->renderer) {
        SDL_Log("CreateRenderer failed: %s", SDL_GetError());
        return -1;
    }

    const char *fp = find_font();
    if (!fp) {
        SDL_Log("未找到中文字体。请把 NotoSansSC-Regular.otf 放到 ./assets/，"
                "或执行 pkg install ttf-wqy-microhei");
        return -1;
    }
    SDL_Log("使用字体: %s", fp);
    eng->font = TTF_OpenFont(fp, 28);
    if (!eng->font) {
        SDL_Log("TTF_OpenFont(%s) failed: %s", fp, TTF_GetError());
        return -1;
    }

    return 0;
}

void MSRAG_Destroy(MSRAG_Engine *eng) {
    if (eng->font)     TTF_CloseFont(eng->font);
    if (eng->renderer) SDL_DestroyRenderer(eng->renderer);
    if (eng->window)   SDL_DestroyWindow(eng->window);
    TTF_Quit();
    SDL_Quit();
}

void MSRAG_AddScene(MSRAG_Engine *eng, const MSRAG_Scene *scene) {
    if (eng->scene_count >= MSRAG_MAX_SCENES) return;
    eng->scenes[eng->scene_count++] = *scene;
}

static void render_text_wrapped(SDL_Renderer *ren, TTF_Font *font,
                                const char *text, SDL_Color color,
                                int x, int y, int wrap_width) {
    if (!text || !*text) return;
    char buf[MSRAG_TEXT_BUF];
    strncpy(buf, text, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    SDL_Surface *surf = TTF_RenderUTF8_Blended_Wrapped(font, buf, color, wrap_width);
    if (surf) {
        SDL_Texture *tex = SDL_CreateTextureFromSurface(ren, surf);
        if (tex) {
            SDL_Rect dst = {x, y, surf->w, surf->h};
            SDL_RenderCopy(ren, tex, NULL, &dst);
            SDL_DestroyTexture(tex);
        }
        SDL_FreeSurface(surf);
    }
}

static void render_scene(MSRAG_Engine *eng) {
    MSRAG_Scene *sc = &eng->scenes[eng->current_scene];

    SDL_Color bg = MSRAG_ParseColor(sc->bg_color ? sc->bg_color : "#0d0d1a");
    SDL_SetRenderDrawColor(eng->renderer, bg.r, bg.g, bg.b, 255);
    SDL_RenderClear(eng->renderer);

    SDL_Rect box = {40, eng->height - 260, eng->width - 80, 220};
    SDL_SetRenderDrawColor(eng->renderer, 20, 20, 40, 230);
    SDL_RenderFillRect(eng->renderer, &box);
    SDL_SetRenderDrawColor(eng->renderer, 180, 140, 220, 255);
    SDL_RenderDrawRect(eng->renderer, &box);

    if (sc->speaker && *sc->speaker) {
        SDL_Color name_col = {220, 180, 255, 255};
        render_text_wrapped(eng->renderer, eng->font, sc->speaker, name_col,
                            box.x + 20, box.y + 16, box.w - 40);
    }

    SDL_Color text_col = {230, 230, 240, 255};
    render_text_wrapped(eng->renderer, eng->font, sc->text, text_col,
                        box.x + 20, box.y + 60, box.w - 40);

    for (int i = 0; i < sc->choice_count; i++) {
        int y = box.y + 130 + i * 36;
        SDL_Color col = (i == eng->selected_choice)
                            ? (SDL_Color){255, 220, 120, 255}
                            : (SDL_Color){200, 200, 220, 255};
        render_text_wrapped(eng->renderer, eng->font,
                            sc->choices[i].label, col,
                            box.x + 40, y, box.w - 80);
        if (i == eng->selected_choice) {
            render_text_wrapped(eng->renderer, eng->font, "▶", col,
                                box.x + 16, y, 24);
        }
    }

    SDL_RenderPresent(eng->renderer);
}

static void advance(MSRAG_Engine *eng) {
    MSRAG_Scene *sc = &eng->scenes[eng->current_scene];
    int next = -1;

    if (sc->choice_count > 0) {
        next = sc->choices[eng->selected_choice].next_scene;
    } else {
        next = sc->next_scene;
    }

    if (next < 0 || next >= eng->scene_count) {
        eng->running = 0;
        return;
    }
    eng->current_scene = next;
    eng->selected_choice = 0;
}

void MSRAG_Run(MSRAG_Engine *eng) {
    while (eng->running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                eng->running = 0;
            } else if (e.type == SDL_KEYDOWN) {
                MSRAG_Scene *sc = &eng->scenes[eng->current_scene];
                if (e.key.keysym.sym == SDLK_UP && sc->choice_count > 0) {
                    eng->selected_choice--;
                    if (eng->selected_choice < 0)
                        eng->selected_choice = sc->choice_count - 1;
                } else if (e.key.keysym.sym == SDLK_DOWN && sc->choice_count > 0) {
                    eng->selected_choice++;
                    if (eng->selected_choice >= sc->choice_count)
                        eng->selected_choice = 0;
                } else if (e.key.keysym.sym == SDLK_RETURN ||
                           e.key.keysym.sym == SDLK_SPACE) {
                    advance(eng);
                } else if (e.key.keysym.sym == SDLK_ESCAPE) {
                    eng->running = 0;
                }
            } else if (e.type == SDL_FINGERDOWN) {
                advance(eng);
            }
        }
        render_scene(eng);
        SDL_Delay(16);
    }
}
