#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Copyright 2023 UT-Battelle, LLC and other Celeritas Developers.
# See the top-level COPYRIGHT file for details.
# SPDX-License-Identifier: (Apache-2.0 OR MIT)
"""
Calculate jump matrices or polynomials for the xorwow PRNG.

See [1]_ for the theory behind the xorshift family of RNGs and a description of
how to form the transformation matrix.

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

References
----------
.. [1] Marsaglia, G. 2003. Xorshift RNGs. Journal of Statistical Software 8,
   14, 1–6. https://www.jstatsoft.org/index.php/jss/article/view/v008i14/916
.. [2] Haramoto, H., Matsumoto, M., Nishimura, T., Panneton, F., L’Ecuyer, P.
   2008. Efficient jump ahead for F2-linear random number generators. INFORMS
   Journal on Computing.
   https://pubsonline.informs.org/doi/10.1287/ijoc.1070.0251
.. [3] Collins, Joseph C. 2008. Testing, Selection, and Implementation of Random
   Number Generators. ARL-TR-4498. https://apps.dtic.mil/sti/pdfs/ADA486637.pdf
"""

import argparse
import numpy as np
from numpy.linalg import matrix_power
import sympy as sp

# The seed set is the set of 1x160 binary vectors (made up of 5 32-bit
# components). The choice of parameters is [a, b, c] = [2, 1, 4].
nb = 32
nw = 5
nk = nb * nw
a, b, c = 2, 1, 4

O = np.zeros((nb, nb), dtype=np.uint32)
I = np.eye(nb, dtype=np.uint32)
L = np.eye(nb, k=1, dtype=np.uint32)
R = np.eye(nb, k=-1, dtype=np.uint32)

A = (I + matrix_power(L, a)) @ (I + matrix_power(R, b))
B = I + matrix_power(R, c)
C, D, E = O, O, O

T = np.block([
    [O, O, O, O, A],
    [I, O, O, O, C],
    [O, I, O, O, D],
    [O, O, I, O, E],
    [O, O, O, I, B]
])


def calc_jump_matrices(size, start, exp=4):
    """Generate the jump matrices.

    This builds the jump matrices T^{start * exp^0}, T^{start * exp^1},
    T^{start * exp^2}, ..., T^{start * exp^{size - 1}}.

    By representing the rows of the binary matrixs as computer words, fast
    binary matrix products can be evaluated by XORing all of the rows of the
    matrix for which the corresponding element in the binary vector is a one.
    """
    result = np.zeros([size, nw, nb, nw], dtype=np.uint32)
    # Equivalent to matrix multiplication with XOR instead of addition
    J = matrix_power(T, start) % 2
    for i_mat in range(size):
        for i in range(nw):
            for j in range(nb):
                for k in range(nw):
                    result[i_mat][i][j][k] = int(
                        ''.join(str(x) for x in J[i*nb+j][k*nb:(k+1)*nb]), base=2
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

def calc_char_poly():
    """ Calculate the characteristic polynomial in F2
    """
    M = sp.Matrix(T)
    return sp.PurePoly.from_poly(M.charpoly(), domain=sp.GF(2))

def calc_jump_poly(char_poly, jump_size):
    """Calculate the jump polynomial.

    This calculates the jump polynomial in O(k^2 log d) time, where k is the
    polynomial degree and d is the jump size, using Knuth's square-and-multipy
    method.
    """
    # Characteristic polynomial has degree 160
    n = char_poly.degree() - 1

    if jump_size <= n:
        return 2**jump_size

    # Integer representation of characteristic polynomial
    cp = int(''.join(str(x) for x in char_poly.all_coeffs()), base=2)

    # Start with jump polynomial jp(z) = z
    jp = 2

    # Skip the most significant bit
    for i in range(jump_size.bit_length() - 2, -1, -1):
        # Apply square-multiply operations to jp(z)
        tmp = 0
        for j in range(n, -1, -1):
            # Get a_{n-1}: leading (left-most) coefficient
            a_n = (tmp >> n) & 1
            tmp <<= 1
            if a_n:
                tmp ^= cp
            if (jp >> j) & 1:
                tmp ^= jp
        # jp(z) = jp(z)^2
        jp = tmp

        # Multiply jp(z) by z
        if (jump_size >> i) & 1:
            a_n = (jp >> n) & 1
            jp <<= 1
            if a_n:
                jp ^= cp
    return jp

def calc_jump_polys(char_poly, size, start, exp=4):
    """Calculate the jump polynomials.

    This generates the jump polynomials for jumps start * exp^0, start * exp^1,
    start * exp^2, ..., start * exp^{size - 1}.
    """
    result = []
    jump_size = start
    for _ in range(size):
        result.append(calc_jump_poly(char_poly, jump_size))
        jump_size *= exp
    return result

def print_jump_polys(jump, name):
    polys = []
    for jp in jump:
        jp_bits = f'{jp:0{nk}b}'
        jp_words = [f'{int(jp_bits[i:i + nb], base=2):#010x}u' for i in range(0, nk, nb)][::-1]
        polys.append(f'{{{", ".join(x for x in jp_words)}}}')
    print(f'static unsigned int const {name}[] = {{{", ".join(x for x in polys)}}};')


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        '-t', '--type', type=str, choices={'matrix', 'poly'}, default='poly',
        help='Whether to generate jump matrices or polynomials',
    )
    args = parser.parse_args()

    num_jumps = 32
    if args.type == 'matrix':
        # T^(4^i) for i in [0, 32)
        jump_mat = calc_jump_matrices(num_jumps, 1)
        print_jump_matrices(jump_mat, 'jump')

        # T^(4^i * 2^67) for i in [0, 32)
        jump_subsequence_mat = calc_jump_matrices(num_jumps, 2**67)
        print_jump_matrices(jump_subsequence_mat, 'jump_subsequence')
    else:
        char_poly = calc_char_poly()

        # Jump sizes 4^i for i in [0, 32)
        jump_poly = calc_jump_polys(char_poly, num_jumps, 1)
        print_jump_polys(jump_poly, 'jump')

        # Jump sizes 4^i * 2^67 for i ini [0, 32)
        jump_subsequence_poly = calc_jump_polys(char_poly, num_jumps, 2**67)
        print_jump_polys(jump_subsequence_poly, 'jump_subsequence')


if __name__ == '__main__':
    main()
