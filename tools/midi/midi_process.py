import mido
import os
import struct
import glob

# MIDI 音符到频率的映射表
midi_to_freq = [
    16, 17, 18, 19, 20, 21, 23, 24, 25, 27, 29, 30, 32, 34, 36, 38, 41, 43, 46, 49,
    51, 55, 58, 61, 65, 69, 73, 77, 82, 87, 92, 98, 103, 110, 116, 123, 130, 138,
    146, 155, 164, 174, 185, 196, 207, 220, 233, 246, 261, 277, 293, 311, 329, 349,
    369, 392, 415, 440, 466, 493, 523, 554, 587, 622, 659, 698, 739, 783, 830, 880,
    932, 987, 1046, 1108, 1174, 1244, 1318, 1396, 1479, 1567, 1661, 1760, 1864, 1975,
    2093, 2217, 2349, 2489, 2637, 2793, 2959, 3135, 3322, 3520, 3729, 3951, 4186,
    4434, 4698, 4978, 5274, 5587, 5919, 6271, 6644, 7040, 7458, 7902, 8372, 8869,
    9397, 9956, 10548, 11175, 11839, 12543, 13289, 14080, 14917, 15804, 16744, 17739,
    18794, 19912, 21096, 22350, 23679, 25087
]

# 定义输入和输出目录
INPUT_DIR = os.path.join(os.path.dirname(__file__), "midi_files")
OUTPUT_DIR = os.path.join(os.path.dirname(__file__), "buz_files")

def create_directories():
    """创建必要的目录"""
    if not os.path.exists(INPUT_DIR):
        os.makedirs(INPUT_DIR)
        print(f"已创建输入目录: {INPUT_DIR}")
        print("请将 MIDI 文件放入此目录后再运行脚本")
    
    if not os.path.exists(OUTPUT_DIR):
        os.makedirs(OUTPUT_DIR)
        print(f"已创建输出目录: {OUTPUT_DIR}")

def get_midi_files():
    """获取所有 MIDI 文件"""
    midi_patterns = ["*.mid", "*.midi"]
    midi_files = []
    
    for pattern in midi_patterns:
        midi_files.extend(glob.glob(os.path.join(INPUT_DIR, pattern)))
    
    return midi_files

def get_processing_mode():
    """获取处理模式"""
    while True:
        mode = input("请选择处理模式（1:最高音优先 / 2:最接近上个音符优先，默认为1）: ").strip()
        if mode == "":
            return "1"
        elif mode in ["1", "2"]:
            return mode
        else:
            print("无效的模式，请输入 1 或 2！")

def get_time_threshold():
    """获取时间阈值"""
    while True:
        try:
            threshold = input("请输入时间阈值（秒，默认 0.08）: ").strip()
            if threshold == "":
                return 0.08
            threshold = float(threshold)
            if threshold > 0:
                return threshold
            else:
                print("时间阈值必须大于 0！")
        except ValueError:
            print("请输入有效的数字！")

def process_track(track, track_id, mode, time_threshold, ticks_per_beat):
    """
    处理单个音轨
    :param track: MIDI 音轨
    :param track_id: 音轨 ID
    :param mode: 处理模式
    :param time_threshold: 时间阈值
    :param ticks_per_beat: MIDI 文件的 ticks_per_beat 参数
    :return: 处理后的音乐数据
    """
    tempo = 500000  # 默认节拍
    current_pressing = 0  # 当前按下的音符
    current_second = 0    # 当前时间
    last_pressing = -1    # 上一个按下的音符
    music = []            # 音乐数据
    
    for msg in track:
        # 将消息时间从 ticks 转换为秒
        if msg.time > 0:
            delta = mido.tick2second(msg.time, ticks_per_beat, tempo)
        else:
            delta = 0
        
        current_second += delta
        
        if msg.type == 'set_tempo':
            tempo = msg.tempo
        elif msg.type == 'note_on':
            if current_second == 0:
                if mode == "2":
                    # 模式2：选择最接近上一个音符的音符
                    if last_pressing != -1:
                        if abs(current_pressing - last_pressing) < abs(midi_to_freq[msg.note] - last_pressing):
                            continue
                else:
                    # 模式1：选择最高音符
                    if current_pressing > midi_to_freq[msg.note]:
                        continue
            
            if current_second >= time_threshold:
                # 上个音符已完成，添加到音乐数据中
                music.append({'freq': current_pressing, 'time': round(current_second * 1000)})
                current_second = 0
                last_pressing = current_pressing
            
            current_pressing = midi_to_freq[msg.note]
            
        elif msg.type == 'note_off':
            if msg.note == current_pressing:
                current_pressing = 0
    
    return music

def save_music_data(music, filename, track_id):
    """
    保存音乐数据到文件
    :param music: 音乐数据
    :param filename: 原始文件名
    :param track_id: 音轨 ID
    """
    if len(music) == 0:
        return False
    
    # 生成输出文件名
    base_name = os.path.splitext(os.path.basename(filename))[0]
    output_filename = f"{base_name}_{track_id}.buz"
    output_path = os.path.join(OUTPUT_DIR, output_filename)
    
    # 文件格式：小端序，2字节频率 + 2字节时间
    with open(output_path, 'wb') as f:
        for i in range(len(music)):
            # 跳过开始的休止符
            if i == 0 and music[i]['freq'] == 0:
                continue
            # 写入频率（2字节）
            f.write(struct.pack('<H', music[i]['freq']))
            # 写入时间（2字节，最大30000毫秒）
            f.write(struct.pack('<H', min(music[i]['time'], 30000)))
    
    print(f"  已保存: {output_filename} ({len(music)} 个音符)")
    return True

def process_midi_file(filepath, mode, time_threshold):
    """
    处理单个 MIDI 文件
    :param filepath: MIDI 文件路径
    :param mode: 处理模式
    :param time_threshold: 时间阈值
    """
    try:
        print(f"正在处理: {os.path.basename(filepath)}")
        midi_file = mido.MidiFile(filepath)
        print(f"  音轨数: {len(midi_file.tracks)}, ticks/beat: {midi_file.ticks_per_beat}")
        
        success_count = 0
        for i, track in enumerate(midi_file.tracks):
            music = process_track(track, i, mode, time_threshold, midi_file.ticks_per_beat)
            if save_music_data(music, filepath, i):
                success_count += 1
        
        print(f"  完成处理: {success_count}/{len(midi_file.tracks)} 个音轨成功转换")
        return True
        
    except Exception as e:
        print(f"  处理文件时出错: {e}")
        return False

def main():
    """主函数"""
    print("MIDI 批量处理工具")
    print("=" * 30)
    
    # 创建必要的目录
    create_directories()
    
    # 获取所有 MIDI 文件
    midi_files = get_midi_files()
    
    if not midi_files:
        print(f"在目录 '{INPUT_DIR}' 中未找到 MIDI 文件")
        print("请将 .mid 或 .midi 文件放入该目录后再运行脚本")
        return
    
    print(f"找到 {len(midi_files)} 个 MIDI 文件")
    
    # 获取处理参数
    mode = get_processing_mode()
    time_threshold = get_time_threshold()
    
    # 处理所有 MIDI 文件
    success_count = 0
    for midi_file in midi_files:
        if process_midi_file(midi_file, mode, time_threshold):
            success_count += 1
    
    print("\n" + "=" * 30)
    print(f"处理完成: {success_count}/{len(midi_files)} 个文件成功转换")
    print(f"输出文件保存在: {OUTPUT_DIR}")

if __name__ == "__main__":
    main()