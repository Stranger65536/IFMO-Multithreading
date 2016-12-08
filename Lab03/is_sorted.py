import sys

lst = []
with open(sys.argv[1], 'r') as f:
    for line in f:
        lst.append(line.strip().split(' '))

print('Sorted: ', lst == sorted(lst))