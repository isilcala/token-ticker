# IoT LLM Monitor — ESP-IDF + LVGL 交付指南

## 概述

本文档用于将网页设计原型交付给 ESP32 + LVGL 开发团队。设计已完全遵循 LVGL 的实际限制（字体、渲染、分辨率），100% 可在硬件上复现。

---

## 1. 屏幕规格

| 项目 | 值 |
|------|-----|
| 分辨率 | 400 × 300 像素（横屏） |
| 显示类型 | 单色反射式 LCD（无背光） |
| 颜色 | 纯黑白，无灰度渐变 |
| 刷新率 | 自由，建议 100–500ms 更新一次 |
| DPI | 取决于实际屏幕尺寸，设计不依赖 DPI |

---

## 2. 布局分层

```
┌─────────────────────────────────────┐  高 30px
│  状态栏 (StatusBar)                 │  
│  时间|日期/星期  [Provider]  T/H/电量 WiFi │
├─────────────────────────────────────┤
│                                     │  高 270px
│  主内容区 (MiniMaxPanel)            │
│  ├─ 5-Hour Quota (进度条)           │
│  ├─ Weekly Quota (进度条)           │
│  ├─ Video Gen (分段条)              │
│  └─ 底部状态栏 (同步时间)           │
│                                     │
└─────────────────────────────────────┘
```

---

## 3. 字体规格（LVGL 内置）

### Montserrat（推荐默认）

- **来源**：LVGL 官方字体，矢量格式
- **可用字号**：12, 14, 16, 18, 20, 24, 28, 32px（在 LVGL 中需预编译）
- **字重**：Regular (400), SemiBold (600), Bold (700)
- **用途**：所有主要文本、标签、数字显示

**建议配置**：
- 大标题（时间）：18px, Bold
- 小标题（标签）：11px, SemiBold
- 正文（数值）：10px, Regular
- 注记（说明文字）：9px, Regular

### unscii-16（点阵替选）

- **来源**：公开域点阵字体，精确 16×8px 字符
- **格式**：bitmap，色彩深度 1bit（黑白）
- **用途**：极端可读性要求、复古 8bit 美学（可选）

**获取**：http://viznut.fi/unscii/unscii-16.ttf

---

## 4. 颜色系统（纯单色）

```c
// LVGL 颜色定义（单色模式）
#define LCD_BG       0xD4CC8A  // 淡绿背景（反射式 LCD 仿真）
#define LCD_FG       0x1A1F0A  // 深黑前景
#define LCD_DIM      0x6E7A4E  // 中灰（辅助文字）
#define LCD_MID      0x8A9A6A  // 浅灰（次要文字）
#define LCD_BORDER   0x3A4020  // 边框色
```

**建议 LVGL 配置**：
```c
#if LV_COLOR_DEPTH == 1
  // 单色模式：只有 0 (黑) 和 1 (白)
  lv_color_t bg = lv_color_black();   // 背景，实际显示为淡绿
  lv_color_t fg = lv_color_white();   // 前景，实际显示为深黑
#else
  // 如果 LVGL 支持灰度（8bit），按上述 RGB 值配置
#endif
```

---

## 5. 组件尺寸与布局

### 5.1 状态栏 (StatusBar)

**总高度**：30px  
**内部布局**：flex row, space-between, 垂直居中

| 区域 | 内容 | 尺寸 |
|------|------|------|
| 左 | 时间（18px）+ 日期（10px）+ 星期（9px） | 约 60px |
| 中 | Provider 标签（带边框）| 约 60px |
| 右 | 温度/湿度 + 电池图标 + WiFi 图标 | 约 90px |

**关键元素**：
- 时间：等宽字体，18px Bold，行高 1.0
- 日期/星期：9–10px，略灰
- Provider 标签：10px，有 1px 边框，内边距 1–5px
- 温湿度：10px
- 电池图标：22×10px（4 格分段）+ 2px 正极，后跟 "78%" （9px）
- WiFi 图标：10×9px SVG（3 层弧线，根据信号强度显示 1–3 格）

### 5.2 主内容区 (MiniMaxPanel)

**总高度**：270px  
**布局**：flex column, space-between，垂直填满

#### 5-Hour Quota / Weekly Quota Bar

