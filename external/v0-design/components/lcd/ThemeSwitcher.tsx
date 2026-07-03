"use client"

import { useTheme } from "./ThemeContext"
import { THEMES, type ThemeId } from "@/lib/themes"

const ORDER: ThemeId[] = ["montserrat", "unscii"]

/** 字体切换器 — 单色屏上唯一可感知的区别就是字体，均为 LVGL 内置字体 */
export function ThemeSwitcher() {
  const { themeId, setThemeId } = useTheme()

  return (
    <div className="flex flex-col items-center gap-3">
      <span className="text-xs uppercase tracking-widest text-neutral-500">
        Font (LVGL built-in)
      </span>
      <div className="flex items-center gap-1 rounded-full bg-neutral-800 p-1">
        {ORDER.map((id) => {
          const active = id === themeId
          return (
            <button
              key={id}
              type="button"
              onClick={() => setThemeId(id)}
              className="rounded-full px-4 py-1.5 text-sm transition-colors"
              style={{
                background: active ? "#f5f5f3" : "transparent",
                color: active ? "#1a1a1a" : "#a3a3a3",
                fontWeight: active ? 600 : 400,
              }}
            >
              {THEMES[id].name}
            </button>
          )
        })}
      </div>
    </div>
  )
}
