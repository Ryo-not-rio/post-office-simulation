import os
import matplotlib.pyplot as plt
import numpy as np
from copy import deepcopy
from math import log, exp, ceil

try:
    with open("data.npy", "rb") as f:
        files = list(np.load(f, allow_pickle=True))
except (FileNotFoundError, EOFError, OSError):
    print("Warning: file open error")
    files = []

for i, file in enumerate(os.listdir("experiments/")):
    data = {}
    with open(os.path.join("experiments", file)) as f:
        r = f.readlines()
    try:
        params_string = [x.replace(' ', '') for x in r[0][:-1].split(',')]
        data['parameters'] = {k: v for k, v in zip(params_string, [float(x) for x in r[1][:-1].split(',')])}
        
        data['avFulfilled'] = float(r[2].split(' ')[-1][:-1])
        data['avUnfulfilled'] = float(r[3].split(' ')[-1][:-1])
        data['avTimedOut'] = float(r[4].split(' ')[-1][:-1])
        data['avAvWaitTime'] = float(r[5].split(' ')[-1][:-1])
        data['avEmptyTime'] = float(r[6].split(' ')[-1][:-1])
        files.append(data)
        os.remove(os.path.join("experiments", file))
    except IndexError:
        os.remove(os.path.join("experiments", file))

    if i%2000 == 0:
        try:
            with open("data.npy", "wb") as f:
                np.save(f, np.array(files))
        except KeyboardInterrupt:
            print("Currently saving file")
            with open("data.npy", "wb") as f:
                np.save(f, np.array(files))

with open("data.npy", "wb") as f:
    np.save(f, np.array(files))

def clean_data(data, save_file):
    data_dict = {}
    count_dict = {}

    for d in data:
        key = ",".join([str(x) + ":" + str(y) for x, y in d['parameters'].items()])
        if key not in data_dict:
            count_dict[key] = 1
            data_dict[key] = {k:v for k, v in d.items() if k != 'parameters'}
        else:
            count_dict[key] += 1
            for k in data_dict[key]:
                data_dict[key][k] += d[k]

    new_data = []
    for k, v in data_dict.items():
        for k2, v2 in v.items():
            data_dict[k][k2] = v2/count_dict[k]

        new_dict = {}
        
        new_dict['parameters'] = {x: float(y) for z in k.split(",") for x, y in [z.split(":")]}
        new_dict = {**new_dict, **{k3:v3 for k3, v3 in v.items() if k3 != 'parameters'}}
        new_data.append(new_dict)

    with open(save_file, "wb") as f:
        np.save(f, np.array(new_data))

    return data

def find_similar(required_params, files=files, tolerance=0.001):
    found = []
    for data in files:
        match = True
        for param, value in required_params.items():
            try:
                if abs(data['parameters'][param] - value) > tolerance:
                    match = False
                    break
            except KeyError:
                match = False
                break
        if match: found.append(data)
    return found

def plot_graph(data_match_params, plot_params):
    x, y = [], []
    for data in find_similar(data_match_params):
        data = {**data['parameters'], **data}
        x.append(data[plot_params[0]])
        y.append(data[plot_params[1]])

    plt.scatter(x, y, label=plot_params[1])

def plot_total(data_match_params, x_param):
    x, y = [], []
    for data in find_similar(data_match_params):
        data = {**data['parameters'], **data}
        x.append(data[x_param])
        y.append(data['avTimedOut'] + data['avUnfulfilled'])

    plt.scatter(x, y)

def plot_matrix(params, success=0.95):
    horizontal_axis = list(range(5, 30)) # queue
    vertical_axis = list(range(2, 30)) # tills

    matrix = np.zeros((len(vertical_axis), len(horizontal_axis)))

    for data in find_similar(params):
        tillNum, length = data['parameters']['tillNum']+1, data['parameters']['maxLength']+1
        tillNum, length = int(tillNum-vertical_axis[0]), int(length-horizontal_axis[0])
        try:
            matrix[tillNum, length] = data['avFulfilled']/(data['avFulfilled'] + data['avUnfulfilled'] + data['avTimedOut'])
        except IndexError:
            pass
    
    fig, ax = plt.subplots()
    im = ax.imshow(matrix)
    ax.invert_yaxis()
    ax.set_aspect(0.6)
    
    ax.set_xticks(np.arange(len(horizontal_axis)))
    ax.set_yticks(np.arange(len(vertical_axis)))
    # ... and label them with the respective list entries
    ax.set_xticklabels([x-1 for x in horizontal_axis])
    ax.set_yticklabels([x-1 for x in vertical_axis])

    for i in range(len(vertical_axis)):
        for j in range(len(horizontal_axis)):
            if matrix[i, j] >= success:
                color = "orangered"
            else:
                color = "w"
            text = ax.text(j, i, round(matrix[i, j], 2),
                               ha="center", va="center", color=color)

    ax.set_title(f"arriveRate={round(params['arriveRate'], 2)}")
    ax.set_xlabel('max queue length')
    ax.set_ylabel('num service points')

    fig.tight_layout()
    plt.show()

def is_successful(data1, success=0.95):
    return data1['avFulfilled']/(data1['avFulfilled'] + data1['avUnfulfilled'] + data1['avTimedOut']) >= success
        

