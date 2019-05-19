import pandas as pd
import numpy as np
import cv2
import os


path = './tiles/'
i_max = 5.0
rgb_green = np.array([0, 128, 0])

for file in os.listdir(path):
	file_path = os.path.join(path, file)
	tile = pd.read_csv(file_path, sep=';')
	tile = tile.fillna(i_max)

	values = tile.values

	image = np.arange(256 * 256 * 4).reshape(256, 256, 4)
	for i in range(256):
		for j in range(256):
			alpha = int(255 * values[i * 256 + j][2] / i_max)
			image[i][j] = np.array([0, 128, 0, alpha])
	cv2.imwrite(os.path.join('/home/max/Загрузки/Telegram Desktop/HeatmapsFrontRelease/Data/Tiles/yandex_tiles', file.split('.')[0] + '.png'), image)
