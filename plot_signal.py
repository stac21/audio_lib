import matplotlib.pyplot as plt
import argparse
import numpy

def calc_num_samples(time: int, sample_rate: int) -> int:
    return int(time * (sample_rate / 1000.0))

def read_signal_file(file_path: str, time_window: int) -> tuple[(list[float], list[float], int)]:
    left_samples: list[float]  = []
    right_samples: list[float] = []

    num_samples_read: int = 0

    with open(file_path) as file:
        line: str = file.readline()
        sample_rate: int = int(line)
        num_samples_to_plot: int = calc_num_samples(time_window, sample_rate)
        print('number of samples to plot ', num_samples_to_plot)
        print('sample rate ', sample_rate)

        while num_samples_read != num_samples_to_plot:
            line = file.readline()

            if line == '':
                break

            split_line: list[str] = line.split(',')

            left_samples.append(float(split_line[0]))
            right_samples.append(float(split_line[1]))

            num_samples_read += 1

    return (left_samples, right_samples, sample_rate)

def plot_example_signal() -> None:
    Fs: int = 44100
    freq: int = 220
    num_samples: int = 100
    y_values: list[int] = []

    for sample_num in range(0, num_samples):
        y_values.append(numpy.sin(numpy.pi * freq * sample_num / Fs))
        print(y_values[sample_num])

    plt.plot(y_values)
    plt.xlabel('Samples')
    plt.ylabel('Amplitude')
    plt.show()

if __name__ == "__main__":
    parser = argparse.ArgumentParser(prog='plot_signal', description='Script to plot signal files')
    parser.add_argument('--file_path', required=True)
    parser.add_argument('--time_window', required=True)
    args = parser.parse_args()

    file_path: str = str(args.file_path)
    time_window: int = int(args.time_window)

    print('Reading from ', file_path)
    print('Time Window, in ms', time_window)

    # plot_example_signal()
    (left_samples, right_samples, sample_rate) = read_signal_file(file_path, time_window)

    plt.plot(left_samples)
    plt.ylabel('Amplitude')
    plt.xlabel('Sample number')

    plt.show()
