import plotly.graph_objs as go
import numpy as np


def vector_field(
    x,
    y,
    z,
    u,
    v,
    w,
    normalise_scale=False,
    scale=1,
    arrow_head_sizeref=0.2,
    arrow_head_color="grey",
    arrow_body_length_scale=0.1,
    arrow_body_line_width=3,
    arrow_body_line_color=None,
):
    arrow_head_sizeref *= scale
    arrow_body_length_scale *= scale
    arrow_body_line_width *= scale

    if normalise_scale:
        x = x - x.min()
        x = x / x.max()
        y = y - y.min()
        y = y / y.max()
        z = z - z.min()
        z = z / z.max()

    traces = []

    normed_uvw = np.stack([u, v, w]).T

    norms = np.linalg.norm(normed_uvw, axis=1)[:, None]
    normed_uvw = normed_uvw / norms

    _displacement_x = x + arrow_body_length_scale * u
    _displacement_y = y + arrow_body_length_scale * v
    _displacement_z = z + arrow_body_length_scale * w

    if arrow_head_sizeref > 0:
        arrow_head = go.Cone(
            x=_displacement_x,
            y=_displacement_y,
            z=_displacement_z,
            u=normed_uvw[:, 0],
            v=normed_uvw[:, 1],
            w=normed_uvw[:, 2],
            colorscale=[[0, arrow_head_color], [1, arrow_head_color]],
            showscale=False,
            sizemode="scaled",
            anchor="tip",
            sizeref=arrow_head_sizeref,
        )
        traces.append(arrow_head)

    nones_per_dim = [None] * len(x)

    data_pack = np.stack(
        [
            [x, y, z],
            [
                _displacement_x,
                _displacement_y,
                _displacement_z,
            ],
            [nones_per_dim, nones_per_dim, nones_per_dim],
        ]
    )

    n_data = len(x)

    edges = data_pack.reshape(3, -1).T.reshape(-1, 3 * n_data)

    if arrow_body_line_color is not None:
        line_color = arrow_body_line_color
    else:
        line_color = norms.flatten()
        line_color = np.repeat(line_color, 3)

    arrow_body = go.Scatter3d(
        x=edges[0, :],
        y=edges[1, :],
        z=edges[2, :],
        line_color=line_color,
        line_showscale=True,
        line_colorscale="thermal",
        mode="lines",
        line_width=arrow_body_line_width,
        line_colorbar_thickness=15,
    )
    traces.append(arrow_body)
    return traces
