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

static const char *find_font(void) {
    static const char *candidates[] = {
        "./assets/NotoSansCJKsc-Regular.otf", "./assets/NotoSansSC-Regular.otf",
        "./assets/font.ttf", "/system/fonts/NotoSansCJK-Regular.ttc",
        "/system/fonts/NotoSansCJKsc-Regular.otf", NULL
    };
    for (int i = 0; candidates[i]; i++) {
        FILE *f = fopen(candidates[i], "rb");
        if (f) { fclose(f); return candidates[i]; }
    }
    return NULL;
}

// ================= 存档系统实现 =================

static void get_save_path(char *buf, size_t size) {
    // SDL_GetPrefPath 会自动创建基于组织名和游戏名的专属目录，并在 Android/Termux 上返回合法路径
    char *pref = SDL_GetPrefPath("Maxx-EX", "MSRAG");
    if (pref) {
        snprintf(buf, size, "%ssave.dat", pref);
        SDL_free(pref);
    } else {
        snprintf(buf, size, "save.dat"); // 兜底
    }
}

void MSRAG_SaveGame(MSRAG_Engine *eng) {
    char path[512];
    get_save_path(path, sizeof(path));
    FILE *f = fopen(path, "w");
    if (f) {
        // 目前只存当前场景 ID 和选中的选项
        fprintf(f, "%d %d\n", eng->current_scene, eng->selected_choice);
        fclose(f);
        SDL_Log("已保存至: %s", path);
        
        // 触发 UI 提示
        snprintf(eng->toast_msg, sizeof(eng->toast_msg), "✓ 进度已保存");
        eng->toast_timer = SDL_GetTicks() + 2000; // 显示 2 秒
    }
}

int MSRAG_LoadGame(MSRAG_Engine *eng) {
    char path[512];
    get_save_path(path, sizeof(path));
    FILE *f = fopen(path, "r");
    if (f) {
        int scene, choice;
        if (fscanf(f, "%d %d", &scene, &choice) == 2) {
            if (scene >= 0 && scene < eng->scene_count) {
                eng->current_scene = scene;
                eng->selected_choice = choice;
                fclose(f);
                SDL_Log("已读取: %s", path);
                
                snprintf(eng->toast_msg, sizeof(eng->toast_msg), "✓ 进度已读取");
                eng->toast_timer = SDL_GetTicks() + 2000;
                return 1;
            }
        }
        fclose(f);
    }
    snprintf(eng->toast_msg, sizeof(eng->toast_msg), "✗ 无存档记录");
    eng->toast_timer = SDL_GetTicks() + 2000;
    return 0;
}

// ===============================================

int MSRAG_Init(MSRAG_Engine *eng, const char *title, int w, int h) {
    memset(eng, 0, sizeof(*eng));
    eng->width = w; eng->height = h; eng->running = 1;

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
    TTF_Init();

    eng->window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, SDL_WINDOW_SHOWN);
    eng->renderer = SDL_CreateRenderer(eng->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!eng->renderer) eng->renderer = SDL_CreateRenderer(eng->window, -1, SDL_RENDERER_SOFTWARE);

    const char *fp = find_font();
    if (fp) eng->font = TTF_OpenFont(fp, 28);
    if (!eng->font) return -1;

    // 启动时尝试自动读取存档
    MSRAG_LoadGame(eng);
    eng->toast_timer = 0; // 清空启动时的提示
    return 0;
}

void MSRAG_Destroy(MSRAG_Engine *eng) {
    if (eng->font)     TTF_CloseFont(eng->font);
    if (eng->renderer) SDL_DestroyRenderer(eng->renderer);
    if (eng->window)   SDL_DestroyWindow(eng->window);
    TTF_Quit(); SDL_Quit();
}

void MSRAG_AddScene(MSRAG_Engine *eng, const MSRAG_Scene *scene) {
    if (eng->scene_count >= MSRAG_MAX_SCENES) return;
    eng->scenes[eng->scene_count++] = *scene;
}

static void render_text_wrapped(SDL_Renderer *ren, TTF_Font *font, const char *text, SDL_Color color, int x, int y, int wrap_width) {
    if (!text || !*text) return;
    SDL_Surface *surf = TTF_RenderUTF8_Blended_Wrapped(font, text, color, wrap_width);
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
        render_text_wrapped(eng->renderer, eng->font, sc->speaker, (SDL_Color){220, 180, 255, 255}, box.x + 20, box.y + 16, box.w - 40);
    }
    render_text_wrapped(eng->renderer, eng->font, sc->text, (SDL_Color){230, 230, 240, 255}, box.x + 20, box.y + 60, box.w - 40);

    for (int i = 0; i < sc->choice_count; i++) {
        int y = box.y + 130 + i * 36;
        SDL_Color col = (i == eng->selected_choice) ? (SDL_Color){255, 220, 120, 255} : (SDL_Color){200, 200, 220, 255};
        render_text_wrapped(eng->renderer, eng->font, sc->choices[i].label, col, box.x + 40, y, box.w - 80);
        if (i == eng->selected_choice) {
            render_text_wrapped(eng->renderer, eng->font, "▶", col, box.x + 16, y, 24);
        }
    }

    // --- 渲染右上角的提示 UI ---
    if (eng->toast_timer > SDL_GetTicks()) {
        SDL_SetRenderDrawColor(eng->renderer, 0, 0, 0, 200);
        SDL_Rect tbox = {eng->width - 250, 20, 230, 50};
        SDL_RenderFillRect(eng->renderer, &tbox);
        render_text_wrapped(eng->renderer, eng->font, eng->toast_msg, (SDL_Color){100, 255, 100, 255}, tbox.x + 20, tbox.y + 10, tbox.w - 40);
    }

    SDL_RenderPresent(eng->renderer);
}

static void advance(MSRAG_Engine *eng) {
    MSRAG_Scene *sc = &eng->scenes[eng->current_scene];
    int next = (sc->choice_count > 0) ? sc->choices[eng->selected_choice].next_scene : sc->next_scene;
    
    if (next < 0 || next >= eng->scene_count) {
        eng->running = 0; return;
    }
    eng->current_scene = next;
    eng->selected_choice = 0;

    // 场景切换时，自动保存进度
    MSRAG_SaveGame(eng); 
}

void MSRAG_Run(MSRAG_Engine *eng) {
    while (eng->running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) eng->running = 0;
            else if (e.type == SDL_KEYDOWN) {
                MSRAG_Scene *sc = &eng->scenes[eng->current_scene];
                switch (e.key.keysym.sym) {
                    case SDLK_UP:
                        if (sc->choice_count > 0) {
                            eng->selected_choice--;
                            if (eng->selected_choice < 0) eng->selected_choice = sc->choice_count - 1;
                        }
                        break;
                    case SDLK_DOWN:
                        if (sc->choice_count > 0) {
                            eng->selected_choice++;
                            if (eng->selected_choice >= sc->choice_count) eng->selected_choice = 0;
                        }
                        break;
                    case SDLK_RETURN:
                    case SDLK_SPACE:
                        advance(eng);
                        break;
                    case SDLK_s: // 手动保存
                        MSRAG_SaveGame(eng);
                        break;
                    case SDLK_l: // 手动读取
                        MSRAG_LoadGame(eng);
                        break;
                    case SDLK_ESCAPE:
                        eng->running = 0;
                        break;
                }
            } else if (e.type == SDL_FINGERDOWN) {
                advance(eng);
            }
        }
        render_scene(eng);
        SDL_Delay(16);
    }
}
