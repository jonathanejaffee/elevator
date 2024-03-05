import argparse
import random
import csv
import os

parser = argparse.ArgumentParser()

parser.add_argument("-n", "--number", default=10, type=int, help="Number of examples to create")
parser.add_argument("-f", "--floors", type=int, default=10, help="Number of floors")
parser.add_argument("-s", "--seed", type=int, default=1234, help="Random Seed")
parser.add_argument("-o", "--output", type=str, default="./input/inputData.csv", help="Where to put the generated input data file")
parser.add_argument("-d", "--data", type=str, default="./data/", help="Directory containing the human dataset")
parser.add_argument("-t", "--time", type=int, default=30, help="Max time delta between actions")
args = parser.parse_args()

def main(args):
    random.seed(args.seed)
    fields = ['floor', 'direction', 'image', 'time']
    with open(args.output, 'w') as csvfile:
        csvwriter = csv.writer(csvfile)
        csvwriter.writerow(fields)
        lastFloor = 0
        for i in range(args.number):
            floor = random.randint(1, args.floors)
            while (floor == lastFloor):
                floor = random.randint(1, args.floors)
            direction = random.randint(-1, 1)
            time = random.randint(0, args.time)
            image = random.choice(os.listdir(args.data))
            image = args.data + image
            image = os.path.abspath(image)
            row = [floor, direction, image, time]
            csvwriter.writerow(row)
            lastFloor = floor


if __name__ == "__main__":
    main(args)