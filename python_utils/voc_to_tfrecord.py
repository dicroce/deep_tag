
import sys
import os
import io
import glob
import hashlib
import random
import pathlib
import shutil
import pandas as pd
import xml.etree.ElementTree as et
import tensorflow.compat.v1 as tf
import label_map_util
import argparse
from PIL import Image
from object_detection.utils import dataset_util

def parse_and_write_annotation(writer, annotation_path, images_dir, label_map):
    root_element = et.parse(annotation_path).getroot()

    image_name = root_element.find('filename').text
    file_name = image_name.encode('utf8')

    size_element = root_element.find('size')
    width = float(size_element.find('width').text)
    height = float(size_element.find('height').text)

    object_elements = root_element.findall('object')

    xmin = []
    ymin = []
    xmax = []
    ymax = []
    classes = []
    classes_text = []
    truncated = []
    poses = []
    difficult = []

    for object_element in object_elements:
        name = object_element.find('name').text

        classes.append(label_map[name])
        classes_text.append(name.encode('utf8'))
        
        bndbox_element = object_element.find('bndbox')
        xmin.append(float(bndbox_element.find('xmin').text) / width)
        ymin.append(float(bndbox_element.find('ymin').text) / height)
        xmax.append(float(bndbox_element.find('xmax').text) / width)
        ymax.append(float(bndbox_element.find('ymax').text) / height)

        difficult.append(0)
        truncated.append(0)
        poses.append('Unspecified'.encode('utf8'))

        image_path = images_dir + "/" + image_name
        with tf.gfile.GFile(image_path, 'rb') as fid:
            encoded_jpg = fid.read()
        encoded_jpg_io = io.BytesIO(encoded_jpg)
        image = Image.open(encoded_jpg_io)
        if image.format != 'JPEG':
            raise ValueError('Image format not JPEG')
        key = hashlib.sha256(encoded_jpg).hexdigest()

        example = tf.train.Example(features=tf.train.Features(feature={
            'image/height': dataset_util.int64_feature(int(height)),
            'image/width': dataset_util.int64_feature(int(width)),
            'image/filename': dataset_util.bytes_feature(file_name),
            'image/source_id': dataset_util.bytes_feature(file_name),
            'image/key/sha256': dataset_util.bytes_feature(key.encode('utf8')),
            'image/encoded': dataset_util.bytes_feature(encoded_jpg),
            'image/format': dataset_util.bytes_feature('jpeg'.encode('utf8')),
            'image/object/bbox/xmin': dataset_util.float_list_feature(xmin),
            'image/object/bbox/xmax': dataset_util.float_list_feature(xmax),
            'image/object/bbox/ymin': dataset_util.float_list_feature(ymin),
            'image/object/bbox/ymax': dataset_util.float_list_feature(ymax),
            'image/object/class/text': dataset_util.bytes_list_feature(classes_text),
            'image/object/class/label': dataset_util.int64_list_feature(classes),
            'image/object/difficult': dataset_util.int64_list_feature(difficult),
            'image/object/truncated': dataset_util.int64_list_feature(truncated),
            'image/object/view': dataset_util.bytes_list_feature(poses),
        }))

        writer.write(example.SerializeToString())

def main():
    parser = argparse.ArgumentParser(description='Convert VOC annotations to tensorflow tfrecord files.')
    parser.add_argument('-a','--annotations_dir', help='Directory containing VOC XML annotations.', required=True)
    parser.add_argument('-i','--images_dir', help='Directory contains JPEG images.', required=True)
    args = vars(parser.parse_args())

    annotations_path = args["abbb"]
    images_path = parser.images_dir
    
    annotations = os.listdir(annotations_path)
    random.shuffle(annotations)
    lm = {'person': 1, 'bird': 2, 'cat': 3, 'cow': 4, 'dog': 5, 'horse': 6, 'sheep': 7, 'aeroplane': 8, 'bicycle': 9, 'boat': 10, 'bus': 11, 'car': 12, 'motorbike': 13, 'train': 14, 'bottle': 15, 'chair': 16, 'diningtable': 17, 'pottedplant': 18, 'sofa': 19, 'tvmonitor': 20 }

    train_writer = tf.python_io.TFRecordWriter('train.record')
    test_writer = tf.python_io.TFRecordWriter('test.record')

    num_for_test = int(len(annotations) * 0.15)

    for i in range(len(annotations)):
        apath = annotations_path + '/' + annotations[i]
        parse_and_write_annotation(test_writer if i < num_for_test else train_writer, apath, images_path, lm)

    train_writer.close()
    test_writer.close()

if __name__ == "__main__":
    main()
