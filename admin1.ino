#include <Keypad.h>
#include <Adafruit_Fingerprint.h>
#include <HardwareSerial.h>

#define GREEN_LED 5  // Green LED (Success)
#define RED_LED 18   // Red LED (Failure)

bool inAdminMode = false;  // Flag to track admin mode

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
  char key = keypad.getKey();  // Continuously check keypad input

  if (key == '#' && !inAdminMode) {
    handleAdminMode();
  } 

  // Prevent fingerprint scanning when in Admin Mode
  if (!inAdminMode) {
    searchFingerprint();
  }
}

// Function to search for a fingerprint
void searchFingerprint() {
  Serial.println("\n👉 Place your finger to search...");
  int p = finger.getImage();

  if (p == FINGERPRINT_NOFINGER) {
    return;
  }

  if (p != FINGERPRINT_OK) {
    Serial.println("❌ Error capturing image");
    return;
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
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(RED_LED, LOW);
  } else {
    Serial.println("❌ Fingerprint not found");
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);
  }

  delay(2000);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, LOW);
}

// Function to handle admin access
void handleAdminMode() {
  inAdminMode = true;  // Prevent fingerprint scanning in admin mode
  Serial.println("\n🔑 Enter Admin Password (Press `#` to confirm): ");
  String adminPass = getAdminPassword();

  if (adminPass == "123") {
    showAdminMenu();
  } else {
    Serial.println("❌ Incorrect password. Resuming fingerprint search...");
    inAdminMode = false;  // Resume fingerprint scanning
  }
}

// Function to get admin password from keypad
String getAdminPassword() {
  String password = "";
  char key;

  while (true) {
    key = keypad.getKey();
    if (key && key >= '0' && key <= '9') {
      password += key;
      Serial.print("*");  
    } else if (key == '#') {  
      Serial.println();
      return password;
    }
  }
}

// Function to show admin menu
void showAdminMenu() {
  Serial.println("\n✅ Admin Access Granted!");

  while (true) {  // Keep Admin Mode running until Exit (3) is selected
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
      Serial.println("👋 Exiting Admin Mode...");
      inAdminMode = false;  // Resume fingerprint scanning
      return;  // Exit admin mode and go back to fingerprint scanning
    }
  }
}

// Function to enroll a fingerprint
void enrollFingerprint(int id) {
  int p = -1;

  // Step 1: Capture the first fingerprint image
  Serial.print("👉 Place your finger for ID "); Serial.println(id);
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    if (p == FINGERPRINT_NOFINGER) {
      Serial.println("👆 Please place your finger on the sensor...");
      delay(1000);  // Wait for 1 second before retrying
    } else if (p != FINGERPRINT_OK) {
      Serial.println("❌ Error capturing image. Please try again.");
      return;  // Return to admin menu if there's an error
    }
  }
  Serial.println("✅ First image captured");

  // Step 2: Convert the first image to a template
  p = finger.image2Tz(1);
  if (p != FINGERPRINT_OK) {
    Serial.println("❌ Error processing image. Please try again.");
    return;  // Return to admin menu if there's an error
  }

  // Step 3: Ask the user to remove their finger
  Serial.println("👉 Remove your finger...");
  delay(2000);  // Wait for the user to remove their finger

  // Step 4: Capture the second fingerprint image
  Serial.println("👉 Place the same finger again...");
  p = -1;
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    if (p == FINGERPRINT_NOFINGER) {
      Serial.println("👆 Please place your finger on the sensor...");
      delay(1000);  // Wait for 1 second before retrying
    } else if (p != FINGERPRINT_OK) {
      Serial.println("❌ Error capturing second image. Please try again.");
      return;  // Return to admin menu if there's an error
    }
  }
  Serial.println("✅ Second image captured");

  // Step 5: Convert the second image to a template
  p = finger.image2Tz(2);
  if (p != FINGERPRINT_OK) {
    Serial.println("❌ Error processing second image. Please try again.");
    return;  // Return to admin menu if there's an error
  }

  // Step 6: Create a fingerprint model
  p = finger.createModel();
  if (p != FINGERPRINT_OK) {
    Serial.println("❌ Error creating fingerprint model. Please try again.");
    return;  // Return to admin menu if there's an error
  }

  // Step 7: Store the fingerprint model
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("✅ Fingerprint stored successfully!");
  } else {
    Serial.println("❌ Error storing fingerprint. Please try again.");
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