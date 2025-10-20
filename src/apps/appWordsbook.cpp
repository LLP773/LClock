#include "AppManager.h"
#include <LittleFS.h>

static const uint8_t dcb_bits[] = {
    0x00, 0x00, 00, 0x00, 0x80, 0x03, 0x00, 0x00, 0x80, 0x03, 0x00, 0x00, 0x80, 0x03, 0x00, 0x00,
    0x9c, 0xf3, 0xcf, 0x39, 0x9e, 0xf3, 0xcf, 0x79, 0xbe, 0xfb, 0xdf, 0x7d, 0x3e, 0xf8, 0x1f, 0x7c,
    0x7e, 0xfc, 0x3f, 0x7e, 0xfe, 0xff, 0xff, 0x7f, 0x0e, 0x00, 0x00, 0x70, 0x06, 0x00, 0x00, 0x60,
    0x06, 0x00, 0x00, 0x60, 0x06, 0x00, 0x00, 0x60, 0x06, 0x00, 0x00, 0x60, 0x06, 0x00, 0x00, 0x60,
    0x06, 0x0c, 0xf8, 0x60, 0x06, 0x1e, 0xfc, 0x61, 0x06, 0x1e, 0x04, 0x63, 0x06, 0x12, 0x04, 0x63,
    0x06, 0x33, 0x84, 0x61, 0x06, 0x33, 0xfc, 0x60, 0x06, 0x3f, 0x84, 0x61, 0x86, 0x7f, 0x04, 0x63,
    0x86, 0x61, 0x04, 0x63, 0xc6, 0xe1, 0xfc, 0x61, 0xc6, 0xc0, 0xfc, 0x60, 0x06, 0x00, 0x00, 0x60,
    0x06, 0x00, 0x00, 0x60, 0x0e, 0x00, 0x00, 0x70, 0xfe, 0xff, 0xff, 0x7f, 0xfc, 0xff, 0xff, 0x3f, 
    0x00, 0x00, 0x00, 0x00};

class appWordbook : public AppBase
{
private:
    /* data */
public:
    appWordbook()
    {
        name = "Wordbook";
        title = "单词本";
        description = "单词本，词库等待完善";
        image = dcb_bits;
        _showInList = true;
    }
    void setup();
    void RefreshDisplay();
    void loop();
};

static appWordbook app;

int PRcount = 1;
int STB = 0;

int16_t PRcount_Random = 0;
int16_t PRcount_Max = 0;
bool Random_Mode = 0;
int8_t Words_Mode = 0;
String USEEn;
String USEYB;
String USECn;

const menu_item items[] = {
    {NULL, "返回"},
    {NULL, "随机模式"},
    {NULL, "跳转到.."},
    {NULL, "更换单词本.."},
    {NULL, "退出"},
    {NULL, NULL},
};

// 从文件读取单词数据而不是硬编码在程序中
String readWordFromFile(int index) {
    File file = LittleFS.open("/word.txt", "r");
    if (!file) {
        return "Error: Can't open word.txt";
    }
    
    String line;
    int currentLine = 0;
    
    while (file.available() && currentLine <= index) {
        line = file.readStringUntil('\n');
        if (currentLine == index) {
            file.close();
            line.trim();
            return line;
        }
        currentLine++;
    }
    
    file.close();
    return "Error: Word not found";
}

int getTotalWords() {
    File file = LittleFS.open("/word.txt", "r");
    if (!file) {
        return 0;
    }
    
    int count = 0;
    while (file.available()) {
        file.readStringUntil('\n');
        count++;
    }
    
    file.close();
    return count;
}

void appWordbook::setup()
{
    display.fillScreen(GxEPD_WHITE);
    u8g2Fonts.setFont(u8g2_font_wqy12_t_gb2312);
    u8g2Fonts.setForegroundColor(GxEPD_BLACK);
    u8g2Fonts.setBackgroundColor(GxEPD_WHITE);

    // 获取单词总数
    PRcount_Max = getTotalWords();
    if (PRcount_Max <= 0) {
        u8g2Fonts.setCursor(0, 30);
        u8g2Fonts.print("未找到单词文件或文件为空");
        display.display();
        return;
    }

    // 确保PRcount在有效范围内
    if (PRcount > PRcount_Max) {
        PRcount = 1;
    }

    // 读取当前单词
    String wordData = readWordFromFile(PRcount - 1);
    if (wordData.startsWith("Error:")) {
        u8g2Fonts.setCursor(0, 30);
        u8g2Fonts.print(wordData);
        display.display();
        return;
    }

    // 解析单词数据，格式为 "单词-/音标/-释义"
    // 首先去掉首尾的引号
    if (wordData.startsWith("\"") && wordData.endsWith("\"")) {
        wordData = wordData.substring(1, wordData.length() - 1);
    }
    
    // 查找第一个连字符和斜杠的位置
    int firstDash = wordData.indexOf('-');
    if (firstDash > 0) {
        USEEn = wordData.substring(0, firstDash);
        
        // 查找音标部分（在第一个/和第二个/之间）
        int firstSlash = wordData.indexOf('/', firstDash);
        int secondSlash = wordData.indexOf('/', firstSlash + 1);
        
        if (firstSlash > 0 && secondSlash > 0) {
            USEYB = wordData.substring(firstDash + 1, firstSlash);
            USECn = wordData.substring(secondSlash + 1);
        } else {
            // 如果解析失败，使用默认值
            USEYB = "无法解析音标";
            USECn = "无法解析释义";
        }
    } else {
        // 如果解析失败，使用整行作为英文单词
        USEEn = wordData;
        USEYB = "";
        USECn = "无法解析单词格式";
    }

    // 显示单词信息
    u8g2Fonts.setCursor(0, 20);
    u8g2Fonts.print("单词: ");
    u8g2Fonts.print(USEEn);
    
    u8g2Fonts.setCursor(0, 40);
    u8g2Fonts.print("音标: ");
    u8g2Fonts.print(USEYB);
    
    u8g2Fonts.setCursor(0, 60);
    u8g2Fonts.print("释义: ");
    u8g2Fonts.print(USECn);
    
    u8g2Fonts.setCursor(0, 80);
    u8g2Fonts.printf("第 %d/%d 个", PRcount, PRcount_Max);
    
    display.display();
}

void appWordbook::RefreshDisplay()
{
    // 更新显示逻辑保持不变
    setup();
}

void appWordbook::loop()
{
    // 添加循环逻辑
    if (hal.btnl.isPressing()) {
        if (PRcount > 1) {
            PRcount--;
            RefreshDisplay();
        }
        // 等待按钮释放
        while (hal.btnl.isPressing()) {
            delay(10);
        }
    }
    else if (hal.btnr.isPressing()) {
        if (PRcount < PRcount_Max) {
            PRcount++;
            RefreshDisplay();
        }
        // 等待按钮释放
        while (hal.btnr.isPressing()) {
            delay(10);
        }
    }
    else if (hal.btnc.isPressing()) {
        appManager.goBack();
        // 等待按钮释放
        while (hal.btnc.isPressing()) {
            delay(10);
        }
    }
}