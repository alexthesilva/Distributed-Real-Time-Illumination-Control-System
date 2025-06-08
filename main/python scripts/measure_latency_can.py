import serial
import time
import matplotlib.pyplot as plt

def monitor_and_plot(port='COM4', baud_rate=115200, duration=60):
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

    # After the trigger, collect latency data (in microseconds) from the serial output.
    data_points = []  # List to store tuples (timestamp, latency)
    start_time = time.time()
    print(f"Collecting data for {duration} seconds...")

    try:
        while time.time() - start_time < duration:
            if ser.in_waiting:
                try:
                    line = ser.readline().decode('utf-8').strip()
                    # Check if the line contains the string "Latency in us:"
                    if "Latency in us:" in line:
                        # Example line: "Latency in us: 223"
                        parts = line.split("Latency in us:")
                        if len(parts) >= 2:
                            latency_str = parts[1].strip()
                            try:
                                latency_val = float(latency_str)
                                timestamp = time.time() - start_time
                                data_points.append((timestamp, latency_val))
                                print(f"{timestamp:.2f} s: Latency = {latency_val} us")
                            except ValueError:
                                print(f"Error converting latency value: {latency_str}")
                except UnicodeDecodeError:
                    print("Error decoding received data")
    except KeyboardInterrupt:
        print("Interrupted by user.")
    finally:
        ser.close()

    if not data_points:
        print("No latency data was collected.")
        return

    # Separate the data for plotting.
    times = [pt[0] for pt in data_points]
    latencies = [pt[1] for pt in data_points]

    # Plot the results.
    plt.figure(figsize=(10, 5))
    plt.plot(times, latencies, 'b-', label='Latency (us)')
    plt.xlabel('Time (s)')
    plt.ylabel('Latency (us)')
    plt.title("CAN-BUS Latency Measurement")
    plt.legend()
    plt.grid(True)

    # Save the figure.
    filename = "latency.png"
    plt.savefig(filename)
    print(f"Figure saved as {filename}")
    plt.show()

if __name__ == '__main__':
    monitor_and_plot()
