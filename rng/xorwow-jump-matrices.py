#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Copyright 2023 UT-Battelle, LLC and other Celeritas Developers.
# See the top-level COPYRIGHT file for details.
# SPDX-License-Identifier: (Apache-2.0 OR MIT)
"""
Calculate jump matrices for the xorwow RNG.

See Marsaglia (2003) for the theory behind the xorshift family of RNGs and a
description of how to form the transformation matrix.

The transformation matrix T is a companion matrix in block form:

    | 0 0 0 0 A |
    | I 0 0 0 C | 
T = | 0 I 0 0 D |
    | 0 0 I 0 E |
    | 0 0 0 I B |

where A = (I + L^a)(I + R^b), B = (I + R^c), C = D = E = 0, L is the binary
matrix that performs a left shift of one position on the binary vector (all
zeros except for ones on the principal subdiagonal), and R is the
right-shift-one binary matrix (all zeros except for ones on the principal
superdiagonal).

https://www.jstatsoft.org/index.php/jss/article/view/v008i14/916
"""

import numpy as np
from numpy.linalg import matrix_power

# The seed set is the set of 1x160 binary vectors (made up of 5 32-bit
# components). The choice of parameters is [a, b, c] = [2, 1, 4].
m = 32
n = 5
a, b, c = 2, 1, 4

L = np.eye(m, k=1, dtype=np.uint32)
R = np.eye(m, k=-1, dtype=np.uint32)
I = np.eye(m, dtype=np.uint32)
Z = np.zeros((m, m), dtype=np.uint32)

A = (I + matrix_power(L, a)) @ (I + matrix_power(R, b))
B = I + matrix_power(R, c)
C, D, E = Z, Z, Z

T = np.block([
    [Z, Z, Z, Z, A],
    [I, Z, Z, Z, C],
    [Z, I, Z, Z, D],
    [Z, Z, I, Z, E],
    [Z, Z, Z, I, B]
])

def calc_jump_matrices(size, start, exp=4):
    result = np.zeros([size, n, m, n], dtype=np.uint32)
    # Equivalent to matrix multiplication with XOR instead of addition
    J = matrix_power(T, start) % 2
    for i_mat in range(size):
        for i in range(n):
            for j in range(m):
                for k in range(n):
                    # By representing the rows of the binary matrix as computer
                    # words, fast binary matrix products can be evaluated by
                    # XORing all of the rows of the matrix for which the
                    # corresponding element in the binary vector is a one
                    result[i_mat][i][j][k] = int(
                        ''.join(str(x) for x in J[i*m+j][k*m:(k+1)*m]), base=2
                    )
        J = matrix_power(J, exp) % 2
    return result

def print_jump_matrices(jump, name):
    matrices = []
    for matrix in jump:
        blocks = []
        for block in matrix:
            rows = []
            for row in block:
                rows.append(f'{{{", ".join(str(x) for x in row)}}}')
            blocks.append(f'{{{", ".join(x for x in rows)}}}')
        matrices.append(f'{{{", ".join(x for x in blocks)}}}')
    print(f'static unsigned int const {name}[] = {{{", ".join(x for x in matrices)}}};')


# T^1, T^4, T^16, ..., T^262144
jump = calc_jump_matrices(10, 1)
print_jump_matrices(jump, 'jump')

# T^(1 * 2^67), T^(4 * 2^67), T^(16 * 2^67), ..., T^(262144 * 2^67)
jump_subsequence = calc_jump_matrices(10, 2**67)
print_jump_matrices(jump_subsequence, 'jump_subsequence')
