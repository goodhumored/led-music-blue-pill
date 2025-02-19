import serial
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from collections import deque

# Настройки порта
PORT = '/dev/ttyUSB1'
BAUDRATE = 115200  # Должно совпадать с настройкой на STM32
NUM_BINS = 127

# Инициализация данных
data_buffer = deque([0]*NUM_BINS, maxlen=NUM_BINS)

# Настройка графика
fig, ax = plt.subplots()
line, = ax.plot(range(NUM_BINS), data_buffer)
ax.set_ylim(0, 1000)  # Подберите под ваш диапазон значений
ax.set_xlabel('Bin')
ax.set_ylabel('Amplitude')
plt.title('Real-time FFT Spectrum')

# Инициализация последовательного порта
ser = serial.Serial(PORT, BAUDRATE, timeout=0.1)
ser.reset_input_buffer()

def update(frame):
    global data_buffer
    
    while ser.in_waiting > 0:
        try:
            # Чтение строки данных
            line_data = ser.readline().decode('utf-8').strip()
            
            if line_data:
                # Преобразование строки в список чисел
                new_data = list(map(int, line_data.split()))
                
                # Проверка количества бинов
                if len(new_data) == NUM_BINS:
                    data_buffer = new_data
                    line.set_ydata(data_buffer)
                    
        except (UnicodeDecodeError, ValueError) as e:
            print(f"Error processing data: {e}")
            continue
            
    return line,

# Анимация для обновления графика
ani = animation.FuncAnimation(fig, update, interval=50, blit=True)

plt.show()
