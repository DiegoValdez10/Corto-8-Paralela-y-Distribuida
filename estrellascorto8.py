# max_brightness_multiprocessing.py
import os
import time
import random
from multiprocessing import Pool, cpu_count

HEIGHT = 10
WIDTH = 10
SEED = 12345  # fija la semilla para reproducibilidad
PRINT_MATRIX = True

def llenar_matriz(height, width, seed=None):
    rng = random.Random(seed)
    return [[rng.randrange(1000) for _ in range(width)] for _ in range(height)]

def imprimir_matriz(foto):
    for fila in foto:
        print(" ".join(f"{v:4d}" for v in fila))

def max_brillo_serial(foto):
    start = time.perf_counter()
    max_val = foto[0][0]
    max_i, max_j = 0, 0
    for i in range(len(foto)):
        for j in range(len(foto[i])):
            if foto[i][j] > max_val:
                max_val = foto[i][j]
                max_i, max_j = i, j
    end = time.perf_counter()
    tiempo = end - start
    print("Serial:")
    print(f"Maximo valor: {max_val}")
    print(f"Coordenadas: ({max_i}, {max_j})")
    print(f"Tiempo serial: {tiempo:.6f} segundos")
    return tiempo, (max_val, max_i, max_j)

def worker_chunk(args):
    """Procesa un bloque de filas [i_start, i_end)."""
    foto, i_start, i_end = args
    local_max = foto[i_start][0]
    max_i, max_j = i_start, 0
    for i in range(i_start, i_end):
        for j, v in enumerate(foto[i]):
            if v > local_max:
                local_max, max_i, max_j = v, i, j
    return local_max, max_i, max_j

def max_brillo_paralelo(foto, num_procs=None):
    n_rows = len(foto)
    if num_procs is None or num_procs < 1:
        num_procs = min(cpu_count(), n_rows)

    # Particiona filas en bloques casi iguales
    base = n_rows // num_procs
    resto = n_rows % num_procs
    ranges = []
    start = 0
    for p in range(num_procs):
        size = base + (1 if p < resto else 0)
        end = start + size
        ranges.append((foto, start, end))
        start = end

    start_t = time.perf_counter()
    with Pool(processes=num_procs) as pool:
        results = pool.map(worker_chunk, ranges)
    end_t = time.perf_counter()

    # Reducción: escoger el máximo global
    max_val, max_i, max_j = results[0]
    for val, i, j in results[1:]:
        if val > max_val:
            max_val, max_i, max_j = val, i, j

    tiempo = end_t - start_t
    print("Paralelo:")
    print(f"Maximo valor: {max_val}")
    print(f"Coordenadas: ({max_i}, {max_j})")
    print(f"Tiempo paralelo: {tiempo:.6f} segundos")
    return tiempo, num_procs, (max_val, max_i, max_j)

def main():
    foto = llenar_matriz(HEIGHT, WIDTH, seed=SEED)
    if PRINT_MATRIX:
        print("Matriz de brillo:")
        imprimir_matriz(foto)

    t_serial, (s_val, s_i, s_j) = max_brillo_serial(foto)

    env_procs = os.getenv("PY_PROCS")
    num_procs = int(env_procs) if env_procs and env_procs.isdigit() else None

    t_par, P, (p_val, p_i, p_j) = max_brillo_paralelo(foto, num_procs)

    # Métricas
    speedup = t_serial / t_par if t_par > 0 else float("inf")
    eficiencia = speedup / P if P > 0 else 0.0

    print("\n--- METRICAS ---")
    print(f"Speedup: {speedup:.6f}")
    print(f"Eficiencia: {eficiencia:.6f}")
    print(f"Numero de procesos: {P}")
    print(f"Tamaño de la matriz: {HEIGHT}x{WIDTH}")

    # Validación de consistencia
    assert s_val == p_val and s_i == p_i and s_j == p_j, "Resultados difieren entre serial y paralelo"

if __name__ == "__main__":
    main()
