import serial
import time
import cv2
import os

# 設定串口連接（修改為你的 Arduino 串口名稱）
ser = serial.Serial('COM4', 9600, timeout=1)

# 設定相機
camera = cv2.VideoCapture(0)
if not camera.isOpened():
    print("無法啟動相機！請檢查相機是否已連接。")
    exit()

# 設定攝影機影像大小（這仍會影響拍攝原始影像）
camera_width = 1960
camera_height = 1280
camera.set(cv2.CAP_PROP_FRAME_WIDTH, camera_width)
camera.set(cv2.CAP_PROP_FRAME_HEIGHT, camera_height)

# 基本配置
base_folder = "captured_images"
os.makedirs(base_folder, exist_ok=True)

# 定義搖桿狀態函數
def read_joystick():
    try:
        # 讀取串口數據
        data = ser.readline().decode('utf-8').strip()
        print(f"Received data: {data}")  # 調試用
        if data:
            values = data.split(',')
            if len(values) == 4:
                return [int(value) for value in values]
        return []
    except Exception as e:
        print(f"Error reading joystick data: {e}")
        return []

# 主程式
try:
    print("開始拍攝並記錄搖桿數據，每秒儲存一張照片...按下 Ctrl+C 停止。")
    while True:
        joystick_values = read_joystick()

        if joystick_values:
            folder_name = "_".join(map(str, joystick_values))
            save_folder = os.path.join(base_folder, folder_name)
            os.makedirs(save_folder, exist_ok=True)

            ret, frame = camera.read()
            if not ret:
                print("無法讀取影像！")
                break

            # 裁剪影像中間部分
            height, width, _ = frame.shape
            crop_width = 720  # 要裁剪的寬度
            crop_height = 600  # 要裁剪的高度
            x_start = (width - crop_width) // 2
            y_start = (height - crop_height) // 2
            cropped_frame = frame[y_start:y_start + crop_height, x_start:x_start + crop_width]

            cv2.imshow("Captured Image", cropped_frame)

            timestamp = time.strftime("%Y%m%d_%H%M%S")
            filename = os.path.join(save_folder, f"image_{timestamp}.jpg")
            cv2.imwrite(filename, cropped_frame)
            print(f"裁剪後的照片已儲存至資料夾 '{folder_name}'：{filename}")
        else:
            print("無法讀取搖桿數據，跳過此次拍攝。")

        time.sleep(0.05)  # 與 Arduino 的發送頻率同步

        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

except KeyboardInterrupt:
    print("\n程式已停止。")
finally:
    camera.release()
    cv2.destroyAllWindows()
