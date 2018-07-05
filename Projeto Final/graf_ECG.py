# -*- coding: utf-8 -*-
"""
Created on Wed Jul  4 14:39:37 2018

@author: bruno
"""

import numpy as np
import matplotlib.pyplot as plt
import pandas as pd

data = pd.read_csv("recebidos.txt", header=None, delimiter=r"\s+")

data1 = np.asarray(data)

plt.plot(data1[0:1000])
plt.grid()
plt.show()