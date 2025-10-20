#include "AppManager.h"
#include <stdlib.h> 
#include <time.h>

static const uint8_t fortune_bits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x1f, 0x00, 0x00, 0xfe, 0x7f, 0x00,
    0x80, 0x1f, 0xf8, 0x01, 0xc0, 0x03, 0xe0, 0x03, 0xe0, 0x01, 0x80, 0x07,
    0xf0, 0x00, 0x00, 0x0f, 0x78, 0x80, 0x01, 0x1e, 0x38, 0x80, 0x01, 0x1c,
    0x1c, 0x80, 0x01, 0x38, 0x1c, 0x80, 0x01, 0x30, 0x0e, 0x80, 0x01, 0x70,
    0x0e, 0x80, 0x01, 0x70, 0x06, 0x80, 0x01, 0x60, 0x06, 0x80, 0x01, 0x60,
    0x06, 0x80, 0x01, 0x60, 0x06, 0x80, 0x03, 0x60, 0x06, 0x80, 0x07, 0x60,
    0x06, 0x00, 0x0e, 0x60, 0x0e, 0x00, 0x1c, 0x70, 0x0e, 0x00, 0x38, 0x30,
    0x1c, 0x00, 0x10, 0x38, 0x1c, 0x00, 0x00, 0x38, 0x38, 0x00, 0x00, 0x1c,
    0x78, 0x00, 0x00, 0x1e, 0xf0, 0x00, 0x00, 0x0f, 0xe0, 0x01, 0x80, 0x07,
    0xc0, 0x07, 0xe0, 0x03, 0x80, 0x1f, 0xf8, 0x01, 0x00, 0xfe, 0x7f, 0x00,
    0x00, 0xf8, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00};

// 使用RTC数据保持状态，这样即使在deep sleep中也能保持
static RTC_DATA_ATTR int fortuneValue = -1;
static RTC_DATA_ATTR int refreshCount = 0;
static RTC_DATA_ATTR unsigned long lastRefreshTime = 0;

class AppFortune : public AppBase
{
private:
    void updateDisplay();
    
public:
    AppFortune()
    {
        name = "fortune";
        title = "今日人品";
        description = "查看今日人品值";
        image = fortune_bits;
        noDefaultEvent = false;
    }
    void set();
    void setup();
};

static AppFortune app;

void AppFortune::set(){
    _showInList = hal.pref.getBool(hal.get_char_sha_key(title), true);
}

void AppFortune::setup()
{
    // 如果是第一次运行，生成初始人品值
    if (fortuneValue == -1) {
        srand(esp_random());
        fortuneValue = rand() % 101;
        refreshCount = 0;
        lastRefreshTime = millis();
    }
    
    updateDisplay();
    
    // 按钮事件处理循环
    while (true) {
        // 处理右按钮按下事件
        if (hal.btnr.isPressing()) {
            // 防止过快刷新（1秒内不能重复刷新）
            if (millis() - lastRefreshTime > 1000) {
                refreshCount++;
                lastRefreshTime = millis();
                
                // 根据刷新次数执行不同操作
                if (refreshCount < 5) {
                    // 前5次正常刷新
                    fortuneValue = rand() % 101;
                } else if (refreshCount == 10) {
                    // 第10次显示问号
                    // 人品值不变，只改变显示文本
                } else if (refreshCount == 13) {
                    // 第13次显示3个问号
                    // 人品值不变，只改变显示文本
                } else if (refreshCount == 15) {
                    // 第15次将人品值设为0
                    fortuneValue = 0;
                } else if (refreshCount >= 18) {
                    // 18次及以后显示特殊文本
                    // 人品值不变，只改变显示文本
                } else if (refreshCount >= 5 && refreshCount < 10) {
                    // 5-9次不改变人品值
                    // 人品值不变，只改变显示文本
                }
                
                // 更新显示
                if (refreshCount <= 20) {
                    updateDisplay();
                }
            }
            
            // 等待按钮释放
            while (hal.btnr.isPressing()) {
                delay(10);
            }
        }
        
        // 处理返回按钮（左键）
        if (hal.btnl.isPressing()) {
            appManager.goBack();
            return;
        }
        
        delay(10);
    }
}

