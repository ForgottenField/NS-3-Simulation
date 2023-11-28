import numpy as np
import matplotlib.pyplot as plt
import re

def read_data(file_path):
    data = []
    entry = {'parameters': {}, 'qos': {}}
    flag = False

    with open(file_path, 'r') as file:
        for line in file:
            if line.strip():
                key, value = map(str.strip, line.split(': ', 1))

                if key == "Flow ID" and value == "1":
                    flag = True
                else:
                    if key == "Flow ID":
                        flag = False
                        if entry != {'parameters': {}, 'qos': {}}:
                            data.append(entry)
                        entry = {'parameters': {}, 'qos': {}}
                    elif key in ['TCP_rate', 'UDP_rate', 'tcp_buffer_size', 'router_buffer_size', 'MTU']:
                        entry['parameters'][key] = value
                    elif key in ['Throughput', 'Transmission Delay', 'Packet Loss Rate', 'Jitter'] and flag == True:
                        entry['qos'][key] = value

    return data

def extract_value(value_str):
    # Extract numeric values from the string
    match = re.search(r"[-+]?\d*\.\d+|\d+", value_str)
    return float(match.group()) if match else None

def process_data(data, qos_parameter):
    router_buffers = []
    qos_values = []

    for entry in data:
        parameters = entry['parameters']
        qos = entry['qos']

        router_buffer = extract_value(parameters['router_buffer_size'])
        qos_value = extract_value(qos[qos_parameter])

        if router_buffer is not None and qos_value is not None:
            router_buffers.append(router_buffer)
            qos_values.append(qos_value)

    return router_buffers, qos_values

def plot_figure(x, y, xlabel, ylabel, title, filename):
    plt.plot(x, y, marker='o')
    plt.xlabel(f'{xlabel} (Mbps)')
    plt.ylabel(f'{ylabel}')
    plt.title(f'{title} vs. {xlabel} (Mbps)')
    plt.grid(True)
    plt.savefig(filename)
    plt.close()

def get_unit(qos_parameter):
    if qos_parameter == 'Throughput':
        return 'Mbps'
    elif qos_parameter == 'Transmission Delay':
        return 'e+07ns'
    elif qos_parameter == 'Packet Loss Rate':
        return '(txPackets - rxPackets) / txPackets'
    elif qos_parameter == 'Jitter':
        return 'ns'
    else:
        return 'units'

if __name__ == "__main__":
    file_path = "ATCN-Program-routerBufferSize.txt"
    data = read_data(file_path)
    #print(data)
    for qos_parameter in ['Throughput', 'Transmission Delay', 'Packet Loss Rate', 'Jitter']:
        router_buffers, qos_values = process_data(data, qos_parameter)
        unit = get_unit(qos_parameter)  # Implement get_unit function based on your requirements
        plot_filename = f'{qos_parameter}_vs_Router_Buffer_Size.png'
        plot_figure(router_buffers, qos_values, 'Router Buffer Size', f'{qos_parameter} ({unit})', f'{qos_parameter} vs. Router Buffer Size', plot_filename)