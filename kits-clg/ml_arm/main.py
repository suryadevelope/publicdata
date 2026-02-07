import cv2
import mediapipe as mp
import firebase_admin
from firebase_admin import credentials, db

# Firebase init
if not firebase_admin._apps:
    cred = credentials.Certificate("credentials.json")
    firebase_admin.initialize_app(cred, {
        'databaseURL': 'https://senior-702f2-default-rtdb.firebaseio.com/'
    })

ref = db.reference("/gesture")

mp_hands = mp.solutions.hands
mp_drawing = mp.solutions.drawing_utils
hands = mp_hands.Hands(min_detection_confidence=0.5, min_tracking_confidence=0.5)

cap = cv2.VideoCapture(0)

# Finger landmark indexes
fingers = {
    "Load1": (4, 2),    # Thumb
    "Load2": (8, 6),    # Index
    "Load3": (12, 10),  # Middle
    "Load4": (16, 14),  # Ring
    "Load5": (20, 18)   # Little
}

while True:
    success, img = cap.read()
    if not success:
        break

    img_rgb = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
    result = hands.process(img_rgb)

    if result.multi_hand_landmarks:
        for hand_landmark in result.multi_hand_landmarks:
            mp_drawing.draw_landmarks(
                img, hand_landmark, mp_hands.HAND_CONNECTIONS
            )

            landmarks = hand_landmark.landmark
            data = {}

            for load, (tip, joint) in fingers.items():
                # Thumb uses X axis, others use Y axis
                if tip == 4:
                    data[load] = 1 if landmarks[tip].x > landmarks[joint].x else 0
                else:
                    data[load] = 1 if landmarks[tip].y < landmarks[joint].y else 0

            ref.update(data)
            print(data)

    cv2.imshow("Hand Gesture Detection", img)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
