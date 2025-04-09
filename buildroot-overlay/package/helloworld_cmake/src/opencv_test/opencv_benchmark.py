import cv2
import time
import numpy as np

def benchmark_opencv_function(func, *args, iterations=100, **kwargs):
    """
    Benchmarks the execution time of an OpenCV function.

    Args:
        func: The OpenCV function to benchmark.
        *args: Positional arguments to pass to the function.
        iterations: The number of times to run the function for averaging.
        **kwargs: Keyword arguments to pass to the function.

    Returns:
        The average execution time in milliseconds.
    """
    total_time = 0
    for _ in range(iterations):
        start_time = time.perf_counter()
        func(*args, **kwargs)
        end_time = time.perf_counter()
        total_time += (end_time - start_time) * 1000  # Convert to milliseconds
    return total_time / iterations

if __name__ == "__main__":
    # Load an image for testing
    try:
        image = cv2.imread("1.jpg")  # Replace with your image path
        if image is None:
            print("Error loading image. Make sure 'your_image.jpg' exists.")
            exit()
    except Exception as e:
        print(f"Error loading image: {e}")
        exit()

    # --- Benchmarking Examples ---

    # 1. Benchmarking cv2.cvtColor
    avg_time_cvt_color = benchmark_opencv_function(cv2.cvtColor, image, cv2.COLOR_BGR2GRAY)
    print(f"Average time for cv2.cvtColor (BGR to GRAY): {avg_time_cvt_color:.4f} ms")

    # 2. Benchmarking cv2.GaussianBlur
    avg_time_gaussian_blur = benchmark_opencv_function(cv2.GaussianBlur, image, (5, 5), 0)
    print(f"Average time for cv2.GaussianBlur: {avg_time_gaussian_blur:.4f} ms")

    # 3. Benchmarking cv2.Canny
    gray_image = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
    avg_time_canny = benchmark_opencv_function(cv2.Canny, gray_image, 100, 200)
    print(f"Average time for cv2.Canny: {avg_time_canny:.4f} ms")

    # 4. Benchmarking cv2.resize
    scale_percent = 50
    width = int(image.shape[1] * scale_percent / 100)
    height = int(image.shape[0] * scale_percent / 100)
    dim = (width, height)
    avg_time_resize = benchmark_opencv_function(cv2.resize, image, dim, interpolation=cv2.INTER_LINEAR)
    print(f"Average time for cv2.resize: {avg_time_resize:.4f} ms")

    # 5. Benchmarking a more complex operation (e.g., object detection - requires a model)
    # Note: This is just a template, you'll need to load your own model
    # try:
    #     net = cv2.dnn.readNet("your_model.weights", "your_model.cfg")
    #     layer_names = net.getLayerNames()
    #     output_layers = [layer_names[i[0] - 1] for i in net.getUnconnectedOutLayers()]
    #     blob = cv2.dnn.blobFromImage(image, 1/255.0, (416, 416), swapRB=True, crop=False)
    #
    #     def run_detection(net, blob):
    #         net.setInput(blob)
    #         return net.forward(output_layers)
    #
    #     avg_time_detection = benchmark_opencv_function(run_detection, net, blob)
    #     print(f"Average time for object detection: {avg_time_detection:.4f} ms")
    # except Exception as e:
    #     print(f"Skipping object detection benchmark: {e}")

    # You can add more benchmarks for other OpenCV functions here

    print("\nPerformance testing complete.")
