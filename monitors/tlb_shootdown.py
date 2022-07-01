#!/usr/bin/env python3
# %%
import os
import subprocess as sp
import shlex
from time import sleep
from unicodedata import name

from numpy import matrix

ncores = os.cpu_count()

# %%


def get_interrupts():
    # /proc/interrupts records the number of interrupts per IRQ per CPU.
    # Syntax:
    # The first row is the CPU number.
    # The first column is the IRQ number,
    # the last two columns are the type of interrput and the related device.
    ret = sp.run(shlex.split("cat /proc/interrupts"),
                 stdout=sp.PIPE, text=True)
    res = {}
    for line in ret.stdout.split(sep='\n')[1:]:
        items = line.split()
        if len(items) < ncores+1:
            continue
        key = "".join([items[0]] + items[ncores+1:])
        value = [int(i) for i in items[1:ncores+1]]
        res[key] = value
    return res


def get_tlb_shootdowns(res):
    for k, v in res.items():
        if "TLB" in k:
            return v
    return None


def print_2d_matrix(matrix):
    print("\n".join(["\t".join([str(cell) for cell in row])for row in matrix]))


def green(text):
    return "\033[32m" + text + "\033[0m"


def red(text):
    return "\033[31m" + text + "\033[0m"


if __name__ == "__main__":
    init = get_tlb_shootdowns(get_interrupts())
    prev = init
    while(True):
        sleep(1)
        curr = get_tlb_shootdowns(get_interrupts())
        delta = [curr[i]-prev[i] for i in range(ncores)]
        total = [curr[i]-init[i] for i in range(ncores)]

        selected_cores = [i for i in range(ncores) if delta[i] > 0]
        matrix = [
            [green('CORE'+str(i)) if delta[i] > 0 else 'CORE'+str(i)
             for i in selected_cores],
            [delta[i] for i in selected_cores],
            [total[i] for i in selected_cores]
        ]
        print("-"*80)
        print_2d_matrix(matrix)
        print(f"delta shootdowns:{sum(delta)}")
        print(f"accumulated shootdowns: {sum(total)}")
        prev = curr