**结构**：
```
标签行（11px SemiBold）     右侧用量数值（11px Bold）
┌─────────────────────────────────────────────────┐
│ 5-HOUR QUOTA              14.0K / 50K   28%     │
├─────────────────────────────────────────────────┤
│  ▼                                     → 2h left│
│ ╔═════════════════╤════════════════════╗        │  BAR_HEIGHT=16px
│ ║████████████████│║║║║║║║║║║║║║║║║║║║║ ║        │  (filled + marks)
│ ╚═════════════════╤════════════════════╝        │
│ Time 60%                                 ~2h... │
└─────────────────────────────────────────────────┘
```

**Quota Bar 细节**：
- 高度：16px，1px 黑色边框
- 已用部分：实心填充
- 进度刻度线：3 条（25%, 50%, 75%），仅覆盖已用区域
- 游标：SVG 三角形（尖端向下）+ 1px 竖线，在进度与已用边界对齐
- 下方标签行：10px 灰字，显示时间百分比 + 重置倒计时（"~2h left"）

**间距**：
- Bar 与标签间隔：4px
- Bar 之间分隔线：1px 虚线，上下间隔 6px

#### Video Gen（分段式）

**结构**：
```
VIDEO GEN                              0/3
┌─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─┐
│ ┌──┐ ┌──┐ ┌──┐                               │  14px 高
│ │  │ │  │ │  │                               │
│ └──┘ └──┘ └──┘                               │
│ 3 gen left                   ~2h left       │
└─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─┘
```

**细节**：
- 分段数量：根据 API 动态（本例 3 段）
- 每段：14px 高，1px 黑色边框，已用段实心，剩余段空心
- 间隔：3px gap
- 标签行：11px SemiBold "VIDEO GEN" + 右侧 "0/3"（11px Bold）
- 下方说明：9–10px，显示"3 gen left" + "~2h left"

#### 底部状态栏

**结构**：
```
┌─────────────────────────────────────────────────┐
│ ● Last OK   Sync 03:00  Next 03:05             │
└─────────────────────────────────────────────────┘
```

**细节**：
- 1px 上边框分隔
- 在线状态：实心方块（5×5px） + "Last OK"/"Last Fail"（10px）
- 同步时间："Sync HH:MM"（10px） + "Next HH:MM"（10px）
- 字色：主色（实心指示）+ 灰色（标签）

---

## 6. 动态逻辑

### 6.1 更新频率

| 组件 | 更新频率 | 触发源 |
|------|--------|--------|
| 时间显示 | 每 30 秒 | 本地系统时间 |
| Quota 进度条 | 每 30 秒 | 本地重新计算 |
| 游标位置 | 每 30 秒 | 当前时间 % 周期 |
| API 同步 | 每 5 分钟 | 定时任务 |

### 6.2 Quota 重置逻辑

**5-Hour Quota**：
- 重置时间：05:00, 10:00, 15:00, 20:00, 00:00（UTC+8 时区示例）
- 计算当前周期的起点和终点
- `timePercent = (当前时间 - 周期起点) / 周期长度 * 100`
- `resetLabel = formatResetTime(周期终点 - 当前时间)`

**Weekly Quota**：
- 重置时间：每周一 00:00
- `timePercent = (当前周数秒 - 周一 00:00 秒) / (7*24*60*60) * 100`
- 显示"~Xd left" 或 "~Xh left"

**Video Gen**：
- 重置时间：每天 00:00
- `remainingQuota = total - used`
- 显示"X gen left" 或 "No quota"
- 重置倒计时：距离午夜的时间

### 6.3 API 同步状态

- **lastRead**：最后一次成功/失败的 API 调用时刻（HH:MM）
- **nextRead**：下一次计划调用的时刻（HH:MM）
- **lastStatus**："ok" / "fail"，决定显示 "Last OK" 或 "Last Fail"

---

## 7. 交付物清单

### 7.1 设计文件

```
/project-root
├── DELIVERY_GUIDE_ESP32_LVGL.md        ← 本文件
├── LVGL_DESIGN_REFERENCE.md            ← 像素精确参考
├── public/
│   ├── fonts/
│   │   └── unscii-16.woff              ← 点阵字体（可选替选）
│   └── screenshots/
│       ├── final.png                   ← 最终设计效果
│       ├── with-wifi.png               ← 带 WiFi 的设计稿
│       └── cursors-aligned.png         ← 游标对齐验证
└── components/lcd/
    ├── LcdScreen.tsx                   ← 逻辑参考（Client Component）
    ├── StatusBar.tsx                   ← 状态栏参考
    ├── MiniMaxPanel.tsx                ← 主面板参考
    ├── QuotaBar.tsx                    ← 进度条组件参考
    ├── VideoQuota.tsx                  ← 视频配额参考
    ├── LcdShell.tsx                    ← 屏幕外壳参考
    └── ThemeContext.tsx                ← 主题系统（参考用）
```

