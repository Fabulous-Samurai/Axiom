import time
import os
import psutil

# Approach 1: instantiate every time
start = time.time()
for _ in range(10000):
    process = psutil.Process(os.getpid())
    mem = process.memory_info().rss / 1024 / 1024
t1 = time.time() - start

# Approach 2: instantiate once
start = time.time()
process = psutil.Process(os.getpid())
for _ in range(10000):
    mem = process.memory_info().rss / 1024 / 1024
t2 = time.time() - start

print(f"Approach 1: {t1:.4f}s")
print(f"Approach 2: {t2:.4f}s")
print(f"Speedup: {t1/t2:.2f}x")
