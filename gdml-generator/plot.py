import sys
import time
import matplotlib.pyplot as plt

sys.path[:0] = ["/Users/veb/opt/celeritas/celerpy/"]
import celerpy
from celerpy.settings import settings
settings.prefix_path = "/Users/veb/opt/celeritas/build/"
from celerpy import model, visualize

celer_geo = visualize.CelerGeo.from_filename("simple_lz.gdml")
draw_image = visualize.Imager(
    celer_geo,
    model.ImageInput(
        lower_left=[-80, -80, 0],
        upper_right=[80, 80, 0],
        rightward=[1.0, 0.0, 0.0],
        vertical_pixels=1024,
    ))

(fig, ax) = plt.subplots()

# Start timer
start_time = time.perf_counter()

# Call draw_image
draw_image(ax)

# Stop timer
end_time = time.perf_counter()
print(f"draw_image execution time: {end_time - start_time:.4f} seconds")

plt.savefig("out.pdf", bbox_inches="tight")
