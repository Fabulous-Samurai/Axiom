import time
import os
import psutil

process = psutil.Process(os.getpid())
start = time.time()
for _ in range(1000):
    p2 = psutil.Process(os.getpid())
    mem = p2.memory_info().rss / 1024 / 1024
t1 = time.time() - start

start = time.time()
for _ in range(1000):
    mem = process.memory_info().rss / 1024 / 1024
t2 = time.time() - start

print(f"Instantiating Process every time (1000 iter): {t1:.4f}s")
print(f"Caching Process (1000 iter): {t2:.4f}s")
