
from __future__ import print_function

from PIL import Image
from PIL import ImageFont
from PIL import ImageDraw
import pickle
import argparse
from argparse import RawTextHelpFormatter
import fnmatch
import os
import cv2
import json
import random
import numpy as np
import shutil
import traceback
import copy
cv = cv2


def rand():
    return random.random()


def randint(min_int, max_int):
    return int(min_int + (max_int-min_int)*random.random())


def randi(min_int, max_int):
    lis = []
    for i in range(len(max_int)):
        lis.append(randint(min_int[i], max_int[i]))
    return tuple(lis)


def mkdir(path):
    isExists = os.path.exists(path)
    if not isExists:
        os.makedirs(path)

def gauss_blur(image, ksize, sigma):
    return cv2.GaussianBlur(image, (ksize, ksize), sigma, sigma)


def motion_blur(image, degree=12, angle=45):
    image = np.array(image)
    # 这里生成任意角度的运动模糊kernel的矩阵， degree越大，模糊程度越高
    M = cv2.getRotationMatrix2D((degree / 2, degree / 2), angle, 1)
    motion_blur_kernel = np.diag(np.ones(degree))
    motion_blur_kernel = cv2.warpAffine(
        motion_blur_kernel, M, (degree, degree))

    motion_blur_kernel = motion_blur_kernel / degree
    blurred = cv2.filter2D(image, -1, motion_blur_kernel)

    # convert to uint8
    cv2.normalize(blurred, blurred, 0, 255, cv2.NORM_MINMAX)
    blurred = np.array(blurred, dtype=np.uint8)
    return blurred


def add_noise(img, n):
    for i in range(n):  # 添加点噪声
        temp_x = np.random.randint(0, img.shape[0])
        temp_y = np.random.randint(0, img.shape[1])
        img[temp_x][temp_y] = 255
    return img


def odd(k):
    if (k % 2) == 0:
        return k+1
    return k


def add_erode(img, k):
    k = odd(k)
    kernel = cv2.getStructuringElement(cv2.MORPH_RECT, (k, k))
    img = cv2.erode(img, kernel)
    return img


def add_dilate(img, k):
    k = odd(k)
    kernel = cv2.getStructuringElement(cv2.MORPH_RECT, (k, k))
    img = cv2.dilate(img, kernel)
    return img

def YUVequalizeHist(img):
    imgYUV = cv2.cvtColor(img, cv2.COLOR_BGR2YCrCb)
    channelsYUV = cv2.split(imgYUV)
    channelsYUV[0] = cv2.equalizeHist(channelsYUV[0])
    channels = cv2.merge(channelsYUV)
    result = cv2.cvtColor(channels, cv2.COLOR_YCrCb2BGR)
    return result


def HSVequalizeHist(img):
    imgHSV = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)
    channelsHSV = cv2.split(imgHSV)
    channelsHSV[2] = cv2.equalizeHist(channelsHSV[2])
    channels = cv2.merge(channelsHSV)
    result = cv2.cvtColor(channels, cv2.COLOR_HSV2BGR)
    return result


def equalizeHist(img):
    (b, g, r) = cv2.split(img)
    bH = cv2.equalizeHist(b)
    gH = cv2.equalizeHist(g)
    rH = cv2.equalizeHist(r)
    # 合并每一个通道
    result = cv2.merge((bH, gH, rH))
    return result


def circle(img, a, c, r, color):
    img2 = img.copy()
    cv.circle(img2, c, r, color, -1)
    img3 = img*(1-a) + img2*a
    return img3


# 定义旋转rotate函数


def rotate(image1, angle, center=None, scale=1.0):
    k = 50
    image = cv2.copyMakeBorder(image1, k, k, k, k, cv.BORDER_REFLECT)
    # 获取图像尺寸
    (h, w) = image.shape[:2]
    # 若未指定旋转中心，则将图像中心设为旋转中心
    if center is None:
        center = (w / 2, h / 2)

    # 执行旋转
    M = cv.getRotationMatrix2D(center, angle, scale)
    rotated = cv.warpAffine(image, M, (w, h))

    # 返回旋转后的图像
    out = rotated[k:(h-k), k:(w-k)]
    return out


def add_shader(img):
    img3 = img.copy()
    img3 = rotate(img3, randint(-10, 10))
    h, w = img3.shape
    k = 20
    # img3 = img3[randint(0,k):h-k,randint(0,k):w-k]
    h, w = img3.shape
    # print(h,w)
    g = 10
    k = 255
    r = 0.5+0.5*random.random()
    img3 = circle(img3, 0.1, randi([0, 0], [w, h]), randi(
        [50], [255])[0], randi([g, g, g], [k, k, k]))
    img3 = circle(img3, 0.1, randi(
        [w*2/3, 0], [w, h]), randi([10], [100])[0], randi([g, g, g], [k, k, k]))

    r = 3+2*int(random.random()*2)
    kernel = cv.getStructuringElement(cv.MORPH_RECT, (r, r))
    if random.random() < 0.5:
        img3 = cv.dilate(img3, kernel)
    else:
        img3 = cv.erode(img3, kernel)

    if random.random() > 0.5:
        k = 1/(1+random.random())
    else:
        k = 1+random.random()
    img3 = np.power(img3/float(np.max(img3)), k)
    return img3
