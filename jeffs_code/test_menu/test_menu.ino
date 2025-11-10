#define BUTTON_UP_PIN 2
#define BUTTON_DOWN_PIN 1
#define BUTTON_SELECT_PIN 0
#define BUTTON_CANCEL_PIN 10

const int debounceDelay = 50;  // Debounce delay in milliseconds
unsigned long lastDebounceTime[4] = {0, 0, 0, 0};
int buttonState[4] = {HIGH, HIGH, HIGH, HIGH};
int lastButtonState[4] = {HIGH, HIGH, HIGH, HIGH};

int currentMenu = 0;  // Current menu index (0-3)
bool inSubMenu = false;  // Track whether we are in a submenu

const char *menuItems[] = {"Feature", "New Race", "Leader Board", "Setting"};
const int menuSize = sizeof(menuItems) / sizeof(menuItems[0]);

void setup() {
  Serial.begin(115200);

  // Initialize buttons as inputs with pull-up resistors
  pinMode(BUTTON_UP_PIN, INPUT_PULLUP);
  pinMode(BUTTON_DOWN_PIN, INPUT_PULLUP);
  pinMode(BUTTON_SELECT_PIN, INPUT_PULLUP);
  pinMode(BUTTON_CANCEL_PIN, INPUT_PULLUP);

  // Display initial menu
  displayMenu();
}

void loop() {
  // Check all buttons
  for (int i = 0; i < 4; i++) {
    int buttonPin = BUTTON_UP_PIN + i;  // Get button pin dynamically
    int reading = digitalRead(buttonPin);

    // Debounce logic
    if (reading != lastButtonState[i]) {
      lastDebounceTime[i] = millis();
    }

    if ((millis() - lastDebounceTime[i]) > debounceDelay) {
      if (reading != buttonState[i]) {
        buttonState[i] = reading;

        // Handle button presses
        if (buttonState[i] == LOW) {
          handleButtonPress(i);
        }
      }
    }
    lastButtonState[i] = reading;
  }
}

void handleButtonPress(int buttonIndex) {
  if (!inSubMenu) {
    // Handle navigation in the main menu
    switch (buttonIndex) {
      case 0:  // Up button
        currentMenu = (currentMenu - 1 + menuSize) % menuSize;  // Wrap around to the last menu item
        displayMenu();
        break;

      case 1:  // Down button
        currentMenu = (currentMenu + 1) % menuSize;  // Wrap around to the first menu item
        displayMenu();
        break;

      case 2:  // Select button
        inSubMenu = true;
        Serial.print("Entering submenu: ");
        Serial.println(menuItems[currentMenu]);
        break;

      case 3:  // Cancel button
        // Do nothing in the main menu
        break;
    }
  } else {
    // Handle submenu actions
    switch (buttonIndex) {
      case 3:  // Cancel button
        inSubMenu = false;
        Serial.println("Returning to main menu.");
        displayMenu();
        break;

      default:
        // Handle other buttons in the submenu if needed
        break;
    }
  }
}

void displayMenu() {
  Serial.println("Main Menu:");
  for (int i = 0; i < menuSize; i++) {
    if (i == currentMenu) {
      Serial.print("> ");  // Highlight the selected menu item
    } else {
      Serial.print("  ");
    }
    Serial.println(menuItems[i]);
  }
}
