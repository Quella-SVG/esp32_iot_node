# esp32_iot_node

基于 **ESP32-S3 + ESP-IDF** 的可扩展嵌入式物联网节点：采集传感器数据，通过 Wi-Fi 与 MQTT 接入 [OneNET](https://open.iot.10086.cn/) 云平台，上报遥测数据并响应平台下发命令。

## 功能特性

- **温湿度采集**：SHT30（I2C），周期上报 `Temp` / `RH` 两条数据流
- **Wi-Fi 管理**：上电自动连接，断线自动重连，连接状态回调驱动上层启动
- **网络时间同步**：SNTP，保证遥测数据时间戳可信
- **OneNET 接入**：MQTT 协议，token 由 accessKey 在设备端动态计算，开机重算不过期
- **平台命令**：支持云端下发 JSON 命令（如修改上报周期）
- **可扩展架构**：数据源注册表 + 组件化驱动，新传感器接入 = 新增一个组件目录 + 注册表加一行

## 系统架构

```
上电 → NVS / 事件循环初始化 → WiFi 连接
    → 连接回调：SNTP 同步时间 → OneNET MQTT 连接 → 启动遥测任务
```

```
esp32_iot_node/
├── main/                        # 入口：只做模块装配，不写业务逻辑
│   └── app_main.c
├── components/
│   ├── app_config/              # 全局配置：WiFi / OneNET / 引脚 / 采集周期
│   ├── drivers/
│   │   └── sht30/               # SHT30 温湿度驱动（I2C）
│   ├── platforms/
│   │   └── onenet/              # OneNET 平台接入（MQTT + token 计算）
│   ├── wifi_manager/            # Wi-Fi 连接 / 断线重连 / 状态回调
│   ├── net_time/                # SNTP 网络时间同步
│   └── app_telemetry/           # 业务采集：数据源注册表 + 命令处理
├── CMakeLists.txt
└── sdkconfig.defaults
```

## 硬件连接

| 外设 | 引脚 | 说明 |
|---|---|---|
| SHT30 SDA | GPIO 21 | I2C 数据 |
| SHT30 SCL | GPIO 8 | I2C 时钟 |
| SHT30 地址 | 0x44 | 默认地址 |

## 快速开始

环境要求：[ESP-IDF](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32s3/get-started/)（本项目基于 v5.x）。

```bash
idf.py set-target esp32s3
idf.py build
idf.py -p <串口> flash monitor    # Windows 下串口如 COM5
```

## 配置

所有参数集中在 [components/app_config/app_config.h](components/app_config/app_config.h)：

| 宏 | 说明 |
|---|---|
| `CFG_WIFI_SSID` / `CFG_WIFI_PASS` | Wi-Fi 名称与密码 |
| `CFG_ONENET_URI` | OneNET MQTT 服务器地址 |
| `CFG_ONENET_PRODUCT_ID` | OneNET 产品 ID |
| `CFG_ONENET_DEVICE_NAME` | 设备名称 |
| `CFG_ONENET_ACCESS_KEY` | 产品 accessKey |
| `CFG_DS_TEMP` / `CFG_DS_HUMI` | 数据流名，须与平台侧一致 |
| `CFG_SHT30_SDA` / `CFG_SHT30_SCL` / `CFG_SHT30_ADDR` | SHT30 引脚与地址 |
| `CFG_TELEMETRY_PERIOD_MS` | 默认上报周期（ms） |

## OneNET 平台侧配置

1. 在 OneNET 控制台创建产品（接入协议：MQTT），记下 **产品 ID** 与 **accessKey**
2. 创建设备，设备名与 `CFG_ONENET_DEVICE_NAME` 一致
3. 创建数据流 `Temp`、`RH`，名称须与宏定义完全一致
4. 将以上信息填入 `app_config.h`

## 云端命令

通过平台向设备下发 JSON 命令，当前支持：

```json
{"cmd":"set_period","value":10000}
```

将上报周期改为 10000 ms。

## 新增传感器指南

以光照传感器 BH1750 为例：

1. 在 `components/drivers/` 下新建 `bh1750/` 组件（实现 `xxx_init` / `xxx_read(float*)`）
2. 在 `app_config.h` 中添加数据流名，如 `CFG_DS_LUX "Lux"`
3. 在 [app_telemetry.c](components/app_telemetry/app_telemetry.c) 的数据源注册表 `g_sources[]` 中加一行：

```c
{ CFG_DS_LUX, bh1750_read },
```

业务代码无需任何改动，新数据流即自动周期上报。

## Roadmap

- [ ] 更多传感器驱动（BH1750 光照、MQ-2 烟雾等）
- [ ] 多云平台适配（`platforms/` 已组件化，可扩展阿里云 / 巴法云等）
- [ ] Wi-Fi 配网方案（SmartConfig / BluFi / AP 配网）
- [ ] 低功耗（Deep Sleep 周期唤醒上报）
- [ ] OTA 远程升级
