import matplotlib.pyplot as plt

def process_data(file_path):
    times = []
    cwnd_sizes = []

    with open(file_path, 'r') as file:
        for line in file:
            parts = line.split()
            if len(parts) == 2:
                time, cwnd = float(parts[0]), int(parts[1])
                times.append(time)
                cwnd_sizes.append(cwnd)

    return times, cwnd_sizes

def plot_figure(times, cwnd_sizes, output_file):
    plt.plot(times, cwnd_sizes)
    plt.xlabel('Time (seconds)')
    plt.ylabel('Congestion Window Size (MSS)')
    plt.title('Congestion Window Size over Time')
    plt.grid(True)
    # Adjust the x-axis limits
    plt.xlim(min(times), max(times))
    plt.savefig(output_file)  # Save the figure to a PNG file
    plt.close()

if __name__ == "__main__":
    file_path = "NewReno/ATCN-Program-cwnd.txt"  # Replace with the actual file path
    output_file = "ATCN-Program-cwnd.png"  # Replace with the desired output file path
    times, cwnd_sizes = process_data(file_path)
    plot_figure(times, cwnd_sizes, output_file)