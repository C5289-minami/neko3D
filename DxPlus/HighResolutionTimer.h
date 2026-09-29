// ============================================================================
// OIC教材用モジュール - 大阪情報コンピュータ専門学校
// 作成者：Y.Tanaka
// このファイルは授業用教材として作成されています。
// ============================================================================

#pragma once
#include <windows.h>
#include <stdexcept>
#include <algorithm>

namespace DxPlus
{
    class HighResolutionTimer
    {
    public:
        HighResolutionTimer() { Init(); }

        // ------------------------------------------------------------
        // Life cycle
        // ------------------------------------------------------------
        void Reset() noexcept
        {
            LARGE_INTEGER now;
            QueryPerformanceCounter(&now);

            baseCount = now.QuadPart;
            prevFrameCount = now.QuadPart;

            stopCount = 0;
            pausedCounts = 0;

            rawDeltaTime = 0.0;
            deltaTime = 0.0;
            totalTime = 0.0;

            stopped = false;
        }

        void Start() noexcept
        {
            if (!stopped) return;

            LARGE_INTEGER now;
            QueryPerformanceCounter(&now);

            // 停止期間を加算して「動いていない時間」を除外
            pausedCounts += (now.QuadPart - stopCount);

            prevFrameCount = now.QuadPart;
            stopCount = 0;
            stopped = false;
        }

        void Stop() noexcept
        {
            if (stopped) return;

            LARGE_INTEGER now;
            QueryPerformanceCounter(&now);

            stopCount = now.QuadPart;
            stopped = true;

            rawDeltaTime = 0.0;
            deltaTime = 0.0;
        }

        bool IsStopped() const noexcept { return stopped; }

        // ------------------------------------------------------------
        // Settings
        // ------------------------------------------------------------
        // ゲームに渡すdtの最大値（暴走防止）
        void SetMaxDeltaTime(double maxDt) noexcept
        {
            maxDeltaTime = (maxDt > 0.0) ? maxDt : maxDeltaTime;
        }

        double GetMaxDeltaTime() const noexcept { return maxDeltaTime; }

        // ------------------------------------------------------------
        // Frame update
        // ------------------------------------------------------------
        // fpsCap: 0以下なら待たない。60なら60fps相当まで待つ。
        void Tick(double fpsCap = 0.0) noexcept
        {
            if (stopped)
            {
                rawDeltaTime = 0.0;
                deltaTime = 0.0;
                return;
            }

            // 1) 必要なら、前フレーム時刻(prevFrameCount)基準で待つ
            if (fpsCap > 0.0)
            {
                const LONGLONG capCounts = static_cast<LONGLONG>(countsPerSecond / fpsCap);
                const LONGLONG target = prevFrameCount + capCounts;

                LARGE_INTEGER now;
                for (;;)
                {
                    QueryPerformanceCounter(&now);
                    if (now.QuadPart >= target) break;

                    // 0msスリープ（スレッド譲る）
                    Sleep(0);
                }
            }

            // 2) 待った後の“今”で dt を確定
            LARGE_INTEGER now;
            QueryPerformanceCounter(&now);

            const LONGLONG cur = now.QuadPart;
            rawDeltaTime = (cur - prevFrameCount) * secondsPerCount;
            prevFrameCount = cur;

            // 3) 安全化
            if (rawDeltaTime < 0.0) rawDeltaTime = 0.0;

            // ここが「ゲームに渡すdt」：clamp済みで1本化
            deltaTime = rawDeltaTime;
            deltaTime = (std::min)(deltaTime, maxDeltaTime);

            // 4) 累積（※clamp後で進める＝暴走時に時間が飛ばない）
            totalTime += deltaTime;
        }

        // ------------------------------------------------------------
        // Getters
        // ------------------------------------------------------------
        // ゲーム側が使う dt（clamp済み）
        double GetDeltaTime() const noexcept { return deltaTime; }

        // 生の dt（デバッグ用）
        double GetRawDeltaTime() const noexcept { return rawDeltaTime; }

        // Resetからの累積時間（停止中は進まない）
        double GetTotalTime() const noexcept { return totalTime; }

        // QueryPerformanceCounter の生値（必要なら）
        LONGLONG GetPrevFrameCount() const noexcept { return prevFrameCount; }

        // Resetからの経過（raw基準で欲しい場合用：停止・一時停止を除外した「実時間」）
        // ※totalTimeとは違い「clampしない」計測値が欲しい時だけ使う
        double GetMeasuredTime() const noexcept
        {
            if (stopped)
            {
                return ((stopCount - pausedCounts) - baseCount) * secondsPerCount;
            }

            LARGE_INTEGER now;
            QueryPerformanceCounter(&now);
            return ((now.QuadPart - pausedCounts) - baseCount) * secondsPerCount;
        }

    private:
        void Init()
        {
            LARGE_INTEGER freq;
            if (!QueryPerformanceFrequency(&freq))
            {
                throw std::runtime_error("High-resolution timer not supported.");
            }

            countsPerSecond = freq.QuadPart;
            secondsPerCount = 1.0 / static_cast<double>(countsPerSecond);

            Reset();
        }

    private:
        // QPC
        LONGLONG countsPerSecond = 0;
        double   secondsPerCount = 0.0;

        // Time points
        LONGLONG baseCount = 0;
        LONGLONG prevFrameCount = 0;
        LONGLONG stopCount = 0;
        LONGLONG pausedCounts = 0;

        // Deltas
        double rawDeltaTime = 0.0;   // 生dt（計測値）
        double deltaTime = 0.0;   // ゲーム用dt（clamp済み）
        double totalTime = 0.0;   // ゲーム用dtで進む累積

        // Safety
        double maxDeltaTime = 1.0 / 30.0; // デフォ：物理も兼ねるならこの値が無難

        bool stopped = false;
    };
} // namespace DxPlus