import matplotlib.pyplot as plt

# Initialize empty lists to store data
policies = []
times = []

# Read data from the text file
with open('policy_time.txt', 'r') as file:
    for line in file:
        parts = line.strip().split()
        if len(parts) == 2:
            policy, time = parts
            policies.append(policy)
            times.append(float(time))

# Define colors for each policy
colors = ['blue', 'green', 'red']

# Create a bar plot
plt.bar(policies, times, color=colors)

# Set labels and title
plt.xlabel('Scheduling Policy')
plt.ylabel('Time (seconds)')
plt.title('Execution Time for Different Scheduling Policies')

# Show the plot
plt.show()
