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

def plot_signal(signal: list[float], title: str, figure_number: int, domain: str, sample_rate: int):
    plt.figure(figure_number)
    plt.title(title)

    if domain == 'time':
        plt.plot(signal)
        plt.ylabel('Amplitude')
        plt.xlabel('Sample number')
    elif domain == 'frequency':
        # Magnitude of frequencies
        magnitude = numpy.abs(signal)
        magnitude_x_axis = numpy.arange(-sample_rate / 2, sample_rate / 2, sample_rate / len(magnitude))
        plt.plot(magnitude_x_axis, magnitude, 'b', label = 'Magnitude')

        phase = numpy.angle(signal)
        phase_x_axis = numpy.arange(-sample_rate / 2, sample_rate / 2, sample_rate / len(phase))
        plt.plot(phase_x_axis, phase, 'r', label = 'Phase')
        plt.legend()
        plt.xlabel('Frequency')
    else:
        print('Invalid domain type: ' + domain)
        exit()

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

    figure_number: int = 0

    figure_number += 1
    plot_signal(left_samples, 'Left Channel Time Domain', figure_number, 'time', sample_rate)
    figure_number += 1
    plot_signal(right_samples, 'Right Channel Time Domain', figure_number, 'time', sample_rate)

    left_samples_fft = numpy.fft.fftshift(numpy.fft.fft(left_samples))
    right_samples_fft = numpy.fft.fftshift(numpy.fft.fft(right_samples))

    figure_number += 1
    plot_signal(left_samples_fft, 'Left Channel Frequency Domain', figure_number, 'frequency', sample_rate)
    figure_number += 1
    plot_signal(right_samples_fft, 'Right Channel Frequency Domain', figure_number, 'frequency', sample_rate)

    plt.show()
