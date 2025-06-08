import serial
import time
import matplotlib.pyplot as plt
import numpy as np  # Used for computing mean and standard deviation

def monitor_and_plot(port='COM5', baud_rate=115200, duration=60):
    try:
        ser = serial.Serial(port, baud_rate, timeout=1)
        print(f"Connected to {port} with baud rate {baud_rate}")
    except serial.SerialException as e:
        print(f"Error opening port {port}: {e}")
        return

    # Wait for the "Ready!" trigger from the serial port.
    print("Waiting for 'Ready!' trigger from serial...")
    while True:
        if ser.in_waiting:
            try:
                line = ser.readline().decode('utf-8').strip()
                if "Ready!" in line:
                    print("Trigger received: Ready!")
                    break
            except UnicodeDecodeError:
                print("Error decoding received data")

    # Collect PWM measurements (in the format "$value$") after the trigger.
    data_points = []  # List to store tuples (timestamp, measurement)
    start_time = time.time()
    print(f"Collecting Fs measurements for {duration} seconds...")
    time.sleep(1)
    try:
        while time.time() - start_time < duration:
            if ser.in_waiting:
                try:
                    line = ser.readline().decode('utf-8').strip()
                    # Process lines that start and end with '$'
                    if line.startswith('$') and line.endswith('$'):
                        value_str = line.strip('$')
                        try:
                            value = float(value_str)
                            timestamp = time.time() - start_time
                            data_points.append((timestamp, value))
                            print(f"{timestamp:.2f} s: Measurement = {value}")
                        except ValueError:
                            print(f"Error converting measurement: {value_str}")
                except UnicodeDecodeError:
                    print("Error decoding received data")
    except KeyboardInterrupt:
        print("Interrupted by user.")
    finally:
        ser.close()

    if not data_points:
        print("No Fs data was collected.")
        return

    # Separate timestamps and measurements.
    times = [pt[0] for pt in data_points]
    measurements = [pt[1] for pt in data_points]

    # Compute the average and standard deviation (jitter) of the PWM values.
    avg_value = np.mean(measurements)
    std_dev = np.std(measurements)
    max_value = np.max(measurements)
    min_value = np.min(measurements)

    print(f"\nAverage Fs measurement: {avg_value:.5f}")
    print(f"Jitter (standard deviation): {std_dev:.5f}")
    print(f"Minimum: {min_value:.5f}, Maximum: {max_value:.5f}")

    # Plot the PWM measurements.
    plt.figure(figsize=(10, 5))
    plt.plot(times, measurements, 'b-', label=' Fs Jitter Measurement')
    plt.xlabel('Time (s)')
    plt.ylabel('Fs Value')
    plt.title("Fs Signal Measurement and Jitter")
    plt.legend()
    plt.grid(True)

    # Plot the average line for reference.
    plt.axhline(avg_value, color='r', linestyle='--', label=f'Average = {avg_value:.5f}')
    plt.text(times[0], avg_value, f'Avg: {avg_value:.5f}', color='r', verticalalignment='bottom')

    # Save and display the plot.
    filename = "pwm_jitter.png"
    plt.savefig(filename)
    print(f"Plot saved as {filename}")
    plt.show()

if __name__ == '__main__':
    monitor_and_plot()
