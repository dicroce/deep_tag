import sys
import os
import random
import xml.etree.ElementTree as ET
import pathlib
import shutil

def image_path(annotation_path):
    return ET.parse(annotation_path).getroot().find('filename').text

def main():
    annotations_path = sys.argv[1]
    images_path = sys.argv[2]
    output_annotations_path = sys.argv[3] + '/annotations'
    output_images_path = sys.argv[3] + '/images'
    annotations = os.listdir(annotations_path)
    annotation_files = os.listdir(annotations_path)
    random.shuffle(annotation_files)

    pathlib.Path(output_annotations_path).mkdir(parents=True, exist_ok=True)
    pathlib.Path(output_images_path).mkdir(parents=True, exist_ok=True)

    num_to_move = int(len(annotation_files) * 0.15)

    for i in range(num_to_move):
        apath = annotations_path + '/' + annotation_files[i]
        img_file_name = image_path(apath)
        ipath = images_path + '/' + img_file_name
        shutil.move(apath, output_annotations_path + '/' + annotation_files[i])
        shutil.move(ipath, output_images_path + '/' + img_file_name)
        print(ipath)

if __name__ == "__main__":
    main()
