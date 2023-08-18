import matplotlib.pyplot as plt

# Define scale factors and offsets
pressure_scale = 0.00075339618  # Adjust this based on your data
pressure_offset = 0.0  # Adjust this based on your data
volume_scale = -0.008171282491987  # Adjust this based on your data
volume_offset = 2.0  # Adjust this based on your data
volume_cold = 1.1349 #

# Define volume threshold values
volume_min_threshold = 140.0
volume_max_threshold = 280.0

# Define color thresholds for D value
d_threshold = 120.0

# Read data from the text file
pressure = []
volume = []
colors = []  # Store colors for each data point
last_valid_volume = None

with open('pv_data.txt', 'r') as file:
    for line in file:
        if line.startswith("P="):
            parts = line.strip().split(':')
            pressure_part = None
            volume_part = None
            d_part = None

            for part in parts:
                if part.startswith('P='):
                    pressure_part = part
                elif part.startswith('V='):
                    volume_part = part
                elif part.startswith('D='):
                    d_part = part

            if pressure_part and volume_part and d_part:
                pressure_value = float(pressure_part.split('=')[1].split()[0]) 
                volume_value = float(volume_part.split('=')[1].split()[0]) 
                d_value = float(d_part.split('=')[1].split()[0])

                # Filter volume outliers
                if volume_value < volume_min_threshold or volume_value > volume_max_threshold:
                    if last_valid_volume is None:
                        volume_value = (volume_offset + volume_cold) / -volume_scale
                    else:
                        volume_value = (volume_offset + volume_cold) / -volume_scale
                        #volume_value = last_valid_volume

                last_valid_volume = volume_value
                pressure_value = pressure_value * pressure_scale + pressure_offset
                volume_value = volume_value * volume_scale + volume_offset + volume_cold
                pressure.append(pressure_value)
                volume.append(volume_value)
                
                # Assign color based on D value
                if d_value < d_threshold:
                    colors.append('blue')
                else:
                    colors.append('red')

# Create the PV scatter plot with colored dots
plt.scatter(volume, pressure, c=colors, marker='o')
plt.xlabel('Volume')
plt.ylabel('Pressure Bar')
plt.title('PV Diagram')
plt.grid()
plt.show()
