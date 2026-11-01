#include "engine.h"
#include <stdio.h>

int main(int argc, char *argv[]) {
    (void)argc; (void)argv;

    MSRAG_Engine eng;
    if (MSRAG_Init(&eng, "MSRAG — 千恋万花风格演示", 960, 640) < 0) {
        fprintf(stderr, "引擎初始化失败。\n");
        return 1;
    }

    // ===== 场景 0：开场 =====
    MSRAG_Scene s0 = {
        .id = 0,
        .speaker = "旁白",
        .text = "建实町的夏天，蝉鸣比往年更响。\n你站在神社的石阶前，手里攥着一封没有署名的信。",
        .bg_color = "#0b1020",
        .choice_count = 1,
        .choices = {
            { "继续", 1 }
        }
    };

    // ===== 场景 1：邂逅 =====
    MSRAG_Scene s1 = {
        .id = 1,
        .speaker = "？？？",
        .text = "“你也是来参加祭典的吗？”\n少女回过头，发间的白色发带被风吹起。",
        .bg_color = "#1a1025",
        .choice_count = 2,
        .choices = {
            { "“算是吧。”", 2 },
            { "“我在等人。”", 3 }
        }
    };

    // ===== 场景 2：分支 A =====
    MSRAG_Scene s2 = {
        .id = 2,
        .speaker = "芳乃",
        .text = "“那正好，我也一个人。”\n她笑了笑，指向远处的灯笼。\n“要不要一起走走？”",
        .bg_color = "#1f1420",
        .choice_count = 1,
        .choices = {
            { "接受邀请", 4 }
        }
    };

    // ===== 场景 3：分支 B =====
    MSRAG_Scene s3 = {
        .id = 3,
        .speaker = "芳乃",
        .text = "“等人吗……那我不打扰了。”\n她微微鞠躬，转身走向人群。\n你忽然有些后悔。",
        .bg_color = "#141a1f",
        .choice_count = 1,
        .choices = {
            { "追上去", 4 }
        }
    };

    // ===== 场景 4：结局 =====
    MSRAG_Scene s4 = {
        .id = 4,
        .speaker = "旁白",
        .text = "祭典的灯火在夜色里摇晃。\n无论选择哪条路，这个夏天都已经开始了。\n\n—— 演示结束 ——",
        .bg_color = "#0a0a12",
        .choice_count = 0,
        .next_scene = -1
    };

    MSRAG_AddScene(&eng, &s0);
    MSRAG_AddScene(&eng, &s1);
    MSRAG_AddScene(&eng, &s2);
    MSRAG_AddScene(&eng, &s3);
    MSRAG_AddScene(&eng, &s4);

    MSRAG_Run(&eng);
    MSRAG_Destroy(&eng);
    return 0;
}

// 供动态库调用：运行演示游戏
__attribute__((visibility("default")))
int msrag_run_demo(void) {
    return main(0, NULL);
}