### 7.2 必需交付给开发团队的文件

1. **设计规格文档**
   - ✅ `DELIVERY_GUIDE_ESP32_LVGL.md`（本文件）
   - ✅ `LVGL_DESIGN_REFERENCE.md`（像素规格）

2. **参考代码**
   - ✅ React 组件源代码（`components/lcd/`）
   - 作用：逻辑流程、状态管理、计算方法的参考
   - 注意：不能直接在 LVGL 中使用，需翻译为 C/Lua

3. **设计截图**
   - ✅ 最终设计效果图（1920×1080 渲染）
   - ✅ 单独的组件截图（Quota Bar、Video Gen、WiFi 图标等）

4. **字体资源**
   - Montserrat：下载 LVGL 官方预编译版本
   - unscii-16：`public/fonts/unscii-16.woff`（可选）

---

## 8. 移植指南（LVGL C/Lua）

### 8.1 LVGL 屏幕初始化

```c
#include "lvgl.h"

// 屏幕尺寸定义
#define SCREEN_WIDTH  400
#define SCREEN_HEIGHT 300

void iot_monitor_init(void) {
    // 1. 初始化 LVGL
    lv_init();

    // 2. 创建屏幕缓冲区
    static lv_disp_draw_buf_t draw_buf;
    static uint8_t buf[SCREEN_WIDTH * SCREEN_HEIGHT / 8];
    lv_disp_draw_buf_init(&draw_buf, buf, NULL, SCREEN_WIDTH * SCREEN_HEIGHT);

    // 3. 初始化驱动（例如 EPD、反射式 LCD）
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.draw_buf = &draw_buf;
    disp_drv.hor_res = SCREEN_WIDTH;
    disp_drv.ver_res = SCREEN_HEIGHT;
    disp_drv.flush_cb = your_display_flush_callback;  // 自定义刷新函数
    disp_drv.rounder_cb = your_rounder_cb;            // 自定义舍入（可选）
    lv_disp_drv_register(&disp_drv);

    // 4. 加载字体
    LV_FONT_DECLARE(montserrat_18_font);
    LV_FONT_DECLARE(montserrat_11_font);
    // ... 其他字号

    // 5. 创建主容器
    lv_obj_t *scr = lv_disp_get_scr_act(NULL);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0xD4CC8A), 0);

    // 6. 创建各个组件（见下文）
    create_status_bar(scr);
    create_main_panel(scr);
}
```

### 8.2 状态栏实现示例（C 语言伪代码）

```c
lv_obj_t *status_bar_obj;

void create_status_bar(lv_obj_t *parent) {
    // 创建背景面板
    status_bar_obj = lv_obj_create(parent);
    lv_obj_set_size(status_bar_obj, SCREEN_WIDTH, 30);
    lv_obj_set_pos(status_bar_obj, 0, 0);
    lv_obj_set_style_bg_color(status_bar_obj, lv_color_hex(0x1A1F0A), 0);
    lv_obj_set_layout(status_bar_obj, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(status_bar_obj, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(status_bar_obj, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // 左侧：时间
    lv_obj_t *time_label = lv_label_create(status_bar_obj);
    lv_obj_set_style_text_font(time_label, &montserrat_18_bold, 0);
    lv_label_set_text(time_label, "14:35");

    // 中间：Provider 标签（带边框）
    lv_obj_t *provider_label = lv_label_create(status_bar_obj);
    lv_obj_set_style_text_font(provider_label, &montserrat_10_font, 0);
    lv_obj_set_style_border_width(provider_label, 1, 0);
    lv_label_set_text(provider_label, "MiniMax");

    // 右侧：温湿度、电池、WiFi
    // ... 类似创建

    // 定时器更新时间
    lv_timer_create(update_time_cb, 30000, NULL);  // 每 30s 更新
}

void update_time_cb(lv_timer_t *timer) {
    // 获取当前时间
    time_t now = time(NULL);
    struct tm *timeinfo = localtime(&now);
    char time_str[6];
    strftime(time_str, sizeof(time_str), "%H:%M", timeinfo);

    // 更新标签
    lv_label_set_text(time_label, time_str);
}
```

### 8.3 Quota Bar 实现要点

