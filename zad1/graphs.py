from matplotlib import pyplot as plt

def plot():
    xs = [
        [1143, 4380, 9546],
        [907, 4083, 8861],
        [864, 3905, 8807],
    ]

    ys = {}
    ys[0] = {}
    ys[1] = {}
    ys[2] = {}

    titles = [
        "Tekst z dużą liczbą białych znaków",
        "Tekst z dużą liczbą znaków interpunkcyjnych",
        "Tekst z dużą liczbą duplikatów słów",
    ]

    for i in range(6):
        with open(f"times/{i+1}", 'r') as f:
            lines = f.readlines()

            for j in range(3):
                ys[j][i] = []

                for k in range(3):
                    ys[j][i].append(float(lines[4*j + k]))

    for j in range(3):
        for i in range(6):
            plt.plot(xs[j], ys[j][i], label=f"wersja {i+1}")

        plt.title(titles[j])
        plt.xlabel("Liczba znaków na wejściu")
        plt.xticks(xs[j])
        plt.ylabel("Czas wykonania programu [s]")
        plt.legend()
        plt.savefig(f"graph{j}.png")
        plt.show()
    
    plot_all(xs, ys)

def plot_all(lens, times):
    lens_sum = sum([sum(var_lens) for var_lens in lens])
    times_sum = [sum([sum(times[j][i]) for j in range(3)]) for i in range(6)]
    
    xs = [1, 2, 3, 4, 5, 6]
    ys = [lens_sum / time for time in times_sum]

    plt.bar(xs, ys)
    plt.title("Porównanie efektywności przetwarzania wszystkich wersji")
    plt.xlabel("Wersja")
    plt.ylabel("Liczba przetwarzanych znaków / s")
    plt.savefig("graph.png")
    plt.show()

plot()
                    