"use client"

import { useTheme } from "./ThemeContext"

interface StatusBarProps {
  time: string // "14:35"
  date: string // "2025-06-28"
  weekday: string // "SAT"
  temperature: number // 24
  humidity: number // 65
  battery: number // 0–100
  provider: string // "MINIMAX"
  wifiSignal?: number // 0–4，表示 WiFi 信号强度，默认 3
}

/** LCD 风格顶部状态栏，高度 30px */
export function StatusBar({
  time,
  date,
  weekday,
  temperature,
  humidity,
  battery,
  provider,
  wifiSignal = 3,
}: StatusBarProps) {
  const { theme } = useTheme()

  return (
    <div
      className="flex items-center justify-between px-2"
      style={{
        height: 30,
        background: theme.statusBg,
        color: theme.statusFg,
        fontFamily: theme.fontMain,
        fontSize: 10,
        letterSpacing: theme.letterSpacing,
        borderBottom: `1px solid ${theme.statusBorder}`,
      }}
    >
      {/* 左侧：时间 + 日期 + 星期 */}
      <div className="flex items-center gap-2">
        <span
          style={{
            fontFamily: theme.fontDisplay,
            fontSize: 18,
            fontWeight: 700,
            letterSpacing: theme.id === "unscii" ? "0.06em" : "0",
            lineHeight: 1,
          }}
        >
          {time}
        </span>
        <div className="flex flex-col" style={{ lineHeight: 1.2 }}>
          <span style={{ color: theme.statusMid, fontSize: 10 }}>{date}</span>
          <span style={{ color: theme.statusDim, fontSize: 9, letterSpacing: "0.08em" }}>
            {weekday}
          </span>
        </div>
      </div>

      {/* 中间：Provider 名称（设备全局标识，多 Provider 时显示当前监控对象） */}
      <div
        style={{
          fontSize: 10,
          fontWeight: 700,
          letterSpacing: "0.15em",
          color: theme.statusFg,
          border: `1px solid ${theme.statusBorder}`,
          borderRadius: theme.cornerRadius,
          padding: "1px 5px",
          lineHeight: 1.3,
        }}
      >
        {theme.uppercase ? provider.toUpperCase() : provider}
      </div>

      {/* 右侧：温湿度 + 电量 + WiFi 信号 */}
      <div className="flex items-center gap-2">
        <span style={{ fontSize: 10, color: theme.statusMid }}>
          {temperature}&deg;C {humidity}%
        </span>
        <div className="flex items-center gap-1">
          <BatteryIcon level={battery} />
          <span style={{ fontSize: 9, color: theme.statusMid }}>{battery}%</span>
        </div>
        <WiFiIcon signal={wifiSignal} />
      </div>
    </div>
  )
}

function BatteryIcon({ level }: { level: number }) {
  const { theme } = useTheme()
  const filled = Math.round((level / 100) * 4)
  return (
    <div className="flex items-center gap-0.5">
      <div
        className="flex items-center gap-px p-px"
        style={{
          border: `1px solid ${theme.statusFg}`,
          borderRadius: theme.cornerRadius,
          width: 22,
          height: 10,
          boxSizing: "border-box",
        }}
      >
        {Array.from({ length: 4 }).map((_, i) => (
          <div
            key={i}
            style={{
              flex: 1,
              height: "100%",
              background: i < filled ? theme.statusFg : "transparent",
            }}
          />
        ))}
      </div>
      {/* 电池正极凸头 */}
      <div
        style={{
          width: 2,
          height: 5,
          background: theme.statusFg,
        }}
      />
    </div>
  )
}

function WiFiIcon({ signal }: { signal: number }) {
  const { theme } = useTheme()
  // WiFi 图标：三层弧线，从下往上根据信号强度逐个显示
  // signal 0–1：1 格，2–3：2 格，4+：3 格
  const bars = signal <= 1 ? 1 : signal <= 3 ? 2 : 3

  return (
    <svg
      width="10"
      height="9"
      viewBox="0 0 10 9"
      style={{ display: "block", flexShrink: 0 }}
    >
      {/* 底层弧线（最弱信号始终显示） */}
      <path
        d="M 5 8 Q 3 6 1 4"
        stroke={theme.statusFg}
        strokeWidth="1"
        fill="none"
        strokeLinecap="round"
      />
      {/* 中层弧线（信号 ≥ 2 时显示） */}
      {bars >= 2 && (
        <path
          d="M 5 6 Q 3.5 4.5 2 3"
          stroke={theme.statusFg}
          strokeWidth="1"
          fill="none"
          strokeLinecap="round"
        />
      )}
      {/* 顶层弧线（信号 ≥ 3 时显示） */}
      {bars >= 3 && (
        <path
          d="M 5 4 Q 4 3 3 2"
          stroke={theme.statusFg}
          strokeWidth="1"
          fill="none"
          strokeLinecap="round"
        />
      )}
    </svg>
  )
}
