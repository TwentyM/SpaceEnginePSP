#!/usr/bin/env python3

import argparse
import json
import math
import os
import struct
from pathlib import Path


MAGIC = b"SEPSPM01"
VERSION = 1

MARKER_ENGINE = 1
MARKER_WEAPON = 2


# ------------------------------------------------------------
# Vector / quaternion helpers
# ------------------------------------------------------------

def vec_add(a, b):
    return (
        a[0] + b[0],
        a[1] + b[1],
        a[2] + b[2],
    )


def vec_length(v):
    return math.sqrt(
        v[0] * v[0] +
        v[1] * v[1] +
        v[2] * v[2]
    )


def normalize(v):
    length = vec_length(v)

    if length < 0.000001:
        return (0.0, 0.0, 0.0)

    return (
        v[0] / length,
        v[1] / length,
        v[2] / length,
    )


def quaternion_rotate(q, v):
    """
    glTF quaternion order:
        x, y, z, w
    """

    x, y, z, w = q
    vx, vy, vz = v

    # q * v * conjugate(q)
    tx = 2.0 * (y * vz - z * vy)
    ty = 2.0 * (z * vx - x * vz)
    tz = 2.0 * (x * vy - y * vx)

    return (
        vx + w * tx + (y * tz - z * ty),
        vy + w * ty + (z * tx - x * tz),
        vz + w * tz + (x * ty - y * tx),
    )


def quaternion_multiply(a, b):
    ax, ay, az, aw = a
    bx, by, bz, bw = b

    return (
        aw * bx + ax * bw + ay * bz - az * by,
        aw * by - ax * bz + ay * bw + az * bx,
        aw * bz + ax * by - ay * bx + az * bw,
        aw * bw - ax * bx - ay * by - az * bz,
    )


# ------------------------------------------------------------
# glTF hierarchy
# ------------------------------------------------------------

def get_node_local_transform(node):
    translation = tuple(
        node.get(
            "translation",
            [0.0, 0.0, 0.0]
        )
    )

    rotation = tuple(
        node.get(
            "rotation",
            [0.0, 0.0, 0.0, 1.0]
        )
    )

    scale = tuple(
        node.get(
            "scale",
            [1.0, 1.0, 1.0]
        )
    )

    return translation, rotation, scale


def combine_transform(parent, local):
    pt, pr, ps = parent
    lt, lr, ls = local

    scaled_local = (
        lt[0] * ps[0],
        lt[1] * ps[1],
        lt[2] * ps[2],
    )

    rotated_local = quaternion_rotate(
        pr,
        scaled_local
    )

    world_translation = vec_add(
        pt,
        rotated_local
    )

    world_rotation = quaternion_multiply(
        pr,
        lr
    )

    world_scale = (
        ps[0] * ls[0],
        ps[1] * ls[1],
        ps[2] * ls[2],
    )

    return (
        world_translation,
        world_rotation,
        world_scale
    )


def build_world_transforms(gltf):
    nodes = gltf["nodes"]

    identity = (
        (0.0, 0.0, 0.0),
        (0.0, 0.0, 0.0, 1.0),
        (1.0, 1.0, 1.0),
    )

    result = {}

    def visit(index, parent_transform):
        node = nodes[index]

        local = get_node_local_transform(
            node
        )

        world = combine_transform(
            parent_transform,
            local
        )

        result[index] = world

        for child in node.get(
            "children",
            []
        ):
            visit(child, world)

    scene_index = gltf.get("scene", 0)

    scene = gltf["scenes"][scene_index]

    for root_index in scene["nodes"]:
        visit(
            root_index,
            identity
        )

    return result


# ------------------------------------------------------------
# glTF binary accessors
# ------------------------------------------------------------

COMPONENT_FORMATS = {
    5123: ("H", 2),  # unsigned short
    5125: ("I", 4),  # unsigned int
    5126: ("f", 4),  # float
}


TYPE_COMPONENTS = {
    "SCALAR": 1,
    "VEC2": 2,
    "VEC3": 3,
    "VEC4": 4,
}


