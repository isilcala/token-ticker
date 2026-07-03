"use client"

import { ReactNode } from "react"
import { useTheme } from "./ThemeContext"

interface LcdShellProps {
  children: ReactNode
}

/**
 * 设备外壳 — 固定 400×300 像素内容区
 * 外观根据当前主题切换
 */
export function LcdShell({ children }: LcdShellProps) {
  const { theme } = useTheme()

  return (
    <div
      className="relative rounded-2xl p-4"
      style={{
        background: theme.shellBg,
        boxShadow:
          "0 8px 32px rgba(0,0,0,0.5), inset 0 1px 0 rgba(255,255,255,0.1), inset 0 -1px 0 rgba(0,0,0,0.4)",
        border: `2px solid ${theme.shellBorder}`,
      }}
    >
      {/* 屏幕凹槽 */}
      <div
        className="p-1"
        style={{
          background: theme.shellGroove,
          borderRadius: theme.bezelRadius,
          boxShadow: "inset 0 2px 8px rgba(0,0,0,0.7)",
        }}
      >
        {/* 显示面板 — 严格 400×300 */}
        <div
          className="relative overflow-hidden"
          style={{
            width: 400,
            height: 300,
            borderRadius: Math.max(0, theme.bezelRadius - 3),
            backgroundColor: theme.screenBg,
            imageRendering: theme.id === "unscii" ? "pixelated" : "auto",
            fontFamily: theme.fontMain,
            color: theme.fg,
          }}
        >
          {/* 细微 LCD 像素网格叠层 */}
          {theme.pixelGrid && (
            <div
              className="absolute inset-0 pointer-events-none z-10"
              style={{
                backgroundImage:
                  "repeating-linear-gradient(0deg, rgba(0,0,0,0.035) 0px, rgba(0,0,0,0.035) 1px, transparent 1px, transparent 3px), repeating-linear-gradient(90deg, rgba(0,0,0,0.035) 0px, rgba(0,0,0,0.035) 1px, transparent 1px, transparent 3px)",
              }}
            />
          )}
          {/* 内容区 */}
          <div className="relative z-0 w-full h-full">{children}</div>
        </div>
      </div>

      {/* 设备底部标签 */}
      <div className="mt-3 flex items-center justify-between px-1">
        <div
          className="uppercase"
          style={{
            fontFamily: theme.fontMain,
            color: theme.shellLabel,
            fontSize: 9,
            letterSpacing: "0.15em",
          }}
        >
          LLM Monitor v1.0
        </div>
        {/* 电源 LED 指示灯 */}
        <div
          className="w-2 h-2 rounded-full"
          style={{
            background: theme.led,
            boxShadow: `0 0 4px ${theme.led}, 0 0 8px ${theme.led}66`,
          }}
        />
      </div>
    </div>
  )
}
