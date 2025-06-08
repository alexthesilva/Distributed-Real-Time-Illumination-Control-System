import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from sklearn.linear_model import LinearRegression

# Load the data
file_path = 'duty_vs_lux.csv'  # Update the path if necessary
data = pd.read_csv(file_path)

# Extract duty cycle and lux values
X = data['Duty Cycle (%)'].values.reshape(-1, 1)  # Duty cycle as input
y = data['Lux'].values  # Lux as target

# Fit a linear regression model (no offset)
model = LinearRegression(fit_intercept=False)
model.fit(X, y)

# Get gain (G)
G = model.coef_[0]

# Print equation
print(f"L = u * {G:.2f}")

# Generate regression line for plotting
X_range = np.linspace(min(X), max(X), 100).reshape(-1, 1)
y_pred = model.predict(X_range)

# Plot real data
plt.figure(figsize=(10, 6))
plt.scatter(X, y, color='blue', label='Real Data')
plt.plot(X_range, y_pred, color='red', label=f'Linear Fit: L = u * {G:.2f}')

# Labels and legend
plt.title('Duty Cycle vs Lux')
plt.xlabel('Duty Cycle (%)')
plt.ylabel('Lux')
plt.legend()
plt.grid(True)

# Show the plot
plt.show()