def read_accessor(
    gltf,
    binary_data,
    accessor_index
):
    accessor = gltf[
        "accessors"
    ][accessor_index]

    buffer_view = gltf[
        "bufferViews"
    ][accessor["bufferView"]]

    component_type = accessor[
        "componentType"
    ]

    fmt_char, component_size = \
        COMPONENT_FORMATS[
            component_type
        ]

    component_count = \
        TYPE_COMPONENTS[
            accessor["type"]
        ]

    count = accessor["count"]

    accessor_offset = accessor.get(
        "byteOffset",
        0
    )

    view_offset = buffer_view.get(
        "byteOffset",
        0
    )

    stride = buffer_view.get(
        "byteStride",
        component_size *
        component_count
    )

    start = (
        view_offset +
        accessor_offset
    )

    output = []

    fmt = (
        "<" +
        fmt_char *
        component_count
    )

    size = (
        component_size *
        component_count
    )

    for i in range(count):
        offset = (
            start +
            i * stride
        )

        values = struct.unpack_from(
            fmt,
            binary_data,
            offset
        )

        if component_count == 1:
            output.append(
                values[0]
            )
        else:
            output.append(
                values
            )

    return output


# ------------------------------------------------------------
# Transform vertices
# ------------------------------------------------------------

def transform_position(
    position,
    transform
):
    translation, rotation, scale = transform

    scaled = (
        position[0] * scale[0],
        position[1] * scale[1],
        position[2] * scale[2],
    )

    rotated = quaternion_rotate(
        rotation,
        scaled
    )

    return vec_add(
        translation,
        rotated
    )


def transform_normal(
    normal,
    transform
):
    _, rotation, _ = transform

    rotated = quaternion_rotate(
        rotation,
        normal
    )

    return normalize(rotated)


# ------------------------------------------------------------
# Marker handling
# ------------------------------------------------------------

def get_marker_type(name):
    if name.startswith("FX_Engine_"):
        return MARKER_ENGINE

    if name.startswith("HP_Gun_"):
        return MARKER_WEAPON

    return None


def marker_type_name(marker_type):
    if marker_type == MARKER_ENGINE:
        return "ENGINE"

    if marker_type == MARKER_WEAPON:
        return "WEAPON"

    return "UNKNOWN"


# ------------------------------------------------------------
# Main compilation
# ------------------------------------------------------------

