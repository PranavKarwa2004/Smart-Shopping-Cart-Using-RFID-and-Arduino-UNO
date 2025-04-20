#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 9
MFRC522 rfid(SS_PIN, RST_PIN);

// Product Structure
struct Product {
    String name;
    float price;
};

// List of products and their RFID tag IDs
Product products[] = {
    {"Milk", 29},
    {"Bread", 25},
    {"Rice", 32},
    {"Cookie",35}
};

// Update these with the correct RFID tag IDs (Make sure these match your tags)
String productIDs[] = {"ADBBCA6B", "A459FEB6", "D4BC09B0", "43965A29"};

float totalBill = 0.0;
String cart[10];  // Stores added products
int cartSize = 0;
bool shoppingActive = false;  // Flag to track if shopping is active

void setup() {
    Serial.begin(9600);
    SPI.begin();
    rfid.PCD_Init();

    Serial.println("🔄 Smart Shopping Cart System");
    Serial.println("📌 Type 'start' to begin shopping...");
}

void loop() {
    if (Serial.available()) {
        String command = Serial.readStringUntil('\n');
        command.trim();

        if (command == "start") {
            startShopping();
        } else if (command == "remove" && shoppingActive) {
            removeItem();
        } else if (command == "checkout" && shoppingActive) {
            checkout();
        }
    }

    if (!shoppingActive) return;  // If shopping hasn't started, do nothing

    // Check if an RFID card is present
    if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) return;

    // Get RFID Tag ID
    String tagID = "";
    for (byte i = 0; i < rfid.uid.size; i++) {
        tagID += String(rfid.uid.uidByte[i], HEX);
    }
    tagID.toUpperCase();  // Convert to uppercase to match stored IDs

    Serial.println("\n📡 Scanned Tag ID: " + tagID);

    bool found = false;
    for (int i = 0; i < 4; i++) {
        if (tagID == productIDs[i]) {
            Serial.println("✅ Product Added: " + products[i].name);
            Serial.println("💰 Price: ₹" + String(products[i].price));

            totalBill += products[i].price;

            // Add to cart
            cart[cartSize] = products[i].name;
            cartSize++;

            found = true;
            break;
        }
    }

    if (!found) {
        Serial.println("❌ Unknown product! Please register the RFID tag.");
    }

    Serial.println("🛒 Total Bill: ₹" + String(totalBill));

    Serial.println("👉 Type 'remove' to remove an item or 'checkout' to finish shopping");
    Serial.println("--------------------------------------");

    delay(2000);
    rfid.PICC_HaltA();
}

// Function to start shopping
void startShopping() {
    shoppingActive = true;
    totalBill = 0.0;
    cartSize = 0;
    Serial.println("\n✅ Shopping started! Scan products to add to your cart...");
    Serial.println("📌 Type 'remove' to remove an item");
    Serial.println("📌 Type 'checkout' to finish shopping\n");
}

// Function to remove an item from the cart
void removeItem() {
    if (cartSize == 0) {
        Serial.println("⚠️ Your cart is empty! No items to remove.");
        return;
    }

    Serial.println("🗑️ Enter the name of the item to remove:");
    for (int i = 0; i < cartSize; i++) {
        Serial.println(String(i + 1) + ". " + cart[i]);
    }

    while (!Serial.available());  // Wait for user input
    String itemName = Serial.readStringUntil('\n');
    itemName.trim();

    bool removed = false;
    for (int i = 0; i < cartSize; i++) {
        if (cart[i] == itemName) {
            // Adjust bill
            for (int j = 0; j < 4; j++) {
                if (products[j].name == itemName) {
                    totalBill -= products[j].price;
                    break;
                }
            }

            // Shift cart items left
            for (int j = i; j < cartSize - 1; j++) {
                cart[j] = cart[j + 1];
            }
            cartSize--;

            Serial.println("✅ " + itemName + " removed from cart.");
            Serial.println("🛒 New Total Bill: ₹" + String(totalBill));
            removed = true;
            break;
        }
    }

    if (!removed) {
        Serial.println("❌ Item not found in cart.");
    }

    Serial.println("--------------------------------------");
}

// Function to complete checkout
void checkout() {
    Serial.println("\n🛍️ Final Bill:");
    for (int i = 0; i < cartSize; i++) {
        Serial.println(cart[i]);
    }
    Serial.println("🛒 Total Amount: ₹" + String(totalBill));
    Serial.println("✅ Payment Successful!");
    Serial.println("🙏 Thank you for shopping!");

    // Reset shopping session
    shoppingActive = false;
    Serial.println("\n🔄 Shopping session ended. Type 'start' to begin again...\n");
}
