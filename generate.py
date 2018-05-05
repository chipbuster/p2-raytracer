#! /usr/bin/python3

import sys
import os
import subprocess
import argparse
import json
import numpy as np
from PIL import Image

def error(message):
    print(message, file=sys.stderr)
    sys.exit(1)

def readJson(jsonfile):
    try:
        with open(jsonfile) as f:
            data = json.load(f)
            if 'samples' not in data or 'lightSamples' not in data:
                error('json file "{}" missing some parameters'.format(jsonfile))
            return data['samples'], data['lightSamples']
    except:
        error('json file "{}" does not exist or is invalid'.format(jsonfile))

def getImage(samples, lightSamples, scene, foldername, imgname):
    os.chdir(foldername)
    command = 'ray -r 1 -s {} -l {} {} {}'.format(samples, lightSamples, scene, imgname) 
    output = subprocess.check_output(command, shell=True, stderr=subprocess.STDOUT)
    image = Image.open(imgname)
    os.chdir('..')
    return np.asarray(image)

def getImages(samples, lightSamples, scene, foldername):
    images = [[getImage(s, ls, scene, foldername, 'test{}_{}.png'.format(s, ls)) for s in samples]
              for ls in lightSamples]
    images = np.vstack([np.hstack(row) for row in images])
    images = Image.fromarray(images)
    return images

def main(args):
    samples, lightSamples =  readJson(args.json)
    foldername = '.test1234'
    subprocess.call('rm -rf {}'.format(foldername), shell=True)
    subprocess.call('mkdir -p {}'.format(foldername), shell=True)
    try:
        subprocess.check_output('cp build/bin/ray {}'.format(foldername),
                                shell=True, stderr=subprocess.STDOUT)
    except:
        error('ray tracer executable not located')
    try:
        subprocess.check_output('cp {} {}/scene.ray'.format(args.input, foldername),
                                shell=True, stderr=subprocess.STDOUT)
    except:
        error('scene file missing')
    images = getImages(samples, lightSamples, 'scene.ray', foldername)
    images.save(args.output)
    subprocess.call('rm -rf {}'.format(foldername), shell=True)

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--input', help='input ray file', required=True)
    parser.add_argument('--json', help='json file for configuration', required=True)
    parser.add_argument('--output', help='output png file', required=True)
    args = parser.parse_args()
    main(args)
