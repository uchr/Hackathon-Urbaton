import pandas as pd
import numpy as np
import cv2
import os


path = './google_tiles_13/'

lower_green = np.array([45, 0, 0])
upper_green = np.array([65, 256, 256])

x_min = 5975
y_max = 2598

for file in os.listdir(path):
    file_path = os.path.join(path, file)
    if not os.path.isfile(file_path):
        continue
    image = cv2.imread(file_path)

    hsv = cv2.cvtColor(image, cv2.COLOR_BGR2HSV)
    size_x = image.shape[0]
    size_y = image.shape[1]
    chunk = 16
    green_image = np.arange(image.shape[0] * image.shape[1] * 4).reshape(image.shape[0], image.shape[1], 4)
    weights_array = {'x' : [], 'y': [], 'w': []}
    for x in range(0, size_x, chunk):
        for y in range(0, size_y, chunk):
            area = 0.0
            x_length = min(chunk, size_x - x)
            y_length = min(chunk, size_y - y)
            area = x_length * y_length
            green_count = 0
            for i in range(0, x_length):
                for j in range(0, y_length):
                    if (hsv[x + i][y + j] >= lower_green).all() and (hsv[x + i][y + j] <= upper_green).all():
                        green_count += 1
            alpha = float(green_count) / area
            for i in range(0, x_length):
                for j in range(0, y_length):
                    weights_array['x'].append(x + i)
                    weights_array['y'].append(y + j)
                    weights_array['w'].append(alpha)
                    green_image[x + i][y + j] = np.array([0, 128, 0, int(255 * alpha)] )

    cv2.imwrite(os.path.join(path, 'final_tiles/' + file), green_image)

    df = pd.DataFrame(weights_array)
    df.to_csv(os.path.join(os.path.join(path, 'final_csv/'), file.split('.')[0] + '.csv'), sep=';', index=False)
