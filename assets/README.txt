将中文字体文件放在此目录下，例如：
  NotoSansSC-Regular.otf  或  font.ttf

推荐从 Google Fonts 下载 Noto Sans SC：
  https://fonts.google.com/noto/specimen/Noto+Sans+SC

Termux 上如果没有 /system/fonts/ 下的中文字体，
引擎会依次尝试：
  1. /system/fonts/NotoSansCJK-Regular.ttc
  2. /system/fonts/NotoSansSC-Regular.otf
  3. ./assets/NotoSansSC-Regular.otf
  4. ./assets/font.ttf
