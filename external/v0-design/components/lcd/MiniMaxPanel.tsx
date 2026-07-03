"use client"

import { QuotaBar } from "./QuotaBar"
import { VideoQuota } from "./VideoQuota"
import { useTheme } from "./ThemeContext"

export interface MiniMaxData {
  // 5小时/4小时段 Quota
  periodQuota: {
    hours: 4 | 5
    usedPercent: number // 已用百分比
    timePercent: number // 时间进度百分比
    resetInMinutes: number // 距离重置的剩余分钟数
    usedTokens: string // "12.4K"
    totalTokens: string // "50K"
  }
  // 周 Quota
  weeklyQuota: {
    usedPercent: number
    timePercent: number
    resetInMinutes: number
    usedTokens: string
    totalTokens: string
  }
  // 每日视频
  videoQuota: {
    total: number
    used: number
  }
}

export interface ApiSync {
  lastRead: Date
  nextRead: Date
  lastStatus: "ok" | "fail" // 最后一次请求结果
}

function hhmm(d: Date): string {
  return `${String(d.getHours()).padStart(2, "0")}:${String(d.getMinutes()).padStart(2, "0")}`
}

/** 根据剩余分钟数生成"距离重置"标签（全英文，大写/小写两种风格） */
function formatReset(minutes: number, uppercase: boolean): string {
  const days = Math.floor(minutes / (24 * 60))
  const hours = Math.round(minutes / 60)
  if (uppercase) {
    if (days >= 1) return `${days}D LEFT`
    if (minutes >= 90) return `~${hours}HR`
    if (minutes >= 45) return "~1HR"
    if (minutes >= 25) return "~30MIN"
    return `${minutes}MIN`
  }
  if (days >= 1) return `${days}d left`
  if (minutes >= 90) return `~${hours}h left`
  if (minutes >= 45) return "~1h left"
  if (minutes >= 25) return "~30m left"
  return `${minutes}m left`
}

/** MiniMax Provider 专属面板 */
export function MiniMaxPanel({ data, apiSync }: { data: MiniMaxData; apiSync: ApiSync }) {
  const { theme } = useTheme()
  const { periodQuota, weeklyQuota, videoQuota } = data

  const ok = apiSync.lastStatus === "ok"

  return (
    <div
      className="flex flex-col justify-between"
      style={{
        padding: "8px 10px 6px",
        fontFamily: theme.fontMain,
        color: theme.fg,
        height: "100%",
        boxSizing: "border-box",
      }}
    >
      {/* 5H / 4H Quota Bar — 标签直接说明周期，无需额外 Badge */}
      <div>
        <QuotaBar
          label={theme.uppercase ? `${periodQuota.hours}-HOUR QUOTA` : `${periodQuota.hours}-Hour Quota`}
          usedPercent={periodQuota.usedPercent}
          timePercent={periodQuota.timePercent}
          resetLabel={formatReset(periodQuota.resetInMinutes, theme.uppercase)}
          usedAmount={`${periodQuota.usedTokens}/${periodQuota.totalTokens}`}
        />
      </div>

      {/* 分隔线 */}
      <div style={{ borderTop: `1px solid ${theme.dim}`, opacity: 0.5 }} />

      {/* Weekly Quota Bar */}
      <div>
        <QuotaBar
          label={theme.uppercase ? "WEEKLY QUOTA" : "Weekly Quota"}
          usedPercent={weeklyQuota.usedPercent}
          timePercent={weeklyQuota.timePercent}
          resetLabel={formatReset(weeklyQuota.resetInMinutes, theme.uppercase)}
          usedAmount={`${weeklyQuota.usedTokens}/${weeklyQuota.totalTokens}`}
        />
      </div>

      {/* 分隔线 */}
      <div style={{ borderTop: `1px solid ${theme.dim}`, opacity: 0.5 }} />

      {/* 每日视频配额 */}
      <div>
        <VideoQuota total={videoQuota.total} used={videoQuota.used} />
      </div>

      {/* 底部状态栏：最后请求结果 + API 读取时间 */}
      <div
        className="flex items-center justify-between"
        style={{
          borderTop: `1px solid ${theme.dim}`,
          paddingTop: 4,
          fontSize: 10,
          color: theme.dim,
          letterSpacing: theme.letterSpacing,
        }}
      >
        {/* 最后一次请求结果指示 */}
        <div className="flex items-center gap-1">
          {/* 状态符号：成功=实心方块，失败=空心方块带斜线感 */}
          <div
            style={{
              width: 6,
              height: 6,
              border: `1px solid ${theme.fg}`,
              borderRadius: theme.cornerRadius,
              background: ok ? theme.fg : "transparent",
            }}
          />
          <span style={{ color: theme.mid }}>
            {ok
              ? theme.uppercase
                ? "LAST OK"
                : "Last OK"
              : theme.uppercase
                ? "LAST FAIL"
                : "Last Fail"}
          </span>
        </div>

        {/* API 读取时间：上次 / 下次 */}
        <div className="flex items-center gap-3">
          <span>
            {theme.uppercase ? "SYNC" : "Sync"}{" "}
            <span style={{ color: theme.fg }}>{hhmm(apiSync.lastRead)}</span>
          </span>
          <span>
            {theme.uppercase ? "NEXT" : "Next"}{" "}
            <span style={{ color: theme.fg }}>{hhmm(apiSync.nextRead)}</span>
          </span>
        </div>
      </div>
    </div>
  )
}
