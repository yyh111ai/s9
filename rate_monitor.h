#ifndef RATE_MONITOR_H
#define RATE_MONITOR_H

#include <stdint.h>
#include "main.h"

/**
 * @brief 初始化 TX 和 RX 两个速率监视器.
 */
void rate_monitor_init(void);

/**
 * @brief 处理一个已接收数据包, 更新 RX 速率统计.
 * @param packet_len: 刚刚接收到的数据包字节数.
 */
void rate_monitor_process_rx_packet(uint16_t packet_len);

/**
 * @brief 处理一个已发送数据包, 更新 TX 速率统计.
 * @param packet_len: 刚刚发送的数据包字节数.
 */
void rate_monitor_process_tx_packet(uint16_t packet_len);


#endif // RATE_MONITOR_H