def compile_ship(
    input_path,
    output_path
):
    input_path = Path(
        input_path
    )

    output_path = Path(
        output_path
    )

    with open(
        input_path,
        "r",
        encoding="utf-8"
    ) as file:
        gltf = json.load(file)

    if len(gltf["buffers"]) != 1:
        raise RuntimeError(
            "Only one glTF binary buffer "
            "is supported for now."
        )

    bin_uri = gltf[
        "buffers"
    ][0]["uri"]

    bin_path = (
        input_path.parent /
        bin_uri
    )

    with open(
        bin_path,
        "rb"
    ) as file:
        binary_data = file.read()

    world_transforms = \
        build_world_transforms(
            gltf
        )

    hull_node_index = None

    for i, node in enumerate(
        gltf["nodes"]
    ):
        if node.get("name") == \
                "GEO_Hull":
            hull_node_index = i
            break

    if hull_node_index is None:
        raise RuntimeError(
            "GEO_Hull node not found."
        )

    hull_node = gltf[
        "nodes"
    ][hull_node_index]

    mesh_index = hull_node.get(
        "mesh"
    )

    if mesh_index is None:
        raise RuntimeError(
            "GEO_Hull has no mesh."
        )

    mesh = gltf[
        "meshes"
    ][mesh_index]

    if len(mesh["primitives"]) != 1:
        raise RuntimeError(
            "Only one primitive is "
            "supported for now."
        )

    primitive = mesh[
        "primitives"
    ][0]

    attributes = primitive[
        "attributes"
    ]

    positions = read_accessor(
        gltf,
        binary_data,
        attributes["POSITION"]
    )

    normals = read_accessor(
        gltf,
        binary_data,
        attributes["NORMAL"]
    )

    uvs = read_accessor(
        gltf,
        binary_data,
        attributes["TEXCOORD_0"]
    )

    indices = read_accessor(
        gltf,
        binary_data,
        primitive["indices"]
    )

    hull_transform = \
        world_transforms[
            hull_node_index
        ]

    vertices = []

    bounding_radius = 0.0

    # Indexed glTF mesh -> flat PSP triangle list.
    for index in indices:
        position = transform_position(
            positions[index],
            hull_transform
        )

        normal = transform_normal(
            normals[index],
            hull_transform
        )

        uv = uvs[index]

        bounding_radius = max(
            bounding_radius,
            vec_length(position)
        )

        vertices.append(
            (
                uv[0],
                uv[1],

                normal[0],
                normal[1],
                normal[2],

                position[0],
                position[1],
                position[2],
            )
        )

    markers = []

    for node_index, node in enumerate(
        gltf["nodes"]
    ):
        name = node.get(
            "name",
            ""
        )

        marker_type = \
            get_marker_type(
                name
            )

        if marker_type is None:
            continue

        transform = \
            world_transforms[
                node_index
            ]

        position = transform[0]
        rotation = transform[1]

        # Blender Single Arrow points along local +Z.
        #
        # Blender -> glTF basis conversion maps that
        # arrow axis to glTF local +Y.
        #
        # So +Y is the correct local marker direction
        # in the exported glTF.

        forward = quaternion_rotate(
            rotation,
            (0.0, 1.0, 0.0)
        )

        forward = normalize(
            forward
        )

        markers.append(
            (
                marker_type,
                name,
                position,
                forward,
            )
        )

    output_path.parent.mkdir(
        parents=True,
        exist_ok=True
    )

    # --------------------------------------------------------
    # Binary format
    #
    # Header:
    #   magic[8]
    #   version u32
    #   vertexCount u32
    #   markerCount u32
    #   boundingRadius float
    #
    # Vertices:
    #   u, v
    #   nx, ny, nz
    #   x, y, z
    #
    # Markers:
    #   type u32
    #   name[32]
    #   position xyz
    #   forward xyz
    # --------------------------------------------------------

    with open(
        output_path,
        "wb"
    ) as output:
        output.write(
            struct.pack(
                "<8sIIIf",
                MAGIC,
                VERSION,
                len(vertices),
                len(markers),
                bounding_radius,
            )
        )

        for vertex in vertices:
            output.write(
                struct.pack(
                    "<8f",
                    *vertex
                )
            )

        for (
            marker_type,
            name,
            position,
            forward
        ) in markers:

            encoded_name = \
                name.encode(
                    "utf-8"
                )[:31]

            encoded_name += (
                b"\0" *
                (
                    32 -
                    len(encoded_name)
                )
            )

            output.write(
                struct.pack(
                    "<I32s6f",
                    marker_type,
                    encoded_name,

                    position[0],
                    position[1],
                    position[2],

                    forward[0],
                    forward[1],
                    forward[2],
                )
            )

    print()
    print("======================================")
    print(" SpaceEngine PSP Ship Compiler")
    print("======================================")
    print()

    print(
        f"Input:  {input_path}"
    )

    print(
        f"Output: {output_path}"
    )

    print()

    print(
        f"Source vertices: "
        f"{len(positions)}"
    )

    print(
        f"Indices:         "
        f"{len(indices)}"
    )

    print(
        f"Triangles:       "
        f"{len(indices) // 3}"
    )

    print(
        f"PSP vertices:    "
        f"{len(vertices)}"
    )

    print(
        f"Bounding radius: "
        f"{bounding_radius:.3f}"
    )

    print()

    print("Markers:")

    for (
        marker_type,
        name,
        position,
        forward
    ) in markers:

        print(
            f"  [{marker_type_name(marker_type)}] "
            f"{name}"
        )

        print(
            "      position = "
            f"({position[0]:.3f}, "
            f"{position[1]:.3f}, "
            f"{position[2]:.3f})"
        )

        print(
            "      forward  = "
            f"({forward[0]:.3f}, "
            f"{forward[1]:.3f}, "
            f"{forward[2]:.3f})"
        )

    print()
    print(
        f"File size: "
        f"{output_path.stat().st_size} bytes"
    )

    print()
    print("Compilation successful.")
    print()


def main():
    parser = argparse.ArgumentParser(
        description=(
            "Compile a SpaceEngine "
            "Blender glTF ship into "
            "PSP-friendly binary mesh."
        )
    )

    parser.add_argument(
        "input",
        help="Input .gltf file"
    )

    parser.add_argument(
        "output",
        help="Output .pspmesh file"
    )

    args = parser.parse_args()

    compile_ship(
        args.input,
        args.output
    )


if __name__ == "__main__":
    main()