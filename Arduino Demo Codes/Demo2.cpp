// Include required libraries
#include <Wire.h>                       // For I2C communication
#include <Adafruit_SSD1306.h>          // For OLED display
#include <Keypad.h>                    // For keypad input
#include <Adafruit_Fingerprint.h>      // For fingerprint sensor
#include <HardwareSerial.h>            // For serial communication with fingerprint sensor

// OLED screen configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1  // OLED reset pin not used
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// LED pin definitions
#define GREEN_LED 5  // LED for successful fingerprint match
#define RED_LED 18   // LED for failed fingerprint match

bool inAdminMode = false;  // Flag to track if the system is in admin mode

// Keypad configuration
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {13, 12, 14, 27};     // Keypad row pins
byte colPins[COLS] = {26, 25, 33, 32};     // Keypad column pins
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Fingerprint sensor configuration using Hardware Serial 2
HardwareSerial mySerial(2);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

void setup() {
  Serial.begin(115200);  // Start serial monitor
  Serial.println("Initializing Fingerprint Sensor...");

  // Initialize LED pins
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, LOW);

  // Begin communication with fingerprint sensor
  mySerial.begin(57600, SERIAL_8N1, 16, 17);
  finger.begin(57600);

  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (1); // Stop if display fails
  }

  displayMessage("System Ready");
  delay(2000);

  // Verify fingerprint sensor connection
  if (finger.verifyPassword()) {
    Serial.println("Fingerprint sensor detected!");
    displayMessage("Fingerprint OK!");
  } else {
    Serial.println("Fingerprint sensor NOT found!");
    displayMessage("Sensor Error!");
    while (1); // Stop if fingerprint sensor not found
  }
}

void loop() {
  char key = keypad.getKey();  // Read key press

  // Enter admin mode when '#' is pressed
  if (key == '#' && !inAdminMode) {
    handleAdminMode();
  } 

  // If not in admin mode, continue fingerprint search
  if (!inAdminMode) {
    searchFingerprint();
  }
}

// Fingerprint verification function
void searchFingerprint() {
  Serial.println("\nPlace your finger to search...");
  displayMessage("Place Finger...");

  int p = finger.getImage();  // Capture fingerprint image
  if (p == FINGERPRINT_NOFINGER) return;  // No finger detected

  if (p != FINGERPRINT_OK) {
    Serial.println("Error capturing image");
    return;
  }

  Serial.println("Image captured");
  p = finger.image2Tz(1);  // Convert image to template
  if (p != FINGERPRINT_OK) {
    Serial.println("Error processing image");
    return;
  }

  // Search the fingerprint
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    Serial.print("Fingerprint found! ID: ");
    Serial.println(finger.fingerID);
    displayMessage("Access Granted");
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(RED_LED, LOW);
  } else {
    Serial.println("Fingerprint not found");
    displayMessage("Access Denied");
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);
  }

  delay(2000);  // Delay before resetting LEDs
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, LOW);
}

// Handle admin login and options
void handleAdminMode() {
  inAdminMode = true;
  Serial.println("\nEnter Admin Password: ");
  displayMessage("Enter Admin PWD");

  String adminPass = getAdminPassword();  // Get admin password from keypad

  // Check admin password
  if (adminPass == "123") {
    showAdminMenu();  // Show admin options
  } else {
    Serial.println("Incorrect password. Resuming fingerprint search...");
    displayMessage("Wrong Password!");
    delay(2000);
    inAdminMode = false;
  }
}

// Get admin password using keypad input
String getAdminPassword() {
  String password = "";
  char key;
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(10, 20);
  display.print("Enter PWD: ");
  display.display();

  while (true) {
    key = keypad.getKey();
    if (key && key >= '0' && key <= '9') {
      password += key;
      Serial.print("*");  // Masked input in Serial
      display.print("*"); // Masked input on OLED
      display.display();
    } else if (key == '#') {
      Serial.println();
      return password;
    }
  }
}

// Admin menu to enroll/delete fingerprint or exit
void showAdminMenu() {
  Serial.println("\nAdmin Access Granted!");
  displayMessage("Admin Mode");

  while (true) {
    Serial.println("1: Enroll Fingerprint");
    Serial.println("2: Delete Fingerprint");
    Serial.println("3: Exit");
    displayMessage("1:Enroll             2:Del                3:Exit");

    char key = waitForKey();  // Wait for menu selection
    Serial.print("Selected: "); Serial.println(key);

    if (key == '1') {
      int id = getIDFromKeypad("Enter ID to Enroll:");
      enrollFingerprint(id);
    } else if (key == '2') {
      int delID = getIDFromKeypad("Enter ID to Delete:");
      deleteFingerprint(delID);
    } else if (key == '3') {
      Serial.println("Exiting Admin Mode...");
      inAdminMode = false;
      displayMessage("Exiting...");
      delay(2000);
      return;
    }
  }
}

// Enroll new fingerprint with given ID
void enrollFingerprint(int id) {
  displayMessage("Enroll ID: " + String(id));

  Serial.print("Place your finger for ID "); Serial.println(id);
  displayMessage("Place Finger");

  int p = -1;
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
  }

  Serial.println("First image captured");
  p = finger.image2Tz(1);  // Convert first image
  if (p != FINGERPRINT_OK) return;

  Serial.println("Remove your finger...");
  displayMessage("Remove Finger");
  delay(2000);

  Serial.println("Place the same finger again...");
  displayMessage("Place Same Finger");
  p = -1;
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
  }

  Serial.println("Second image captured");
  p = finger.image2Tz(2);  // Convert second image
  if (p != FINGERPRINT_OK) return;

  p = finger.createModel();  // Create fingerprint model
  if (p != FINGERPRINT_OK) return;

  p = finger.storeModel(id);  // Store fingerprint
  if (p == FINGERPRINT_OK) {
    Serial.println("Fingerprint stored successfully!");
    displayMessage("Enroll Success!");
  } else {
    Serial.println("Error storing fingerprint.");
    displayMessage("Enroll Failed!");
  }
}

// Delete fingerprint for given ID
void deleteFingerprint(int id) {
  displayMessage("Deleting ID: " + String(id));
  int p = finger.deleteModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Fingerprint deleted successfully!");
    displayMessage("Deleted Successfully!");
  } else {
    Serial.println("Error deleting fingerprint");
    displayMessage("Delete Failed!");
  }
}

// Display a message on OLED screen
void displayMessage(String message) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(10, 20);
  display.println(message);
  display.display();
}

// Wait until a key is pressed on the keypad
char waitForKey() {
  char key = 0;
  while (!key) key = keypad.getKey();
  return key;
}

// Get numeric ID from keypad input
int getIDFromKeypad(String prompt) {
  int id = 0;
  char key;
  displayMessage(prompt);

  while (true) {
    key = keypad.getKey();
    if (key >= '0' && key <= '9') {
      id = id * 10 + (key - '0');  // Append digit to ID
      displayMessage(prompt + "\n" + String(id));  // Show ID in progress
    } else if (key == '#') {
      return id;  // Confirm input with #
    }
  }
}