```c
// 进度条主体
lv_obj_t *bar = lv_obj_create(parent);
lv_obj_set_size(bar, 300, 16);
lv_obj_set_style_border_width(bar, 1, 0);

// 已用部分（填充）
lv_obj_t *fill = lv_obj_create(bar);
lv_obj_set_width(fill, (usedPercent / 100.0) * 300);
lv_obj_set_height(fill, 16);
lv_obj_set_style_bg_color(fill, lv_color_hex(0x1A1F0A), 0);

// 游标（SVG 或 canvas 绘制）
// 或使用 lv_line 绘制竖线 + SVG 三角形
draw_cursor(bar, timePercent);

// 进度刻度线（每 25% 一条）
for (int i = 1; i <= 3; i++) {
    int x = (i * 25 / 100.0) * 300;
    draw_vertical_line(bar, x, 0, 16);
}
```

### 8.4 更新逻辑

使用 `lv_timer` 定时刷新各个组件数据：

```c
// 每 30 秒更新一次
lv_timer_create(refresh_quota_bars, 30000, NULL);

void refresh_quota_bars(lv_timer_t *timer) {
    time_t now = time(NULL);
    
    // 重新计算进度百分比
    float period_percent = calculate_period_progress(now);
    float weekly_percent = calculate_weekly_progress(now);
    
    // 更新 UI
    update_bar_fill(quota_bar_5h, period_percent);
    update_bar_fill(quota_bar_weekly, weekly_percent);
    update_cursor_position(quota_bar_5h, period_percent);
    
    // 更新重置倒计时
    char reset_str[32];
    format_reset_countdown(period_percent, reset_str);
    lv_label_set_text(reset_label, reset_str);
}
```

---

## 9. 常见问题

### Q: 能否在 LVGL 中直接使用 React 代码？
**A**: 否。React 是 JavaScript UI 框架，LVGL 是 C 库。需要将逻辑和计算方法翻译为 C/Lua，UI 组件则用 LVGL API 重新实现。

### Q: 屏幕刷新频率如何设定？
**A**: 根据实际硬件（EPD、LCD 刷新延迟）：
- 反射式 LCD：200–500ms 可接受
- E-Ink：1–2s（刷新慢，功耗低）
- 设计中的 30s 更新间隔足够

### Q: 字体预编译时需要哪些字号？
**A**: 最小集合：
```
Montserrat Regular (400):    9, 10, 11px
Montserrat SemiBold (600):   11px
Montserrat Bold (700):       11, 18px
```
可按需扩展到 12–24px。

### Q: 单色屏如何处理灰度？
**A**: 
- 纯黑白模式（推荐）：只用 LCD_BG 和 LCD_FG
- 抖动（Dithering）：用 1bit 像素模拟灰度（高频噪声）
- 不建议在动态内容上用抖动（刷新伪影）

### Q: 游标精确对齐很困难，有简化方案吗？
**A**: 简化方案：
- 去掉竖线，只保留三角形标记
- 使用 bar 上的对比色（反色）标记时间位置
- 用二级标签（文字）标注"Time 60%"代替视觉游标

---

## 10. 设计验收清单

移植完成后，用以下清单验收：

- [ ] 屏幕分辨率正确（400×300）
- [ ] 文字清晰可读（字号≥9px，对比度高）
- [ ] Quota Bar 进度条与实际时间对齐
- [ ] 游标三角形与竖线水平对齐
- [ ] Video Gen 的段数与 API 返回值一致
- [ ] 时间每 30s 刷新，日期/星期正确
- [ ] WiFi 信号显示 0–3 格
- [ ] 电池显示百分比与图标一致
- [ ] 在单色屏上无灰度抖动（或可接受）
- [ ] 所有边框为 1px 实线
- [ ] 间隔和 padding 视觉舒适（无拥挤）

---

## 11. 联系与支持

- **设计源文件**：本仓库 `components/lcd/` 下的 React 代码
- **参考截图**：`public/screenshots/` 下的 PNG 文件
- **字体来源**：
  - Montserrat：https://github.com/lvgl/lvgl/tree/master/src/font
  - unscii-16：http://viznut.fi/unscii/
- **LVGL 官方文档**：https://docs.lvgl.io/

---

**版本**：v1.0  
**日期**：2026-06-28  
**设计工具**：Next.js 16 + React 19 + Tailwind CSS  
**目标平台**：ESP32 + ESP-IDF + LVGL 8/9
