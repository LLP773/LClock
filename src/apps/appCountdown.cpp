#include "AppManager.h"

static const uint8_t Countdown_icon[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0xff, 0xff, 0x0f, 0xe0, 0xff, 0xff, 0x07,
    0xc0, 0xff, 0xff, 0x03, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01,
    0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0xe1, 0x87, 0x01,
    0x80, 0xc3, 0xc3, 0x01, 0x00, 0x87, 0xe1, 0x00, 0x00, 0x8e, 0x71, 0x00, 0x00, 0x1c, 0x38, 0x00,
    0x00, 0x38, 0x1c, 0x00, 0x00, 0x1c, 0x38, 0x00, 0x00, 0x0e, 0x70, 0x00, 0x00, 0x87, 0xe1, 0x00,
    0x80, 0xc3, 0xc3, 0x01, 0x80, 0xe1, 0x87, 0x01, 0x80, 0xe1, 0x87, 0x01, 0x80, 0xf1, 0x8f, 0x01,
    0x80, 0xf9, 0x9f, 0x01, 0x80, 0xf9, 0x9f, 0x01, 0x80, 0x01, 0x80, 0x01, 0xc0, 0xff, 0xff, 0x03,
    0xe0, 0xff, 0xff, 0x07, 0xf0, 0xff, 0xff, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

class AppCountdown : public AppBase
{
private:
public:
    AppCountdown()
    {
        name = "Countdown";
        title = "放学计时";
        description = "放学还有多久-分钟";
        image = Countdown_icon;
        wakeupIO[1] = PIN_BUTTONR;

        _showInList = true;
    }
    void setup();
    void openMenu();
};
static AppCountdown app;
extern const char *dayOfWeek[];

void AppCountdown::setup()
{
    int w;
    int nowtime = 0;
    int worktimeup = 0;
    int worktimedn = 0;
    char timeStr[6];

    display.clearScreen();
    u8g2Fonts.setFontMode(1);
    u8g2Fonts.setForegroundColor(0);
    u8g2Fonts.setBackgroundColor(1);

    if (hal.btnr.isPressing())
    {
        Serial.println("右键按下");
        if (GUI::waitLongPress(PIN_BUTTONR))
        {
            Serial.println("长按右键");

            openMenu();
            display.display(true);
        }
    }
    nowtime = hal.timeinfo.tm_hour * 60 + hal.timeinfo.tm_min;
    worktimeup = hal.pref.getInt("workup", 480);
    worktimedn = hal.pref.getInt("workdn", 1020);
    if (nowtime < worktimeup)
    {
        u8g2Fonts.setFont(u8g2_font_wqy16_t_gb2312);
        u8g2Fonts.setCursor(5, 20);
        u8g2Fonts.print("我不知道你这么勤奋呢");
        u8g2Fonts.setFont(u8g2_font_logisoso58_tn);
        sprintf(timeStr, "%02d:%02d", hal.timeinfo.tm_hour, hal.timeinfo.tm_min);

        w = u8g2Fonts.getUTF8Width(timeStr);
        u8g2Fonts.setCursor((296 - w) / 2, 100);
        u8g2Fonts.print(timeStr);
    }
    else if (nowtime >= worktimeup && nowtime < worktimedn)
    {
        u8g2Fonts.setFont(u8g2_font_wqy16_t_gb2312);
        w = u8g2Fonts.getUTF8Width("Ciallo~，放学还有");
        u8g2Fonts.setCursor((296 - w) / 2, 20);
        u8g2Fonts.print("Ciallo~，放学还有");
        u8g2Fonts.setFont(u8g2_font_logisoso58_tn);
        sprintf(timeStr, "%d", 540 - ((hal.timeinfo.tm_hour - 8) * 60 + hal.timeinfo.tm_min));
        w = u8g2Fonts.getUTF8Width(timeStr);
        u8g2Fonts.setCursor((296 - w) / 2, 100);
        u8g2Fonts.print(timeStr);
        u8g2Fonts.setFont(u8g2_font_wqy16_t_gb2312);
        u8g2Fonts.print("分钟");
    }
    else if (nowtime > worktimedn)
    {
        u8g2Fonts.setFont(u8g2_font_wqy16_t_gb2312);
        u8g2Fonts.setCursor(5, 20);
        u8g2Fonts.print("我觉得你在学校一定得有急事");
        u8g2Fonts.setFont(u8g2_font_logisoso58_tn);
        sprintf(timeStr, "%02d:%02d", hal.timeinfo.tm_hour, hal.timeinfo.tm_min);

        w = u8g2Fonts.getUTF8Width(timeStr);
        u8g2Fonts.setCursor((296 - w) / 2, 100);
        u8g2Fonts.print(timeStr);
    }

    u8g2Fonts.setFont(u8g2_font_wqy12_t_gb2312);
    display.drawFastHLine(0, 108, 296, 0);
    u8g2Fonts.setCursor(5, 125);
    u8g2Fonts.printf("%02d月%02d日 星期%s", hal.timeinfo.tm_mon + 1, hal.timeinfo.tm_mday, dayOfWeek[hal.timeinfo.tm_wday]);

    if (peripherals.peripherals_current & PERIPHERALS_AHT20_BIT)
    {
        sensors_event_t humidity, temp;
        peripherals.load_append(PERIPHERALS_AHT20_BIT);
        xSemaphoreTake(peripherals.i2cMutex, portMAX_DELAY);
        peripherals.aht.getEvent(&humidity, &temp);
        xSemaphoreGive(peripherals.i2cMutex);
        u8g2Fonts.printf("Temp:%.1f℃ Humi:%.1f%%", temp.temperature, humidity.relative_humidity);
    }

    display.drawXBitmap(296 - 25 - 9, 115, getBatteryIcon(), 8, 12, 0);
    u8g2Fonts.setCursor(296 - 25, 125);
    if (hal.USBPluggedIn)
    {
        display.drawXBitmap(296 - 25 + 2, 116, getUSBIcon(), 22, 10, 0);
    }
    else
    {
        u8g2Fonts.printf("%d%%", getBatteryNum());
    }

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
    appManager.nextWakeup = 61 - hal.timeinfo.tm_sec;
}

void AppCountdown::openMenu()
{

    const menu_item items[] = {
        {NULL, "返回"},
        {NULL, "设置上学时间"},
        {NULL, "设置放学时间"},
        {NULL, NULL},
    };
    int ret = GUI::menu("计时设置", items);

    switch (ret)
    {
    case 0:
        break;
    case 1:
        hal.pref.putInt("workup", GUI::msgbox_time("请输入上学时间", hal.pref.getInt("workup", 480)));

        break;
    case 2:
        hal.pref.putInt("workdn", GUI::msgbox_time("请输入放学时间", hal.pref.getInt("workdn", 1020)));

        break;
    default:
        break;
    }
}
