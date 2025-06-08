import serial
import time
import matplotlib.pyplot as plt
import pandas as pd

# Serial configuration
SERIAL_PORT = 'COM4'  # Change to match your Arduino port
BAUD_RATE = 115200

# Initialize serial connection
ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
time.sleep(2)  # Allow time for connection

# Data storage
lux_values = []
duty_values = []
recording = False

print("Waiting for duty cycle to start...")

# Read and store data from Arduino
while True:
    if ser.in_waiting > 0:
        line = ser.readline().decode('utf-8').strip()

        if line.startswith("Lux:"):
            lux = float(line.split(': ')[1])
        elif line.startswith("Duty cycle:"):
            duty = float(line.split(': ')[1])
            print(f"Duty cycle: {duty}%, Lux: {lux}")

            # Start recording at duty = 0
            if duty == 0 and not recording:
                print("Starting recording...")
                recording = True

            if recording:
                duty_values.append(duty)
                lux_values.append(lux)

            # Stop recording at duty = 100
            if duty == 100 and recording:
                print("Stopping recording...")
                break

# Close the serial connection
ser.close()

# Ensure we have collected data before plotting and saving
if duty_values and lux_values:
    # Save data to CSV
    data = pd.DataFrame({'Duty Cycle (%)': duty_values, 'Lux': lux_values})
    csv_filename = 'duty_vs_lux.csv'
    data.to_csv(csv_filename, index=False)
    print(f"Data saved to '{csv_filename}'")

    # Plot the data
    plt.figure(figsize=(12, 6))
    plt.plot(duty_values, lux_values, marker='o', linestyle='-')
    plt.title('Duty Cycle (%) vs Lux')
    plt.xlabel('Duty Cycle (%)')
    plt.ylabel('Lux')
    plt.grid(True)
    plt.show()
else:
    print("No data collected — check Arduino output.")

