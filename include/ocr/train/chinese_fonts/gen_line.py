#! /usr/bin/env python
# -*- coding: utf-8 -*-
from gen_utils import *

if 1:
    fontfiles = os.listdir('D:/fonts/ttf/')
    random.shuffle(fontfiles)
    fontfiles = fontfiles[:200] + ['huawenxihei.ttf', 'fz-v4.0.ttf']
    fontfiles = ['huawenxihei.ttf', 'fz-v4.0.ttf']
    outpath = 'E:/OCR_Line/lines'

imgoutpath=outpath+'/img'
mkdir(outpath)
mkdir(imgoutpath)
fonts = []

fontsize = 32

for fontfile in fontfiles:
    font = ImageFont.truetype('D:/fonts/ttf/'+fontfile, fontsize)
    fonts.append(font)

chars = '0123456789函数则是相当于'
chars = '0123456789'

labels = []

def gen_char():
    if rand() < 0.9:
        return chars[randint(0, len(chars))]
    return ' '


for i in range(12000):
    a = 1.1+rand()
    b = 1.1+rand()
    h = int(32*a)
    w = int(280*b)

    while 1:
        b = randint(0, 255)
        f = randint(0, 255)
        if abs(b-f) > 30:
            break
    bgColor = (b, b, b)
    fillColor = (f, f, f)

    img_OpenCV2 = np.ones((h, w, 3), dtype=np.uint8)
    cv2.rectangle(img_OpenCV2, (0, 0), (w, h), bgColor, -1)
    img_PIL = Image.fromarray(cv2.cvtColor(img_OpenCV2, cv2.COLOR_BGR2RGB))
    draw = ImageDraw.Draw(img_PIL)
    pos_x = randint(-5, 5)
    pos_y0 = randint(0, h-fontsize-4)
    ss = []
    font = fonts[randint(0, len(fonts))]
    for j in range(32):
        pos_y = pos_y0 + 0*randint(-2, 2)
        ch = ' '
        idx = -1
        if rand() < 0.9:
            idx = randint(0, len(chars))
            ch = chars[idx]
            size = font.getsize(ch)
            if size[0]+pos_x < w:
                idx = idx + 1
                draw.text((pos_x, pos_y), ch, font=font, fill=fillColor)
                pos_x = pos_x + size[0] + randint(-5, 5)
            else:
                idx = -1
        else:
            if (fontsize/2)+pos_x < w:
                idx = 0
                pos_x = pos_x + fontsize/2
            else:
                idx = -1

        ss.append(idx)


    im = cv2.cvtColor(np.asarray(img_PIL), cv2.COLOR_RGB2GRAY)

    k=3
    if 1:
        if rand()>0.5:
            if b>f:
                im = add_erode(im, randint(1,k))
            else:
                im = add_dilate(im, randint(1,k))
        else:
            if b>f:
                im = add_erode(im, randint(1,k))
            else:
                im = add_dilate(im, randint(1,k))

    im = add_noise(im, randint(10,50))
    if 1:
        im1 = add_shader(im)
        k=0.4
        im = im1*k + im*(1-k)
    # im = HSVequalizeHist(im)
    im = motion_blur(im, randint(2, 5), randint(0, 360))
    im = gauss_blur(im, 5, randint(5, 10))
    
    if 1:
        fn = '%s/%d.jpg' % (imgoutpath, i)
        im = cv2.resize(im, (280, 32))
        la = ' '.join(map(str, ss))
        s = '%s;%s' % (fn, la)
        labels.append(s)
        print(h, w, fn)
        # cv2.imwrite(fn, im)
        cv2.imencode('.jpg', im)[1].tofile(fn)

def savelines(labels, outtxt):
    f = open(outtxt, 'w')
    for s in labels:
        #print(s)
        f.write(s+'\n')
    f.close()

count = len(labels)
count_train = int(count*0.8)

root = outpath + '/'
outtxt = root+'list.txt'
savelines(labels[:count_train], root+'train.txt')
savelines(labels[count_train:], root+'test.txt')
