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
    tcp_rates = []
    qos_values = []

    for entry in data:
        parameters = entry['parameters']
        qos = entry['qos']

        tcp_rate = extract_value(parameters['TCP_rate'])
        qos_value = extract_value(qos[qos_parameter])

        if tcp_rate is not None and qos_value is not None:
            tcp_rates.append(tcp_rate)
            qos_values.append(qos_value)

    return tcp_rates, qos_values

def plot_figure(x, y, xlabel, ylabel, title, filename):
    plt.plot(x, y, marker='o')
    plt.xlabel(xlabel)
    plt.ylabel(ylabel)
    plt.title(title)
    plt.grid(True)
    plt.savefig(filename)
    plt.close()

if __name__ == "__main__":
    file_path = "ATCN-Program-Tcprate.txt"
    data = read_data(file_path)
    #print(data)
    for qos_parameter in ['Throughput', 'Transmission Delay', 'Packet Loss Rate', 'Jitter']:
        tcp_rates, qos_values = process_data(data, qos_parameter)
        plot_filename = f'{qos_parameter}_vs_TCP_Rate.png'
        plot_figure(tcp_rates, qos_values, 'TCP Rate (Mbps)', qos_parameter, f'{qos_parameter} vs. TCP Rate', plot_filename)