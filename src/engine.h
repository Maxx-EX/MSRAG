#ifndef MSRAG_ENGINE_H
#define MSRAG_ENGINE_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#define MSRAG_MAX_CHOICES 4
#define MSRAG_MAX_SCENES  64
#define MSRAG_TEXT_BUF    1024

typedef struct {
    const char *label;
    int         next_scene;
} MSRAG_Choice;

typedef struct {
    int         id;
    const char *speaker;
    const char *text;
    const char *bg_color;
    MSRAG_Choice choices[MSRAG_MAX_CHOICES];
    int         choice_count;
    int         next_scene;
} MSRAG_Scene;

typedef struct {
    SDL_Window   *window;
    SDL_Renderer *renderer;
    TTF_Font     *font;
    int           width;
    int           height;
    int           running;
    int           current_scene;
    MSRAG_Scene   scenes[MSRAG_MAX_SCENES];
    int           scene_count;
    int           selected_choice;
    
    // --- 新增：存档系统 UI 提示 ---
    char          toast_msg[64];
    Uint32        toast_timer;
} MSRAG_Engine;

int  MSRAG_Init(MSRAG_Engine *eng, const char *title, int w, int h);
void MSRAG_Destroy(MSRAG_Engine *eng);
void MSRAG_AddScene(MSRAG_Engine *eng, const MSRAG_Scene *scene);
void MSRAG_Run(MSRAG_Engine *eng);
SDL_Color MSRAG_ParseColor(const char *hex);

// --- 新增：存档相关 API ---
void MSRAG_SaveGame(MSRAG_Engine *eng);
int  MSRAG_LoadGame(MSRAG_Engine *eng);

#endif
