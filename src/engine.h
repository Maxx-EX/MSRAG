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
    const char *bg_color; // 十六进制颜色，如 "#1a1a2e"
    MSRAG_Choice choices[MSRAG_MAX_CHOICES];
    int         choice_count;
    int         next_scene; // 无选项时的默认下一场景，-1 表示结束
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
} MSRAG_Engine;

// 引擎生命周期
int  MSRAG_Init(MSRAG_Engine *eng, const char *title, int w, int h);
void MSRAG_Destroy(MSRAG_Engine *eng);

// 场景注册
void MSRAG_AddScene(MSRAG_Engine *eng, const MSRAG_Scene *scene);

// 主循环（阻塞，直到窗口关闭或剧情结束）
void MSRAG_Run(MSRAG_Engine *eng);

// 工具：十六进制颜色解析
SDL_Color MSRAG_ParseColor(const char *hex);

#endif
