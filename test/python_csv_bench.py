import csv
import os
import time
import gc
import io
import psutil

FILE = "benchmark_1m.csv"
ROWS = 1_000_000
COLS = 20


def now():
    return time.perf_counter()


def mem_mb():
    process = psutil.Process(os.getpid())
    return process.memory_info().rss / (1024 * 1024)


def generate_csv():
    print("Generating CSV...")

    t1 = now()

    with open(FILE, "w", newline="") as f:
        writer = csv.writer(f)

        writer.writerow([f"col{i}" for i in range(1, COLS + 1)])

        for r in range(1, ROWS + 1):
            row = [
                str(r),
                str(r * 2),
                f"{r / 100:.4f}",
                f"user_{r}",
                '"quoted,value"',
                f"city_{r % 100}",
                str(r % 500),
                str(r * 10),
                str(r / 3),
                f"data_{r}",
                str(r + 11),
                str(r + 12),
                str(r + 13),
                str(r + 14),
                str(r + 15),
                str(r + 16),
                str(r + 17),
                str(r + 18),
                str(r + 19),
                str(r + 20),
            ]

            writer.writerow(row)

            if r % 100000 == 0:
                print(f"Generated {r} rows...")

    t2 = now()
    print(f"CSV generated in {t2 - t1:.3f} sec")


def benchmark_parse():
    gc.collect()

    print("\nBenchmarking parse...")
    print(f"Memory before: {mem_mb():.2f} MB")

    t1 = now()

    with open(FILE, "r", newline="") as f:
        reader = csv.reader(f)
        rows = list(reader)

    t2 = now()

    print(f"Parse time: {t2 - t1:.3f} sec")
    print(f"Rows parsed: {len(rows)}")
    print(f"Columns in row 1: {len(rows[0])}")
    print(f"Memory after: {mem_mb():.2f} MB")

    return rows


def benchmark_write():
    gc.collect()

    rows = [
        ["name", "age", "city"],
        ["Shivam", "23", "Gwalior"],
        ["Alice", "25", "New York"],
        ["Bob", "27", "London"],
    ]

    ITER = 100000

    print("\nBenchmarking write...")
    print(f"Memory before write: {mem_mb():.2f} MB")

    buffer = io.StringIO()
    writer = csv.writer(buffer)

    t1 = now()

    for _ in range(ITER):
        buffer.seek(0)
        buffer.truncate(0)
        writer.writerows(rows)
        _ = buffer.getvalue()

    t2 = now()

    print(f"Write benchmark ({ITER} iterations): {t2 - t1:.3f} sec")
    print(f"Memory after write: {mem_mb():.2f} MB")


if not os.path.exists(FILE):
    generate_csv()
else:
    print("Benchmark CSV already exists.")

benchmark_parse()
benchmark_write()
