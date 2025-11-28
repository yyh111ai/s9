#include "rate_monitor.h"
#include "usart.h"
#include "main.h"
#include <stdio.h>
#include <string.h> // 需要包含 string.h 以使用 memset

// 定义计算平均值所使用的历史包数量
#define RATE_AVG_COUNT 10 // 使用10个包来计算平均值, 获得更平滑的速率

/**
 * @brief 内部结构体, 用于保存一个独立数据流(如TX或RX)的速率统计状态.
 */
typedef struct {
    // --- 历史记录 (用于计算平滑平均值) ---
    uint16_t len_history[RATE_AVG_COUNT];     // 历史包长度 (字节)
    uint32_t tick_history[RATE_AVG_COUNT];    // 历史包时间戳 (ms)
    uint8_t  history_index;                   // 当前历史记录的索引 (循环)
    uint8_t  history_count;                   // 当前有多少条历史记录 (最多 RATE_AVG_COUNT)
    
    // --- 累计与瞬时 ---
    uint64_t grand_total_bytes;               // 累计总字节数
    uint32_t last_packet_tick;                // 上一个包的时间戳 (用于计算瞬时速率)
    
    // --- 标识 ---
    const char* stream_name;                  // 用于打印的流名称 (例如 "RX" 或 "TX")
} rate_monitor_instance_t;


// --- 模块的静态(私有)变量 ---

// 实例化两个监视器: 一个用于 RX, 一个用于 TX
static rate_monitor_instance_t s_rx_monitor;
static rate_monitor_instance_t s_tx_monitor;


// --- 内部辅助函数 ---

/**
 * @brief (内部) 初始化单个监视器实例.
 * @param monitor: 指向要初始化的监视器实例的指针.
 * @param name: 用于打印的流名称 (例如 "RX").
 */
static void _monitor_instance_init(rate_monitor_instance_t* monitor, const char* name)
{
    // 使用 memset 将整个结构体清零
    memset(monitor, 0, sizeof(rate_monitor_instance_t));
    // 设置流名称
    monitor->stream_name = name;
}

/**
 * @brief (内部) 核心更新函数. 计算并打印指定监视器实例的速率.
 * @param monitor: 指向要更新的监视器实例的指针 (TX 或 RX).
 * @param packet_len: 刚刚发生事件的包长度.
 */
static void _rate_monitor_update(rate_monitor_instance_t* monitor, uint16_t packet_len)
{
    uint32_t current_tick = HAL_GetTick();

    // --- 1. 计算瞬时速率 (Instantaneous Rate) ---
    // (基于当前包与上一个包的时间间隔)
    float current_rate_bps = 0.0f;
    if (monitor->last_packet_tick > 0)
    {
        uint32_t time_elapsed_ms = current_tick - monitor->last_packet_tick;
        if (time_elapsed_ms > 0)
        {
            // (字节 * 8 bits/byte * 1000 ms/s) / (经过的毫秒)
            current_rate_bps = (float)packet_len * 8.0f * 1000.0f / (float)time_elapsed_ms;
        }
    }

    // 累加总字节数
    monitor->grand_total_bytes += packet_len;

    // --- 2. 存入历史记录 (用于平滑平均值) ---
    // 保存当前包的长度和时间戳
    monitor->len_history[monitor->history_index] = packet_len;
    monitor->tick_history[monitor->history_index] = current_tick;
    
    // 移动到下一个索引, 循环使用
    monitor->history_index = (monitor->history_index + 1) % RATE_AVG_COUNT;
    
    // 增加历史计数, 直到缓冲区满
    if (monitor->history_count < RATE_AVG_COUNT)
    {
        monitor->history_count++;
    }

    // --- 3. 计算平滑平均速率 (Stable Average Rate) ---
    // (基于历史窗口内的总字节数 / 总时间)
    float average_rate_bps = 0.0f;
    if (monitor->history_count > 1) // 至少需要两个点才能计算时间差
    {
        // 找到最旧的那个数据点
        uint8_t oldest_index = 0;
        if (monitor->history_count == RATE_AVG_COUNT)
        {
            // 缓冲区已满, "最旧"的就是 "当前" 索引指向的位置
            oldest_index = monitor->history_index;
        }
        // (如果缓冲区未满, "最旧"的就是索引 0)
        
        uint32_t oldest_tick = monitor->tick_history[oldest_index];
        uint32_t total_time_ms = current_tick - oldest_tick;

        // 计算历史窗口内的总字节数
        uint32_t total_bytes_in_window = 0;
        for (uint8_t i = 0; i < monitor->history_count; i++)
        {
             // (注意: 我们不能只加 packet_len, 因为在缓冲区满时, 
             // oldest_index 的包长度可能与当前的 packet_len 不同.
             // 所以我们必须遍历历史记录.)
            total_bytes_in_window += monitor->len_history[i];
        }

        // 计算速率
        if (total_time_ms > 0)
        {
            // (总字节 * 8 bits/byte * 1000 ms/s) / (总毫秒)
            average_rate_bps = (float)total_bytes_in_window * 8.0f * 1000.0f / (float)total_time_ms;
        }
    }
    else
    {
        // 历史数据不足, 暂时使用瞬时速率
        average_rate_bps = current_rate_bps;
    }

    // --- 4. 打印报告 ---
    char buffer[128];
    // 使用 monitor->stream_name 区分是 TX 还是 RX
    sprintf(buffer, "[%s Rate] Inst: %.2f bps | Avg: %.2f bps | Pkt: %u B | Total: %llu B\r\n",
            monitor->stream_name,
            current_rate_bps,
            average_rate_bps,
            packet_len,
            monitor->grand_total_bytes);

    // 发送到调试串口 (hlpuart1)
    Uart_Printf(&hlpuart1, "%s", buffer);

    // 更新"上一个包"的时间戳, 用于下次计算瞬时速率
    monitor->last_packet_tick = current_tick;
}


// --- 公共 API 函数 ---

/**
 * @brief ?????????.
 */
void rate_monitor_init(void)
{
    _monitor_instance_init(&s_rx_monitor, "RX"); // 初始化接收监视器
    _monitor_instance_init(&s_tx_monitor, "TX"); // 初始化发送监视器
    
    // 打印初始化信息
    Uart_Printf(&hlpuart1, "Rate Monitors (TX/RX) Initialized (Avg over %d packets).\r\n", RATE_AVG_COUNT);
}

/**
 * @brief ???????????,???????? RX ??????.
 * @param packet_len: ??????????(??).
 */
void rate_monitor_process_rx_packet(uint16_t packet_len)
{
    if (packet_len > 0) 
    {
        _rate_monitor_update(&s_rx_monitor, packet_len);
    }
}

/**
 * @brief ???????????,???????? TX ??????.
 * @param packet_len: ??????????(??).
 */
void rate_monitor_process_tx_packet(uint16_t packet_len)
{
    if (packet_len > 0)
    {
        _rate_monitor_update(&s_tx_monitor, packet_len);
    }
}
