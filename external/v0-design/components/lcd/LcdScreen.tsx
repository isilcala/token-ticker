"use client"

import { useEffect, useState } from "react"
import { LcdShell } from "./LcdShell"
import { StatusBar } from "./StatusBar"
import { MiniMaxPanel } from "./MiniMaxPanel"
import { ThemeProvider } from "./ThemeContext"
import { ThemeSwitcher } from "./ThemeSwitcher"
import {
  generateMiniMaxData,
  formatTime,
  formatDate,
  formatWeekday,
} from "@/lib/minimax-mock"
import type { MiniMaxData, ApiSync } from "./MiniMaxPanel"

/** 模拟环境传感器数据（±1 度小幅波动） */
function mockSensor(now: Date) {
  const base = 23 + Math.sin(now.getTime() / 300000) * 2
  return {
    temperature: Math.round(base * 10) / 10,
    humidity: Math.round(60 + Math.cos(now.getTime() / 400000) * 8),
  }
}

/** API 轮询间隔（毫秒）— 设备每 5 分钟读取一次 API */
const API_POLL_INTERVAL = 5 * 60_000

export function LcdScreen() {
  // 初始化为 null 避免 SSR/client 时间不匹配，useEffect 会在客户端设置真实值
  const [now, setNow] = useState<Date | null>(null)
  const [miniMaxData, setMiniMaxData] = useState<MiniMaxData | null>(null)
  const [apiSync, setApiSync] = useState<ApiSync | null>(null)
  const [battery] = useState(78)

  // 仅在客户端初始化并每 30 秒更新一次
  useEffect(() => {
    // 初始化
    const initDate = new Date()
    setNow(initDate)
    setMiniMaxData(generateMiniMaxData(initDate))
    setApiSync({
      lastRead: initDate,
      nextRead: new Date(initDate.getTime() + API_POLL_INTERVAL),
      lastStatus: "ok",
    })

    // 每 30 秒刷新一次
    const tick = () => {
      const d = new Date()
      setNow(d)
      setApiSync((prev) => {
        if (!prev) return null
        if (d.getTime() >= prev.nextRead.getTime()) {
          // 模拟一次 API 读取：90% 成功
          const status: ApiSync["lastStatus"] = Math.random() < 0.9 ? "ok" : "fail"
          if (status === "ok") {
            setMiniMaxData(generateMiniMaxData(d))
          }
          return {
            lastRead: d,
            nextRead: new Date(d.getTime() + API_POLL_INTERVAL),
            lastStatus: status,
          }
        }
        return prev
      })
    }

    const id = setInterval(tick, 30_000)
    return () => clearInterval(id)
  }, [])

  // 等待客户端初始化完成后再渲染实际内容
  if (!now || !miniMaxData || !apiSync) {
    return (
      <ThemeProvider>
        <div className="flex min-h-screen flex-col items-center justify-center gap-8 bg-neutral-900 p-8">
          <LcdShell>{/* 加载中... */}</LcdShell>
        </div>
      </ThemeProvider>
    )
  }

  const { temperature, humidity } = mockSensor(now)

  return (
    <ThemeProvider>
      <div className="flex min-h-screen flex-col items-center justify-center gap-8 bg-neutral-900 p-8">
        <LcdShell>
          {/* 固定状态栏 */}
          <StatusBar
            time={formatTime(now)}
            date={formatDate(now)}
            weekday={formatWeekday(now)}
            temperature={temperature}
            humidity={humidity}
            battery={battery}
            provider="MiniMax"
          />

          {/* 主内容区 — 剩余 270px 高度 */}
          <div style={{ height: 270, overflow: "hidden" }}>
            <MiniMaxPanel data={miniMaxData} apiSync={apiSync} />
          </div>
        </LcdShell>

        {/* 风格切换器（仅模拟器使用，不属于设备本身） */}
        <ThemeSwitcher />
      </div>
    </ThemeProvider>
  )
}
