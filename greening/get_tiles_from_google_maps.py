import numpy as np
import cv2
import os
import time
import requests
import shutil

def get_route_tile(x, y, out_file):
	#http://mt1.google.com/vt/lyrs=y&x=5975&y=2598&z=13
    url = 'http://mt1.google.com/vt/lyrs=y&x={}&y={}&z=13'.format(x, y)
    response = requests.get(url, stream=True)
    with open(out_file, 'wb') as file:
    	shutil.copyfileobj(response.raw, file)
    del response


def union(all_x, all_y, path):
    x_layers = []
    for x_index in range(all_x):
        file_path = os.path.join(path, "_".join(map(str, [x_index, 0])))
        print(file_path)
        img = cv2.imread(file_path)
        for y_index in range(1, all_y):
            file_path = os.path.join(path, "_".join(map(str, [x_index, y_index])))
            print(file_path)
            if os.path.exists(file_path) and os.path.isfile(file_path):
            	print(img.shape)
            	img = np.concatenate((img, cv2.imread(file_path)), axis=0)
            else:
                print("fail")
                break
        x_layers.append(img)

    final_image = x_layers[0]
    for layer in range(1, all_x):
        final_image = np.concatenate((final_image, x_layers[layer]), axis=1)

    cv2.imwrite(os.path.join(path, 'map.png'), final_image)

    return final_image

def main():


    """
    https://api.openstreetmap.org/api/0.6/map?bbox=82.54715,54.839455,83.182984,55.103517
    https://sat02.maps.yandex.net/tiles?l=sat&v=3.465.0&x=2989&y=1297&z=12&lang=ru_RU
    """
    city_min_x = 5975
    city_max_x = 5989
    city_min_y = 2582
    city_max_y = 2597


    all_x = city_max_x - city_min_x + 1
    all_y = city_max_y - city_min_y + 1
    path = './google_tiles_' + str(13) + '/'

    for x_index in range(5975, 5990):
        for y_index in range(2582, 2598):
        	file_name = os.path.join(path, "_".join(map(str, [x_index, y_index])) + '.png')
        	get_route_tile(x_index, y_index, file_name)
        	time.sleep(0.1)
    final_image = union(all_x, all_y, path)

if __name__ == '__main__':
    main()