piece_values = [
    100, 320, 330, 500, 900,
]
for i in range(5):
    print('{ ', end='')
    for j in range(5):
        print(piece_values[i] - piece_values[j], end=', ')
    print('0, },')
for i in range(2):
    print('{ ' + '0, ' * 6 + '},')