void AppFortune::updateDisplay()
{
    display.clearScreen();
    u8g2Fonts.setFontMode(1);
    u8g2Fonts.setForegroundColor(0);
    u8g2Fonts.setBackgroundColor(1);
    
    // 显示标题
    u8g2Fonts.setFont(u8g2_font_wqy12_t_gb2312);
    int titleWidth = u8g2Fonts.getUTF8Width("今日人品");
    u8g2Fonts.setCursor((296 - titleWidth) / 2, 20);
    u8g2Fonts.print("今日人品");
    
    // 显示人品值
    u8g2Fonts.setFont(u8g2_font_logisoso46_tn);
    char fortuneStr[16];
    
    // 根据刷新次数显示不同内容
    if (refreshCount == 10) {
        sprintf(fortuneStr, "?");
    } else if (refreshCount == 13) {
        sprintf(fortuneStr, "???");
    } else {
        sprintf(fortuneStr, "%d", fortuneValue);
    }
    
    int valueWidth = u8g2Fonts.getUTF8Width(fortuneStr);
    u8g2Fonts.setCursor((296 - valueWidth) / 2, 76);
    u8g2Fonts.print(fortuneStr);
    
    // 显示提示文字
    u8g2Fonts.setFont(u8g2_font_wqy12_t_gb2312);
    display.drawFastHLine(0, 86, 296, 0);
    u8g2Fonts.setCursor(10, 100);
    
    // 根据刷新次数显示不同信息
    if (refreshCount == 0) {
        u8g2Fonts.print("今日运势指数 (R键刷新 L键返回)");
    } else if (refreshCount < 5) {
        u8g2Fonts.printf("你都刷了%d次了", refreshCount, 5-refreshCount);
    } else if (refreshCount < 8) {
        u8g2Fonts.print("不准刷了！>_<");
    } else if (refreshCount < 10) {
        u8g2Fonts.print("生活不如意了？");
    } else if (refreshCount == 10) {
        u8g2Fonts.print("?");
    } else if (refreshCount < 13) {
        u8g2Fonts.print("你到底在期待什么？");
    } else if (refreshCount == 13) {
        u8g2Fonts.print("???");
    } else if (refreshCount < 15) {
        u8g2Fonts.print("看在你的愿望这么强烈，再多刷一两次我给你个机会");
    } else if (refreshCount == 15) {
        u8g2Fonts.print("现在开心了吧？");
    } else {
        u8g2Fonts.print("像你这么可恶的人");
    }
    
    // 根据人品值显示不同描述
    u8g2Fonts.setCursor(10, 115);
    
    // 特殊刷新次数显示特殊文本
    if (refreshCount > 15) {
        u8g2Fonts.print("一切都是你咎由自取");
    } else if (refreshCount == 14) {
        u8g2Fonts.print("再多刷一两次我给你个机会");
    } else if (refreshCount == 10) {
        u8g2Fonts.print("别刷了！");
    } else {
        // 正常根据人品值显示描述
        if (fortuneValue < 10) {
            u8g2Fonts.print("好好反思反思你小子是不是干坏事了");
        } else if (fortuneValue < 20) {
            u8g2Fonts.print("运气不佳，你今天最好小心点");
        } else if (fortuneValue < 40) {
            u8g2Fonts.print("运势平平，保持平常心");
        } else if (fortuneValue < 60) {
            u8g2Fonts.print("运势尚可");
        } else if (fortuneValue < 80) {
            u8g2Fonts.print("运气不错");
        } else if (fortuneValue < 95) {
            u8g2Fonts.print("你这运气……一般");
        } else {
            u8g2Fonts.print("你早上一定是扶老奶奶过马路了");
        }
    }
    
    // 电池图标
    display.drawXBitmap(296 - 25, 87, getBatteryIcon(), 20, 16, 0);

    if (force_full_update || part_refresh_count > 20)
    {
        display.display(false);
        force_full_update = false;
        part_refresh_count = 0;
    }
    else
    {
        display.display(true);
        part_refresh_count++;
    }
    
    appManager.noDeepSleep = false;
    appManager.nextWakeup = 0; // 不需要定期更新
}