#!/usr/bin/env python
# Copyright 2024 UT-Battelle, LLC, and other Celeritas developers.
# See the top-level COPYRIGHT file for details.
# SPDX-License-Identifier: (Apache-2.0 OR MIT)
"""
Count the number of logical and physical volumes in a GDML file.
"""

from itertools import count
from collections import namedtuple
import json
from pathlib import Path
import sys
import xml.etree.ElementTree as ET
from sys import stderr

def log(msg, *args, file=stderr, **kwargs):
    print(msg, *args, file=file, **kwargs)
    file.flush()

def get_material(volume_el):
    el = next(volume_el.iter("materialref"))
    return el.attrib["ref"]


def parse_materials(tree):
    materials = next(tree.iter("materials"))

    result = {
        "isotope": count(),
        "element": count(),
        "material": count(),
    }
    for el in materials:
        next(result[el.tag])

    return {k: next(v) for k, v in result.items()}


ChildCount = namedtuple("ChildCount", ["direct", "total", "maxdepth"])
LEAF_CHILD_COUNT = ChildCount(direct=0, total=1, maxdepth=0)


def multiunion_refs(el):
    for node in el:
        for subel in node.iter("solid"):
            yield subel.attrib["ref"]


def boolean_refs(el):
    for node in el:
        if node.tag in ("first", "second"):
            yield node.attrib["ref"]


def physvol_refs(el):
    for vrel in el.iter("volumeref"):
        yield vrel.attrib["ref"]


def parse_solids(tree):
    solids = next(tree.iter("solids"))
    child_counts = {}

    for num_solids, el in enumerate(solids):
        if el.tag == "multiUnion":
            get_refs = multiunion_refs
        elif el.tag in ("union", "subtraction", "intersection"):
            get_refs = boolean_refs
        else:
            continue

        indirect = 1
        maxdepth = 0
        direct = count()
        for vrel in get_refs(el):
            cc = child_counts.get(vrel, LEAF_CHILD_COUNT)
            indirect += cc.total
            next(direct)
            maxdepth = max(maxdepth, cc.maxdepth)

        cc = ChildCount(direct=next(direct), total=indirect, maxdepth=(maxdepth + 1))
        child_counts[el.attrib["name"]] = cc

    if child_counts:
        max_depth = max(cc.maxdepth for cc in child_counts.values())
    else:
        max_depth = None

    return {
        "bool_solids": len(child_counts),
        "bool_max_depth": max_depth,
        "solids": num_solids,
    }


def parse_structure(tree):
    structure = next(tree.iter("structure"))
    child_counts = {}

    # Counters
    border = 0
    skin = 0
    physical = 0

    for logical, el in enumerate(structure):
        if el.tag == "bordersurface":
            border += 1
            continue

        if el.tag == "skinsurface":
            skin += 1
            continue

        if el.tag not in ("volume", "assembly"):
            raise ValueError(f"Unrecognized structure tag: {el!r}")

        indirect = 1
        maxdepth = 0
        direct = count()
        for ref in physvol_refs(el):
            cc = child_counts[ref]
            indirect += cc.total
            next(direct)
            maxdepth = max(maxdepth, cc.maxdepth)

        cc = ChildCount(direct=next(direct), total=indirect, maxdepth=(maxdepth + 1))
        child_counts[el.attrib["name"]] = cc
        physical += cc.direct

    # Account for world volume
    logical += 1
    physical += 1
    world = tree.findall("./setup/world")[0]

    cc = child_counts[world.attrib["ref"]]

    return {
        "logical": logical,
        "physical": physical,
        "touchable": cc.total,
        "border": border,
        "skin": skin,
        "depth": cc.maxdepth,
    }


def log_progress():
    log(".", end="")

def parse_gdml(filename: Path):
    result = {"filename": filename.name}

    tree = ET.parse(filename)
    log_progress()
    result.update(parse_solids(tree))
    log_progress()
    result.update(parse_materials(tree))
    log_progress()
    result.update(parse_structure(tree))
    log("done")
    return result


def main(*args):
    from argparse import ArgumentParser

    parser = ArgumentParser(description=__doc__, prog="gdml-volumes")
    parser.add_argument("-p", "--prepend", default=None)
    parser.add_argument("-o", "--output", default="-")
    parser.add_argument("input", nargs="+")
    ns = parser.parse_args(*args)

    result = []
    if ns.prepend:
        try:
            with open(ns.prepend) as f:
                result = json.load(f)
        except IOError as e:
            log(f"Could not open {ns.prepend}: {e!s}")
    assert isinstance(result, list)

    for filename in sorted(ns.input):
        path = Path(filename)
        if path.is_symlink():
            log(f"Skipping symlink {filename}")
            continue
        log(f"Processing {filename}", end="")
        try:
            result.append(parse_gdml(path))
        except Exception as e:
            log(f"ERROR: {e!s}")
            result.append(None)

    if ns.output == "-":
        json.dump(result, sys.stdout, indent=1)
    else:
        with open(ns.output, "w") as f:
            json.dump(result, f, indent=0)


if __name__ == "__main__":
    main()
