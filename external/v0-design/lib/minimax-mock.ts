/**
 * MiniMax 模拟数据生成逻辑
 * 根据当前真实时间计算各 Quota 的进度
 */

import type { MiniMaxData } from "@/components/lcd/MiniMaxPanel"

/** MiniMax 5小时段重置时间点（小时） */
const RESET_HOURS = [0, 5, 10, 15, 20, 24]

/** 计算当前 5H/4H 段的信息 */
function getPeriodQuotaInfo(now: Date): MiniMaxData["periodQuota"] {
  const h = now.getHours()
  const m = now.getMinutes()
  const totalMinutes = h * 60 + m

  // 找到当前所在的时间段
  let segStart = 0
  let segEnd = 300 // 默认 5H = 300min
  for (let i = 0; i < RESET_HOURS.length - 1; i++) {
    const start = RESET_HOURS[i] * 60
    const end = RESET_HOURS[i + 1] * 60
    if (totalMinutes >= start && totalMinutes < end) {
      segStart = start
      segEnd = end
      break
    }
  }

  const segDurationMin = segEnd - segStart
  const segHours = segDurationMin / 60
  const elapsedMin = totalMinutes - segStart
  const timePercent = (elapsedMin / segDurationMin) * 100
  const remainingMin = segEnd - totalMinutes

  // 模拟已使用量（固定种子，基于时间段开始后经过的比例）
  const mockUsedRatio = 0.28 + Math.sin(segStart * 0.1) * 0.15
  const usedPercent = Math.max(5, Math.min(95, mockUsedRatio * 100))
  const totalK = segHours === 4 ? 40 : 50
  const usedK = (usedPercent / 100) * totalK

  return {
    hours: segHours === 4 ? 4 : 5,
    usedPercent,
    timePercent: Math.min(99, timePercent),
    resetInMinutes: remainingMin,
    usedTokens: usedK >= 1 ? `${usedK.toFixed(1)}K` : `${Math.round(usedK * 1000)}`,
    totalTokens: `${totalK}K`,
  }
}

/** 计算本周 Quota 信息 */
function getWeeklyQuotaInfo(now: Date): MiniMaxData["weeklyQuota"] {
  // 本周第一天（周一 0:00）
  const dayOfWeek = now.getDay() === 0 ? 6 : now.getDay() - 1 // 0=Mon
  const weekElapsedMin = dayOfWeek * 24 * 60 + now.getHours() * 60 + now.getMinutes()
  const weekTotalMin = 7 * 24 * 60
  const timePercent = (weekElapsedMin / weekTotalMin) * 100

  // 模拟使用量
  const usedPercent = 18 + (timePercent * 0.35)
  const totalMillion = 2
  const usedM = (usedPercent / 100) * totalMillion

  // 距离下周一 0:00 的剩余分钟数
  const resetInMinutes = weekTotalMin - weekElapsedMin

  return {
    usedPercent: Math.min(95, usedPercent),
    timePercent: Math.min(99, timePercent),
    resetInMinutes,
    usedTokens: usedM >= 1 ? `${usedM.toFixed(2)}M` : `${Math.round(usedM * 1000)}K`,
    totalTokens: `${totalMillion}M`,
  }
}

/** 计算每日视频配额 */
function getVideoQuotaInfo(now: Date): MiniMaxData["videoQuota"] {
  const h = now.getHours()
  // 模拟：下午后用得多
  const used = h >= 18 ? 2 : h >= 12 ? 1 : 0

  return {
    total: 3,
    used,
  }
}

/** 生成当前时刻的完整 MiniMax 模拟数据 */
export function generateMiniMaxData(now: Date): MiniMaxData {
  return {
    periodQuota: getPeriodQuotaInfo(now),
    weeklyQuota: getWeeklyQuotaInfo(now),
    videoQuota: getVideoQuotaInfo(now),
  }
}

/** 格式化时间为 HH:MM */
export function formatTime(d: Date): string {
  return `${String(d.getHours()).padStart(2, "0")}:${String(d.getMinutes()).padStart(2, "0")}`
}

/** 格式化日期为 YYYY-MM-DD */
export function formatDate(d: Date): string {
  return `${d.getFullYear()}-${String(d.getMonth() + 1).padStart(2, "0")}-${String(d.getDate()).padStart(2, "0")}`
}

const WEEKDAYS = ["SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"]
export function formatWeekday(d: Date): string {
  return WEEKDAYS[d.getDay()]
}
