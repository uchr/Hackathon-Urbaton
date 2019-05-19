import numpy as np
import cv2
import os
import time
import requests
import shutil
import json

def main():
    inputPath = '../Data/Tiles/green_tiles'
    outputPath = '../Data/Tiles/green_tiles_scaled'

    leftBottomTile13 = {"x": 5975, "y": 2598};
    rightUpperTile13 = {"x": 5990, "y": 2582};

    leftBottomTile12 = {"x": 2988, "y": 1298};
    rightUpperTile12 = {"x": 2994, "y": 1291};

    leftBottomTile11 = {"x": 1494, "y": 648};
    rightUpperTile11 = {"x": 1496, "y": 646};

    print(inputPath)
    print(outputPath)
    print(leftBottomTile13)
    print(rightUpperTile13)
    print(leftBottomTile12)
    print(rightUpperTile12)
    print(leftBottomTile11)
    print(rightUpperTile11)

    for i in range(leftBottomTile13["x"], rightUpperTile13["x"] + 1):
        for j in range(rightUpperTile13["y"], leftBottomTile13["y"] + 1):
            img = cv2.imread(os.path.join(inputPath, str(i) + "_" + str(j) + ".png"), cv2.IMREAD_UNCHANGED)
            cv2.imwrite(os.path.join(outputPath, str(i) + "_" + str(j) + "_13.png"), img)


    for i in range(leftBottomTile12["x"], rightUpperTile12["x"] + 1):
        for j in range(rightUpperTile12["y"], leftBottomTile12["y"] + 1):
            img00 = cv2.imread(os.path.join(inputPath, str(i * 2) + "_" + str(j * 2) + ".png"), cv2.IMREAD_UNCHANGED)
            img01 = cv2.imread(os.path.join(inputPath, str(i * 2) + "_" + str(j * 2 + 1) + ".png"), cv2.IMREAD_UNCHANGED)
            img10 = cv2.imread(os.path.join(inputPath, str(i * 2 + 1) + "_" + str(j * 2) + ".png"), cv2.IMREAD_UNCHANGED)
            img11 = cv2.imread(os.path.join(inputPath, str(i * 2 + 1) + "_" + str(j * 2 + 1) + ".png"), cv2.IMREAD_UNCHANGED)
            img0 = np.concatenate((img00, img01), axis=0)
            img1 = np.concatenate((img10, img11), axis=0)
            img = np.concatenate((img0, img1), axis=1)
            cv2.imwrite(os.path.join(outputPath, str(i) + "_" + str(j) + "_12.png"), img)

    for i in range(leftBottomTile11["x"], rightUpperTile11["x"] + 1):
        for j in range(rightUpperTile11["y"], leftBottomTile11["y"] + 1):
            img00 = cv2.imread(os.path.join(outputPath, str(i * 2) + "_" + str(j * 2) + "_12.png"), cv2.IMREAD_UNCHANGED)
            img01 = cv2.imread(os.path.join(outputPath, str(i * 2) + "_" + str(j * 2 + 1) + "_12.png"), cv2.IMREAD_UNCHANGED)
            img10 = cv2.imread(os.path.join(outputPath, str(i * 2 + 1) + "_" + str(j * 2) + "_12.png"), cv2.IMREAD_UNCHANGED)
            img11 = cv2.imread(os.path.join(outputPath, str(i * 2 + 1) + "_" + str(j * 2 + 1) + "_12.png"), cv2.IMREAD_UNCHANGED)
            img0 = np.concatenate((img00, img01), axis=0)
            img1 = np.concatenate((img10, img11), axis=0)
            img = np.concatenate((img0, img1), axis=1)
            cv2.imwrite(os.path.join(outputPath, str(i) + "_" + str(j) + "_11.png"), img)

if __name__ == '__main__':
    main()