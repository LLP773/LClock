#include "AppManager.h"
#include <stdlib.h>
#include <time.h>
#include <LittleFS.h>

// 笑话应用图标 (32x32)
static const uint8_t joke_icon[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

class AppJoke : public AppBase
{
private:
    int current_joke = 0;
    bool is_random_mode = false;
    int last_random_joke = -1; // 记录上一个随机笑话的索引
    int jokes_count = 0;       // 笑话总数

public:
    AppJoke()
    {
        name = "joke";
        title = "笑一个";
        description = "来，笑一个";
        image = joke_icon;
        // 初始化笑话总数
        jokes_count = countJokes();
    }
    
    void setup();
    
    // 获取不重复的随机笑话索引
    int getRandomJokeIndex() {
        if (jokes_count <= 1) {
            return 0;
        }
        
        int new_index;
        do {
            new_index = rand() % jokes_count;
        } while (new_index == last_random_joke && jokes_count > 1);
        
        last_random_joke = new_index;
        return new_index;
    }
    
    // 从文件中读取指定索引的笑话
    String getJokeFromFile(int index) {
        File file = LittleFS.open("/Jokes.txt", "r");
        if (!file) {
            return "错误：无法打开笑话文件";
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
        return "错误：未找到指定笑话";
    }
    
    // 计算文件中笑话的总数
    int countJokes() {
        File file = LittleFS.open("/jokes.txt", "r");
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
};

static AppJoke app;

void AppJoke::setup()
{
    // 检查是否有笑话文件
    if (jokes_count <= 0) {
        GUI::drawWindowsWithTitle("错误");
        u8g2Fonts.setFont(u8g2_font_wqy12_t_gb2312);
        u8g2Fonts.setCursor(2, 30);
        u8g2Fonts.print("未找到笑话文件");
        display.display(false);
        delay(2000);
        appManager.goBack();
        return;
    }
    
    current_joke = 0;
    is_random_mode = false;
    last_random_joke = -1;
    
    // 初始化随机数种子
    srand(esp_random());
    
    while (1)
    {
        // 绘制窗口
        GUI::drawWindowsWithTitle("来，笑一个");
        
        // 显示笑话
        String joke = getJokeFromFile(current_joke);
        u8g2Fonts.setFont(u8g2_font_wqy12_t_gb2312);
        u8g2Fonts.setCursor(2, 30);
        GUI::autoIndentDraw(joke.c_str(), 292, 2, 12);
        
        // 显示页码和模式信息
        char page_info[32];
        sprintf(page_info, "%d/%d", current_joke + 1, jokes_count);
        u8g2Fonts.setCursor(250, 120);
        u8g2Fonts.print(page_info);
        
        display.display(false);
        
        // 等待用户操作
        while (1)
        {
            // 使用isPressing()方法检测按钮状态
            if (hal.btnl.isPressing())
            {
                // 上一个笑话
                if (is_random_mode) {
                    // 在随机模式下，按左键也随机选择
                    current_joke = getRandomJokeIndex();
                } else {
                    current_joke--;
                    if (current_joke < 0)
                        current_joke = jokes_count - 1;
                }
                break;
            }
            else if (hal.btnr.isPressing())
            {
                // 下一个笑话
                if (is_random_mode) {
                    // 在随机模式下，按右键也随机选择
                    current_joke = getRandomJokeIndex();
                } else {
                    current_joke++;
                    if (current_joke >= jokes_count)
                        current_joke = 0;
                }
                break;
            }
            else if (hal.btnc.isPressing())
            {
                // 显示设置菜单
                const menu_item options[] = {
                    {NULL, "随机模式"},
                    {NULL, "返回"},
                    {NULL, "退出应用"},
                    {NULL, NULL} // 结束标记
                };
                
                int choice = GUI::menu("设置", options, 12, 12);
                
                switch (choice) {
                    case 0: // 切换随机模式
                        is_random_mode = !is_random_mode;
                        if (is_random_mode) {
                            last_random_joke = current_joke; // 将当前笑话设为上一个随机笑话
                            current_joke = getRandomJokeIndex();
                        }
                        break;
                    case 1: // 返回
                        // 不执行任何操作，直接返回主界面
                        break;
                    case 2: // 退出应用
                        appManager.goBack();
                        return;
                    default: // 用户取消菜单
                        break;
                }
                break;
            }
            delay(100);
        }
    }
}