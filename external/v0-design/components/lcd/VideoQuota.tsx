"use client"

import { useTheme } from "./ThemeContext"

interface VideoQuotaProps {
  total: number // 每日总次数（动态，例如 3）
  used: number // 已使用次数
}

/** 视频生成配额 — 分段式进度条（按次数切分为 N 段） */
export function VideoQuota({ total, used }: VideoQuotaProps) {
  const { theme } = useTheme()
  const remaining = Math.max(0, total - used)
  // 计算到午夜的剩余分钟数
  const now = new Date()
  const minutesToMidnight = 24 * 60 - (now.getHours() * 60 + now.getMinutes())
  const resetLabel = minutesToMidnight >= 60
    ? theme.uppercase
      ? `~${Math.round(minutesToMidnight / 60)}HR LEFT`
      : `~${Math.round(minutesToMidnight / 60)}h left`
    : theme.uppercase
      ? `${minutesToMidnight}MIN LEFT`
      : `${minutesToMidnight}m left`

  return (
    <div style={{ fontFamily: theme.fontMain, color: theme.fg }}>
      {/* 标签行 */}
      <div className="flex items-baseline justify-between" style={{ marginBottom: 4 }}>
        <span style={{ fontSize: 11, letterSpacing: theme.letterSpacing, fontWeight: 600 }}>
          {theme.uppercase ? "VIDEO GEN" : "Video Gen"}
        </span>
        <div className="flex items-baseline gap-2" style={{ fontSize: 9, color: theme.mid }}>
          <span style={{ color: theme.fg, fontSize: 11, fontWeight: 700 }}>
            {used}/{total}
          </span>
        </div>
      </div>

      {/* 分段进度条：N 段等宽 bar，已用段实心，剩余段空心 */}
      <div className="flex items-center" style={{ gap: 3 }}>
        {Array.from({ length: total }).map((_, i) => {
          const isUsed = i < used
          return (
            <div
              key={i}
              style={{
                flex: 1,
                height: 14,
                border: `1px solid ${theme.fg}`,
                borderRadius: theme.cornerRadius,
                background: isUsed ? theme.fg : "transparent",
                boxSizing: "border-box",
              }}
            />
          )
        })}
      </div>

      {/* 底部说明 */}
      <div
        className="flex items-center justify-between"
        style={{ marginTop: 2, fontSize: 9, color: theme.dim }}
      >
        <span style={{ color: theme.mid }}>
          {remaining > 0
            ? theme.uppercase
              ? `${remaining} GEN LEFT`
              : `${remaining} gen left`
            : theme.uppercase
              ? "NO QUOTA"
              : "No quota"}
        </span>
        <span>{resetLabel}</span>
      </div>
    </div>
  )
}
