# MSRAG

轻量 2D 交互式对话引擎，C + SDL2，面向 Termux / Android 移动端。

## 产物

每次推送 `main` 分支后，Actions 会生成两个 artifact：

| Artifact | 内容 |
|---|---|
| `msrag-x86_64` | x86_64 静态 bin + 动态 so 包（含 SDL2 .so） |
| `msrag-aarch64` | **Termux 原生** aarch64 静态 bin + 动态 so 包（含交叉编译的 SDL2 .so） |

每个 artifact 内有两个压缩包：

- `msrag-<arch>.tar.gz`：动态库 + 静态 bin + SDL2 运行时 `.so`
- `msrag-<arch>-bin-static.tar.gz`：仅静态 `msrag` 可执行文件，SDL2 已编入，解压即可运行

## Termux 使用

```bash
# 下载 msrag-aarch64-bin-static.tar.gz 后
tar xf msrag-aarch64-bin-static.tar.gz
./msrag
```

中文字体会自动探测 /system/fonts/NotoSansCJK-Regular.ttc、Termux 字体目录以及 ./assets/。如果设备缺少 CJK 字体，可执行：

```bash
pkg install ttf-wqy-microhei
```

或把任意中文字体命名为 font.ttf 放入 assets/。

联系作者

邮箱：programmingscience@foxmail.com
