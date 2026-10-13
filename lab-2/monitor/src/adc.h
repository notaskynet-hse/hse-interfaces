#pragma once

// MCP3204: 12 бит, опорное напряжение 4,096 В - 1 мВ на единицу кода.
namespace Adc {
constexpr int kMaxCode = 4095;
constexpr double kVoltsPerCode = 4.096 / 4096.0;
} // namespace Adc