def find_min_queue_tills(params, success=0.95):
    min_queue, min_tills = float('inf'), float('inf')
    for data in find_similar(params):
        params = data['parameters']
        if is_successful(data, success):
            if params['maxLength'] < min_queue: min_queue = params['maxLength']
            if params['tillNum'] < min_tills: min_tills = params['tillNum']
    return min_queue, min_tills


def find_boundary(params, success=0.95):
    min_queue, min_tills = float('inf'), float('inf')
    left_corner, right_corner = (float('inf'), float('inf')), (float('inf'), float('inf'))
    datas = deepcopy(find_similar(params))
    for data in datas:
        params = data['parameters']
        if is_successful(data, success):
            if params['maxLength'] < min_queue:
                min_queue = params['maxLength']
                left_corner = (min_queue, params['tillNum'])
            elif params['maxLength'] == min_queue:
                if params['tillNum'] < left_corner[1]:
                    left_corner = (min_queue, params['tillNum'])
            if params['tillNum'] < min_tills:
                min_tills = params['tillNum']
                right_corner = (params['maxLength'], min_tills)
            elif params['tillNum'] == min_tills:
                if params['maxLength'] < right_corner[0]:
                    right_corner = (params['maxLength'], min_tills)

    coordinates = [left_corner]
    queue, till = left_corner[:]
    while queue<right_corner[0]:
        successful = True
        queue += 1
        while successful:
            next_coor = [queue, till]
            found = False
            while not found:
                till -= 1
                params['maxLength'], params['tillNum'] = queue, till
                found = find_similar(params, files=datas)
                
            successful = is_successful(found[0])
        till += 1
        coordinates.append(next_coor)
    
    return np.array(coordinates)

def find_equation(coor1, coor2, k=None, b=None):
    if k is None and b is None:
        k = (coor1[0]*coor2[0]*(coor2[1]-coor1[1]))/(coor1[0]-coor2[0])
    if b is None:
        b = coor1[1] - k/(coor1[0])
    elif k is None:
        k = (coor1[1]-b)*coor1[0]
    return k, b

def find_y(x, k, b):
    return round(k/x+b)

def predict_coordinates(coordinates, k=None, b=None):
    k, b = find_equation(coordinates[0], coordinates[-1], k=k, b=b)
    return k, b, np.array([(c[0], find_y(c[0], k, b)) for c in coordinates])

def rmse(arr1, arr2):
    return np.sqrt(np.sum(np.power(arr1-arr2, 2))/arr1.shape[0])

def absolute_error(arr1, arr2):
    return np.sum(np.abs(arr1-arr2))/arr1.shape[0]

def get_max_error(arr1, arr2):
    return np.max(np.abs(arr1-arr2))


# plot_total({'tillNum': 2, 'maxLength': 7, 'averageWaitLimit': 7, 'waitLimitSD': 2, 'averageServeTime': 3, 'serveTimeSD': 2}, 'arriveRate')
# for i in range(5, 21, 5):
   #  plot_graph({'tillNum': 2, 'maxLength': i, 'averageWaitLimit': 7, 'waitLimitSD': 2, 'averageServeTime': 3, 'serveTimeSD': 2}, ['arriveRate', 'avUnfulfilled'])
    # plot_graph({'tillNum': 2, 'maxLength': i, 'averageWaitLimit': 7, 'waitLimitSD': 2, 'averageServeTime': 3, 'serveTimeSD': 2}, ['arriveRate', 'avTimedOut'])

# for i in range(46, 70, 8):
    # plot_matrix({'arriveRate': i*0.1, 'averageWaitLimit': 7, 'waitLimitSD': 2, 'averageServeTime': 3, 'serveTimeSD': 2})

rates, rmses, absolutes, maxes, ks, bs = [], [], [], [], [], []
for i in range(30, 100, 1):
    rates.append(i*0.1)
    coordinates = find_boundary({'arriveRate': i*0.1, 'averageWaitLimit': 7, 'waitLimitSD': 2, 'averageServeTime': 3, 'serveTimeSD': 2})
    k, b, predicted = predict_coordinates(coordinates, k=10.22*i*0.1-15.64, b=2.137*i*0.1-0.931)
    ks.append(k)
    bs.append(b)
    rmses.append(rmse(coordinates, predicted))
    absolutes.append(absolute_error(coordinates, predicted))
    maxes.append(get_max_error(coordinates, predicted))

#     if rmse(coordinates, predicted) > 6:
   #      print(i*0.1)
      #   print(find_equation(coordinates[0], coordinates[-1]))
        # print(coordinates.tolist(), "\n", predicted.tolist())
        # plot_matrix({'arriveRate': i*0.1, 'averageWaitLimit': 7, 'waitLimitSD': 2, 'averageServeTime': 3, 'serveTimeSD': 2})

# plt.bar(rates, maxes, label="max value", color="red")
plt.plot(rates, absolutes, label="mean absolute error")
plt.plot(rates, np.array(rmses), label="Rmse")
# plt.plot(rates, ks, label="k")
# plt.plot(rates, bs, label="b")

plt.title("k=10.22*arriveRate-15.64, b=2.137*arriveRate-0.931")
plt.xlabel("Arrive Rate")
plt.ylabel("Value")
plt.legend()
plt.show()








    

    
