import cv2
import time

cap = cv2.VideoCapture(0)
light_durations = []
start_time = None
last_off_time = 0
is_light_on = False
morse_string = ""


while True:
    ret, frame = cap.read()

    # Apply greyscale and threshold before processing
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    _, thresh = cv2.threshold(gray, 250, 255, cv2.THRESH_BINARY)
    pixel_count = cv2.countNonZero(thresh)

    if pixel_count > 9500:
        if not is_light_on:
            start_time = time.time()
            is_light_on = True
            print("Light On")
            off_duration = time.time() - last_off_time
            if light_durations:
                print(max(light_durations))
                if off_duration > (max(light_durations)*0.6):
                    morse_string += " "
    else:
        if is_light_on:
            duration = time.time() - start_time
            light_durations.append(duration)
            is_light_on = False
            print(f"Light Off. Duration: {duration:.2f}s")

            if len(light_durations) == 1:
                # Assume first signal is a dot for initialization
                morse_string += "."

            else:
                # Compare current duration to the shortest seen so far
                min_dur = min(light_durations)
                if duration > (min_dur * 2):
                    morse_string += "-"
                else:
                    morse_string += "."
            print(f"Current Morse: {morse_string}")
            last_off_time = time.time()

    cv2.imshow('Threshold Feed', thresh)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
print(f"Final Message: {morse_string}")