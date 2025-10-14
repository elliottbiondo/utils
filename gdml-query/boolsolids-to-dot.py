#!/usr/bin/env python
# Copyright Celeritas contributors: see top-level COPYRIGHT file for details
# SPDX-License-Identifier: (Apache-2.0 OR MIT)
""" """

import xml.etree.ElementTree as ET

from collections import defaultdict
from pathlib import Path
from gdmlutils import PointerReplacer, get_solid_refs

SOLID_TAG_LABEL = {
    "multiUnion": "|",
    "union": "|",
    "subtraction": "-",
    "intersection": "&",
}


class Graph:
    def __init__(self):
        self.nodes = []
        self.edges = []
        self.replace_pointers = PointerReplacer()

    def add_solid(self, el):
        edges = self.edges

        try:
            tag_label = SOLID_TAG_LABEL[el.tag]
        except KeyError:
            return

        name = self.replace_pointers(el.attrib["name"])
        self.nodes.append((name, tag_label))

        for ref in get_solid_refs(el):
            ref = self.replace_pointers(ref)
            edges.append((name, ref))

    @property
    def pointer_addresses(self):
        return self.replace_pointers.addrs


def read_graph(filename):
    tree = ET.parse(filename)
    solids = next(tree.iter("solids"))

    g = Graph()
    for el in solids:
        g.add_solid(el)
    return g


def write_graph(g, filename):
    with open(filename, "w") as f:
        f.write("digraph {\n")
        f.write("  rankdir=LR;\n")

        for (node, taglabel) in g.nodes:
            f.write(f'  "{node}" [label="{node}\\n{taglabel}"];\n')

        # Write edges with labels
        for u, v in g.edges:
            f.write(f'  "{u}" -> "{v}";\n')

        f.write("}\n")

        # Append pointer mapping
        f.write("// Pointer mapping:\n")
        for prefix, prefix_addrs in g.pointer_addresses.items():
            if len(prefix_addrs) == 1:
                continue
            f.write(f"// {prefix}\n")
            for idx, addr in sorted(
                (idx, addr) for (addr, idx) in prefix_addrs.items()
            ):
                f.write(f"//   {idx:04d}: {addr}\n")


def main(*args):
    from argparse import ArgumentParser

    parser = ArgumentParser(description=__doc__, prog="gdml-to-dot")
    parser.add_argument("-o", "--output")
    parser.add_argument("input", type=Path)
    ns = parser.parse_args(*args)
    g = read_graph(ns.input)
    write_graph(g, ns.output or (ns.input.stem + "-solids.dot"))


if __name__ == "__main__":
    main()
