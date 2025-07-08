import matplotlib.pyplot as plt
import pandas as pd

NO_OPTS = 7

def plot_and_save_times(opt=""):
    plt.figure(figsize=(10, 6))
    names = [
        "base", "opt1", "opt2", "opt3_1", "opt3_2", "opt4_1",
        "opt4_2", "opt5_1", "opt5_2", "opt6_1", "opt6_2" 
    ]
    ctr = 0

    for i in range(NO_OPTS):
        if i < 3:
            filename = f"times{opt}/times{ctr+1}.csv"
            df = pd.read_csv(filename, header=None)
            sizes = df.iloc[:, 0].to_numpy()
            times = df.iloc[:, 1].to_numpy()

            plt.plot(sizes, times)
            plt.scatter(sizes, times, s=10, label=names[ctr])
            ctr += 1
        else:
            filename = f"times{opt}/times{ctr+1}.csv"
            df = pd.read_csv(filename, header=None)
            sizes = df.iloc[:, 0].to_numpy()
            times = df.iloc[:, 1].to_numpy()

            filename = f"times{opt}/times{ctr+2}.csv"
            df = pd.read_csv(filename, header=None)
            times2 = df.iloc[:, 1].to_numpy()

            plt.plot(sizes, times)
            plt.scatter(sizes, times, s=10, label=names[ctr])
            plt.plot(sizes, times2)
            plt.scatter(sizes, times2, s=10, label=names[ctr+1])
            ctr += 2

        plt.xlabel("Rozmiar macierzy", fontsize=14)
        plt.ylabel("Czas [s]", fontsize=14)
        plt.legend(loc="upper left")
        plt.savefig(f"graphs/times{opt}/times{i+1}.png")
        plt.clf()


def plot_and_save_flops(opt=""):
    plt.figure(figsize=(12, 5))
    names = [
        "base", "opt1", "opt2", "opt3_1", "opt3_2", "opt4_1",
        "opt4_2", "opt5_1", "opt5_2", "opt6_1", "opt6_2" 
    ]
    ctr = 0

    for i in range(NO_OPTS):
        if i < 3:
            filename = f"flops{opt}/flops{ctr+1}.csv"
            df = pd.read_csv(filename, header=None)
            sizes = df.iloc[:, 0].to_numpy()
            flops = df.iloc[:, 1].to_numpy()

            if i > 0:
                filename = f"flops{opt}/flops{ctr}.csv"
                df = pd.read_csv(filename, header=None)
                flops_prev = df.iloc[:, 1].to_numpy()

                plt.plot(sizes, flops_prev, linestyle="--")
                plt.scatter(sizes, flops_prev, s=10, label=names[ctr-1])
                

            plt.plot(sizes, flops, linestyle="--")
            plt.scatter(sizes, flops, s=10, label=names[ctr])
            ctr += 1
        elif i == 3:
            filename = f"flops{opt}/flops{ctr+1}.csv"
            df = pd.read_csv(filename, header=None)
            sizes = df.iloc[:, 0].to_numpy()
            flops = df.iloc[:, 1].to_numpy()

            filename = f"flops{opt}/flops{ctr+2}.csv"
            df = pd.read_csv(filename, header=None)
            flops2 = df.iloc[:, 1].to_numpy()

            filename = f"flops{opt}/flops{ctr}.csv"
            df = pd.read_csv(filename, header=None)
            flops_prev = df.iloc[:, 1].to_numpy()
        
            plt.plot(sizes, flops_prev, linestyle="--")
            plt.scatter(sizes, flops_prev, s=10, label=names[ctr-1])
            plt.plot(sizes, flops, linestyle="--")
            plt.scatter(sizes, flops, s=10, label=names[ctr])
            plt.plot(sizes, flops2, linestyle="--")
            plt.scatter(sizes, flops2, s=10, label=names[ctr+1])
            ctr += 2
        else:
            filename = f"flops{opt}/flops{ctr+1}.csv"
            df = pd.read_csv(filename, header=None)
            sizes = df.iloc[:, 0].to_numpy()
            flops = df.iloc[:, 1].to_numpy()

            filename = f"flops{opt}/flops{ctr+2}.csv"
            df = pd.read_csv(filename, header=None)
            flops2 = df.iloc[:, 1].to_numpy()

            filename = f"flops{opt}/flops{ctr}.csv"
            df = pd.read_csv(filename, header=None)
            flops_prev = df.iloc[:, 1].to_numpy()

            filename = f"flops{opt}/flops{ctr-1}.csv"
            df = pd.read_csv(filename, header=None)
            flops_prev2 = df.iloc[:, 1].to_numpy()
        
            plt.plot(sizes, flops_prev2, linestyle="--")
            plt.scatter(sizes, flops_prev2, s=10, label=names[ctr-2])
            plt.plot(sizes, flops_prev, linestyle="--")
            plt.scatter(sizes, flops_prev, s=10, label=names[ctr-1])
            plt.plot(sizes, flops, linestyle="--")
            plt.scatter(sizes, flops, s=10, label=names[ctr])
            plt.plot(sizes, flops2, linestyle="--")
            plt.scatter(sizes, flops2, s=10, label=names[ctr+1])
            ctr += 2
        
        plt.xlabel("Rozmiar macierzy", fontsize=14)
        plt.ylabel("GFLOPS", fontsize=14)
        plt.yticks([i if opt == "o2" else 0.25 * i for i in range(11)])
        plt.legend(loc="upper left")
        plt.savefig(f"graphs/flops{opt}/flops{i+1}.png")
        plt.clf()


def plot_all_flops(opt=""):
    plt.figure(figsize=(12, 5))
    names = [
        "base", "opt1", "opt2", "opt3_1", "opt3_2", "opt4_1",
        "opt4_2", "opt5_1", "opt5_2", "opt6_1", "opt6_2" 
    ]

    for i, name in enumerate(names):
        filename = f"flops{opt}/flops{i+1}.csv"
        df = pd.read_csv(filename, header=None)
        sizes = df.iloc[:, 0].to_numpy()
        flops = df.iloc[:, 1].to_numpy()

        plt.plot(sizes, flops, linestyle="--")
        plt.scatter(sizes, flops, s=10, label=name)
        
    plt.xlabel("Rozmiar macierzy", fontsize=14)
    plt.ylabel("GFLOPS", fontsize=14)
    plt.yticks([i if opt == "o2" else 0.25 * i for i in range(11)])
    plt.legend(loc="upper left")
    plt.savefig(f"graphs/flops_all{opt}.png")


if __name__ == "__main__":
    # graphs for times
    plot_and_save_times()
    plot_and_save_times("o2")

    # graphs for flops
    plot_and_save_flops()
    plot_and_save_flops("o2")

    # all flops together
    plot_all_flops()
    plot_all_flops("o2")
