from PIL import Image
import argparse

def image_decoder(filename):
    decoded_data = ""
    encoded_image = Image.open(filename)
    width, height = encoded_image.size
    data_index = 0
    for y in range(height):
        for x in range(width):
            r, g, b, a = encoded_image.getpixel((x, y))
            if x == 0 and y < 2:
                # This is the side data, skip it
                continue
            else:
                if r != 0:
                    decoded_data += chr(r)
                if g != 0:
                    decoded_data += chr(g)
                if b != 0:
                    decoded_data += chr(b)
                if a != 0:
                    decoded_data += chr(a)
    return decoded_data

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Process a filename.')
    parser.add_argument('filename', type=str, help='The filename to be used')
    args = parser.parse_args()
    decoded_data = image_decoder(args.filename)
    print(decoded_data)