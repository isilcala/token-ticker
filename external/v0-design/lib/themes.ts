/**
 * 设备屏幕视觉主题定义
 *
 * 这是一块 400×300 单色反射式 LCD（ESP32 + LVGL）。屏幕没有颜色，
 * 因此不同主题之间唯一真正的区别是「字体」。这里只提供 LVGL 实际内置、
 * 可直接烧录到设备上的两种字体：
 *   - Montserrat：LVGL 默认矢量字体（12–48px），可读性最好，推荐。
 *   - unscii-16：LVGL 内置的点阵位图字体（仅 ASCII），复古点阵观感。
 *
 * 所有屏幕内容共用同一套单色调色板，以如实反映真实硬件。
 */

export type ThemeId = "montserrat" | "unscii"

export interface Theme {
  id: ThemeId
  name: string // 模拟器中展示的字体名

  /* 字体 */
  fontMain: string
  fontDisplay: string

  /* 屏幕（单色，共用） */
  screenBg: string
  pixelGrid: boolean // 是否叠加细微的 LCD 像素网格

  /* 内容区灰度（单色屏只有黑/白，灰度仅用于模拟器层次） */
  fg: string // 主前景：填充 / 边框 / 文本
  dim: string // 次要文本
  mid: string // 中间灰度文本
  bg: string // = screenBg，用于反色（填充条上的刻度等）

  /* 顶部状态栏（反色：深底浅字） */
  statusBg: string
  statusFg: string
  statusDim: string
  statusMid: string
  statusBorder: string

  /* 设备外壳 */
  shellBg: string
  shellBorder: string
  shellGroove: string
  bezelRadius: number
  cornerRadius: number
  shellLabel: string
  led: string

  /* 排版风格 */
  uppercase: boolean // 标签是否全大写（点阵字体用全大写更协调）
  letterSpacing: string
}

/* 共用的单色调色板 —— 仿反射式单色 LCD（中性偏暖灰，近黑文字） */
const MONO = {
  screenBg: "#c4c7ba",
  fg: "#16180f",
  dim: "#7c8070",
  mid: "#4a4d3e",
  bg: "#c4c7ba",
  statusBg: "#16180f",
  statusFg: "#c4c7ba",
  statusDim: "#8b8f7e",
  statusMid: "#a7ab99",
  statusBorder: "#3a3d2e",
  shellBg: "linear-gradient(145deg, #34362c, #1c1e14)",
  shellBorder: "#0c0d08",
  shellGroove: "#0c0d08",
  shellLabel: "#6a6e54",
  led: "#9aa07e",
}

export const THEMES: Record<ThemeId, Theme> = {
  montserrat: {
    id: "montserrat",
    name: "Montserrat",
    fontMain: "'Montserrat', system-ui, sans-serif",
    fontDisplay: "'Montserrat', system-ui, sans-serif",
    ...MONO,
    pixelGrid: false,
    bezelRadius: 8,
    cornerRadius: 2,
    uppercase: false,
    letterSpacing: "0.01em",
  },

  unscii: {
    id: "unscii",
    name: "unscii (Pixel)",
    fontMain: "'unscii-16', monospace",
    fontDisplay: "'unscii-16', monospace",
    ...MONO,
    pixelGrid: true,
    bezelRadius: 6,
    cornerRadius: 0,
    uppercase: true,
    letterSpacing: "0.02em",
  },
}

export const DEFAULT_THEME: ThemeId = "montserrat"
