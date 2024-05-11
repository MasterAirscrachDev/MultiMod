import argparse
import os
from PIL import Image

def image_encoder(data, filename):
    print(f"Data: {data}, Filename: {filename}")
    #encoded message length is data / 4 rounded up
    enc_message_length = (len(data) + 3) // 4
    x_res = 1
    y_res = 1
    #if the encoded message length is less than 511, then the x_res is the encoded message length
    if enc_message_length + 1 <= 512:
        x_res = enc_message_length + 1
    #if the encoded message length is greater than 512, then the x_res is 512
    else:
        x_res = 512
        y_res = (enc_message_length + 1) // 512 + 1
    
    if(x_res > 512):
        print("Error: x_res is greater than 512")
        return
    if(y_res > 512):
        print("Error: y_res is greater than 512")
        return

    encoded_image = Image.new('RGBA', (x_res, y_res), color = 'black')
    data_index = 0
    for y in range(y_res):
        for x in range(x_res):
            if(x == 0 and y == 0):
                # R + G is the width of the image
                # B + A is the length of the image
                h = x_res
                h2 = 0
                j = y_res
                j2 = 0
                if(h > 255):
                    h = 255
                    h2 = x_res - 255
                if(j > 255):
                    j = 255
                    j2 = y_res - 255
                encoded_image.putpixel((x, y), (h, h2, j, j2))
                print(f"Side Data: X: {h}, X2: {h2}, Y: {j}, Y2: {j2}")
            else:
                if(data_index < len(data)):
                    r = ord(data[data_index])
                    g = 0
                    b = 0
                    a = 0
                    chars = data[data_index]
                    if(data_index + 1 < len(data)):
                        g = ord(data[data_index + 1])
                        chars += data[data_index + 1]
                    if(data_index + 2 < len(data)):
                        b = ord(data[data_index + 2])
                        chars += data[data_index + 2]
                    if(data_index + 3 < len(data)):
                        a = ord(data[data_index + 3])
                        chars += data[data_index + 3]
                    encoded_image.putpixel((x, y), (r, g, b, a))
                    print(f"Data from {chars}: R:({r},{256 - r}), G:({g},{256 - g}), B:({b},{256 - b}), A:({a},{256 - a})")
                    data_index += 4
                else:
                    encoded_image.putpixel((x, y), (0, 0, 0, 0))

    encoded_image.save(filename + '.png')


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Process some data and a filename.')
    parser.add_argument('data', type=str, help='The data to be processed')
    parser.add_argument('filename', type=str, help='The filename to be used')
    args = parser.parse_args()
    image_encoder(args.data, args.filename)

#player2:chat2:meta2:world1:2.0:3.0:4.0:5.0:6.0:7.0