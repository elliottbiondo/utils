#!/usr/bin/env python3
# Copyright 2023 UT-Battelle, LLC, and other Celeritas developers.
# See the top-level COPYRIGHT file for details.
# SPDX-License-Identifier: (Apache-2.0 OR MIT)
"""
Generate a GraphViz DAG of GDML logical volume relationships.

The resulting output file can be converted to a PDF file with, e.g.::

    dot -Tpdf:quartz demo.gdml.dot -o demo.pdf

"""

import re
import xml.etree.ElementTree as ET

from collections import defaultdict
from pathlib import Path

class PointerReplacer:
    sub = re.compile(r'0x[0-9a-f]{4,}').sub

    def __init__(self):
        self.addrs = defaultdict(dict)

    def repl(self, match):
        addr = match.group(0)
        
        # Get the prefix before the pointer in the original string
        prefix = match.string[:match.start()]
        prefix_addrs = self.addrs[prefix]

        # Get or create a new index
        idx = prefix_addrs.setdefault(addr, len(prefix_addrs))

        if idx == 0:
            return ""
        
        # Create the replacement string
        return f"@{idx:d}"

    def __call__(self, s):
        return self.sub(self.repl, s)

class Graph:
    def __init__(self):
        self.nodes = []
        self.edges = defaultdict(int)
        self.borders = defaultdict(list) # phys vol name -> border surf names
        self.replace_pointers = PointerReplacer()

    def add_volume(self, el):
        edges = self.edges

        pname = self.replace_pointers(el.attrib["name"])
        self.nodes.append(pname)
        for vrel in el.iter("volumeref"):
            dname = self.replace_pointers(vrel.attrib["ref"])
            edges[(pname, dname)] += 1

    def add_border(self, el):
        # Add border surface name
        pname = self.replace_pointers(el.attrib["name"])
        self.nodes.append(pname)
        for vrel in el.iter("physvolref"):
            dname = self.replace_pointers(vrel.attrib["ref"])
            self.borders[dname].append(pname)

    def add_world(self, vrel):
        pname = self.replace_pointers(vrel.attrib["ref"])
        self.nodes.append(pname)

    def replace_borders(self, structure):
        if not self.borders:
            return
        for pvel in structure.iter("physvol"):
            pv_name = self.replace_pointers(pvel.attrib["name"])
            if pv_name not in self.borders:
                continue

            # Get the LV name pointed to by the PV
            vrel = next(pvel.iter("volumeref"))
            dname = self.replace_pointers(vrel.attrib["ref"])

            assert self.borders[pv_name]

            # Add edge for each border
            for b in self.borders[pv_name]:
                self.edges[(b, dname)] += 1

        self.borders.clear()

    @property
    def weighted_edges(self):
        for ((u, v), weight) in self.edges.items():
            yield (u, v, weight)

    @property
    def labeled_edges(self):
        for ((u, v), weight) in self.edges.items():
            yield (u, v, ("" if weight == 1 else f"×{weight}"))

    @property
    def pointer_addresses(self):
        return self.replace_pointers.addrs

lvref_tags = {"volume", "assembly", "skinsurface"}

def read_graph(filename):
    tree = ET.parse(filename)
    structure = next(tree.iter("structure"))

    g = Graph()
    for el in structure:
        if el.tag in lvref_tags:
            g.add_volume(el)
        elif el.tag == "bordersurface":
            g.add_border(el)
        else:
            raise ValueError(f"Unrecognized structure tag: {el!r}")
    g.replace_borders(structure)
    g.add_world(tree.findall("./setup/world")[0])

    return g

def write_graph(g, filename):
    with open(filename, 'w') as f:
        f.write("digraph {\n")
        f.write("  rankdir=LR;\n")
        
        # Write nodes (in reverse order to maintain original behavior)
        for node in reversed(g.nodes):
            f.write(f'  "{node}";\n')
        
        # Write edges with labels
        for (u, v, label) in g.labeled_edges:
            if label:
                f.write(f'  "{u}" -> "{v}" [label="{label}"];\n')
            else:
                f.write(f'  "{u}" -> "{v}";\n')
        
        f.write("}\n")
        
        # Append pointer mapping
        f.write("// Pointer mapping:\n")
        for prefix, prefix_addrs in g.pointer_addresses.items():
            if len(prefix_addrs) == 1:
                continue
            f.write(f"// {prefix}\n")
            for idx, addr in sorted((idx, addr) for (addr, idx) in prefix_addrs.items()):
                f.write(f"//   {idx:04d}: {addr}\n")

def main(*args):
    from argparse import ArgumentParser
    parser = ArgumentParser(description=__doc__, prog="gdml-to-dot")
    parser.add_argument('-o', '--output')
    parser.add_argument('input')
    ns = parser.parse_args(*args)
    input = Path(ns.input)
    g = read_graph(input)
    write_graph(g, ns.output or (input.stem + ".dot"))

if __name__ == "__main__":
    main()
