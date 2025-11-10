import pandas as pd
import matplotlib.pyplot as plt

# Load the CSV file
file_path = 'gps_data copy.csv'
data = pd.read_csv(file_path)

# Strip leading/trailing spaces from column names
data.columns = data.columns.str.strip()

# Ensure Latitude and Longitude values have 6 decimal places
data['Latitude'] = data['Latitude'].round(6)
data['Longitude'] = data['Longitude'].round(6)

avrlat = sum(data['Latitude'])/len(data['Latitude'])
avrlng = sum(data['Longitude'])/len(data['Longitude'])


    
def average_chunks(data, chunk_size):
    averages = []
    for i in range(0, len(data), chunk_size):
        chunk = data[i:i + chunk_size]  # Get the current chunk
        chunk_average = sum(chunk) / len(chunk)  # Calculate the average of the chunk
        averages.append(chunk_average)
    return averages


chunk_size = 100
averageslat = average_chunks(data['Latitude'], chunk_size)
averageslng = average_chunks(data['Longitude'], chunk_size)

# Plot Latitude vs Longitude
plt.figure(figsize=(8, 6))
plt.plot(averageslng, averageslat, marker='o', label='GPS Coordinates')
plt.plot(data['Longitude'], data['Latitude'] , marker='o', label='GPS Coordinates')
plt.plot(avrlng, avrlat, marker='o', label='GPS Coordinates')
# Format tick labels to 6 decimal places
plt.gca().xaxis.set_major_formatter(plt.FuncFormatter(lambda x, _: f'{x:.6f}'))
plt.gca().yaxis.set_major_formatter(plt.FuncFormatter(lambda y, _: f'{y:.6f}'))

# Add labels, title, and legend
plt.xlabel('Longitude')
plt.ylabel('Latitude')
plt.title('Latitude vs Longitude with 6 Decimal Places')
plt.grid(True)
plt.legend()
plt.tight_layout()
plt.show()