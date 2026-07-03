"use client"

import { useTheme } from "./ThemeContext"

interface QuotaBarProps {
  label: string // "5H QUOTA" | "WEEKLY"
  usedPercent: number // 0–100，已使用百分比
  timePercent: number // 0–100，当前时间在周期内的位置百分比
  resetLabel: string // "重置于 2小时后" 等
  usedAmount?: string // "12.3K used"
}

/** 带游标的进度条组件，模拟 LVGL lv_bar */
export function QuotaBar({
  label,
  usedPercent,
  timePercent,
  resetLabel,
  usedAmount,
}: QuotaBarProps) {
  const { theme } = useTheme()
  const BAR_HEIGHT = 16

  // 游标位置（夹在 1–99% 之间，避免在两端溢出边框）
  const cursorPct = Math.max(1, Math.min(99, timePercent))
  const cursorLeft = `${cursorPct}%`
  // 游标处于已填充区域时，竖线用底色（亮），否则用前景色（暗），确保始终可见
  const cursorOverFill = cursorPct <= usedPercent

  const labelText = theme.uppercase ? label.toUpperCase() : label

  return (
    <div style={{ fontFamily: theme.fontMain, color: theme.fg }}>
      {/* 标签行 */}
      <div className="flex items-baseline justify-between" style={{ marginBottom: 3 }}>
        <span style={{ fontSize: 11, letterSpacing: theme.letterSpacing, fontWeight: 600 }}>
          {labelText}
        </span>
        <div className="flex items-baseline gap-2" style={{ fontSize: 9, color: theme.mid }}>
          {usedAmount && <span>{usedAmount}</span>}
          <span style={{ color: theme.fg, fontSize: 11, fontWeight: 700 }}>
            {Math.round(usedPercent)}%
          </span>
        </div>
      </div>

      {/* 游标标记（位于条形正上方）— 用 ::before 伪元素的 clip-path 绘制三角形，尖端与竖线共轴 */}
      <div
        className="relative"
        style={{
          height: 6,
          overflow: "visible",
          pointerEvents: "none",
        }}
      >
        {/* 游标箭头：纯 CSS 三角形，尖端在最下方中心 */}
        <div
          style={{
            position: "absolute",
            left: cursorLeft,
            transform: "translateX(-50%)",
            bottom: 0,
            width: 6,
            height: 5,
            clipPath: "polygon(50% 100%, 0% 0%, 100% 0%)",
            background: theme.fg,
          }}
        />
      </div>

      {/* 进度条主体 */}
      <div
        className="relative"
        style={{
          height: BAR_HEIGHT,
          border: `1px solid ${theme.fg}`,
          borderRadius: theme.cornerRadius,
          background: "transparent",
          boxSizing: "border-box",
          overflow: "hidden",
        }}
      >
        {/* 已使用部分（实心填充） */}
        <div
          style={{
            position: "absolute",
            left: 0,
            top: 0,
            width: `${usedPercent}%`,
            height: "100%",
            background: theme.fg,
          }}
        />
        {/* 内部刻度线（每 25% 一条） */}
        {[25, 50, 75].map((tick) => (
          <div
            key={tick}
            style={{
              position: "absolute",
              left: `${tick}%`,
              top: 0,
              width: 1,
              height: "100%",
              background: tick <= usedPercent ? theme.bg : theme.dim,
              opacity: 0.5,
            }}
          />
        ))}
        {/* 时间游标竖线（1px 贯穿整条，与上方三角形同一中心，便于与用量边缘对比） */}
        <div
          style={{
            position: "absolute",
            left: cursorLeft,
            top: 0,
            width: 1,
            height: "100%",
            transform: "translateX(-50%)",
            background: cursorOverFill ? theme.bg : theme.fg,
          }}
        />
      </div>

      {/* 底部信息：左=时间进度，右=重置 */}
      <div
        className="flex items-center justify-between"
        style={{ marginTop: 2, fontSize: 9, color: theme.dim }}
      >
        <span style={{ color: theme.mid }}>
          {theme.uppercase ? "TIME" : "Time"} {Math.round(timePercent)}%
        </span>
        <span>{resetLabel}</span>
      </div>
    </div>
  )
}
