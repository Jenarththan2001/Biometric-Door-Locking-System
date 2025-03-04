#include <Keypad.h>
#include <Adafruit_Fingerprint.h>
#include <HardwareSerial.h>

// LED Pins
#define GREEN_LED 5  // Green LED (Success)
#define RED_LED 18   // Red LED (Failure)

// Keypad Setup
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {13, 12, 14, 27};
byte colPins[COLS] = {26, 25, 33, 32};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Fingerprint Sensor Setup
HardwareSerial mySerial(2);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

void setup() {
  Serial.begin(115200);
  Serial.println("🔍 Initializing Fingerprint Sensor...");

  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, LOW);

  mySerial.begin(57600, SERIAL_8N1, 16, 17);
  finger.begin(57600);

  if (finger.verifyPassword()) {
    Serial.println("✅ Fingerprint sensor detected!");
  } else {
    Serial.println("❌ Fingerprint sensor NOT found! Check wiring.");
    while (1);
  }
}

void loop() {
  Serial.println("\n👉 Place your finger to search...");
  searchFingerprint();

  Serial.println("🔑 Enter admin password (Press `#` to confirm): ");
  String adminPass = getAdminPassword();
  
  if (adminPass == "123") {
    showAdminMenu();
  } else {
    Serial.println("❌ Incorrect password. Only fingerprint scanning available.");
  }

  delay(2000);
}

// Function to search for a fingerprint
void searchFingerprint() {
  int p = -1;
  
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
  }

  Serial.println("✅ Image captured");
  p = finger.image2Tz(1);
  if (p != FINGERPRINT_OK) {
    Serial.println("❌ Error processing image");
    return;
  }

  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    Serial.print("✅ Fingerprint found! ID: ");
    Serial.println(finger.fingerID);
    digitalWrite(GREEN_LED, HIGH); // Turn on Green LED
    digitalWrite(RED_LED, LOW);
    delay(3000);
    digitalWrite(GREEN_LED, LOW);
  } else {
    Serial.println("❌ Fingerprint not found");
    digitalWrite(RED_LED, HIGH); // Turn on Red LED
    digitalWrite(GREEN_LED, LOW);
    delay(3000);
    digitalWrite(RED_LED, LOW);
  }
}

// Function to get admin password from the keypad
String getAdminPassword() {
  String password = "";
  char key;
  
  while (true) {
    key = keypad.getKey();
    if (key && key >= '0' && key <= '9') {
      password += key;
      Serial.print("*");  // Hide password
    } else if (key == '#') {  // Confirm with #
      Serial.println();
      return password;
    }
  }
}

// Function to show the admin menu
void showAdminMenu() {
  Serial.println("✅ Admin Access Granted!");
  Serial.println("Select an option using the Keypad:");
  Serial.println("1: Enroll Fingerprint");
  Serial.println("2: Delete Fingerprint");
  Serial.println("3: Exit");

  char key = waitForKey();
  Serial.print("Selected: "); Serial.println(key);

  if (key == '1') {
    Serial.println("📌 Enter ID (1-127) to enroll:");
    int id = getIDFromKeypad();
    if (id > 0) {
      enrollFingerprint(id);
    } else {
      Serial.println("❌ Invalid ID!");
    }
  } else if (key == '2') {
    Serial.println("📌 Enter ID to delete:");
    int delID = getIDFromKeypad();
    deleteFingerprint(delID);
  } else if (key == '3') {
    Serial.println("👋 Exiting...");
  } else {
    Serial.println("❌ Invalid choice! Try again.");
  }

  delay(2000);
}

// Function to enroll a fingerprint
void enrollFingerprint(int id) {
  int p = -1;
  Serial.print("👉 Place your finger for ID "); Serial.println(id);

  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
  }

  Serial.println("\n✅ Image captured");
  p = finger.image2Tz(1);
  if (p != FINGERPRINT_OK) return;

  Serial.println("👉 Remove your finger...");
  delay(2000);

  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }

  Serial.println("👉 Place the same finger again...");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
  }

  Serial.println("✅ Second image captured");
  p = finger.image2Tz(2);
  if (p != FINGERPRINT_OK) return;

  p = finger.createModel();
  if (p != FINGERPRINT_OK) return;

  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("✅ Fingerprint stored successfully!");
  } else {
    Serial.println("❌ Error storing fingerprint");
  }
}

// Function to delete a fingerprint
void deleteFingerprint(int id) {
  int p = finger.deleteModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("✅ Fingerprint deleted successfully!");
  } else {
    Serial.println("❌ Error deleting fingerprint");
  }
}

// Function to wait for a key press
char waitForKey() {
  char key = 0;
  while (!key) {
    key = keypad.getKey();
  }
  return key;
}

// Function to get an ID from the keypad
int getIDFromKeypad() {
  int id = 0;
  char key;
  
  Serial.print("Enter ID: ");
  while (true) {
    key = keypad.getKey();
    if (key && key >= '0' && key <= '9') {
      id = id * 10 + (key - '0');
      Serial.print(key);
    } else if (key == '#') {
      Serial.println();
      return id;
    }
  }
}
